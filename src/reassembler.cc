#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // If this substring signals EOF, remember the index of the end-of-stream
  if ( is_last_substring ) {
    eof_received_ = true;
    // eof_index_ is the index one past the last byte (absolute)
    eof_index_ = first_index + data.size();
  }

  if ( data.empty() ) {
    // nothing to store; maybe close if EOF already satisfied
    if ( eof_received_ && first_unassembled_ >= eof_index_ ) {
      output_.writer().close();
    }
    return;
  }

  // Refresh our notion of what has been popped from the output stream
  first_unpopped_ = output_.reader().bytes_popped();
  const uint64_t cap = output_.writer().available_capacity() + output_.reader().bytes_buffered();
  // acceptable window slides with what has been popped
  first_unacceptable_ = first_unpopped_ + cap;

  // compute insertion range clipped to acceptable window
  uint64_t seg_begin = first_index;
  uint64_t seg_end = first_index + data.size(); // one past last

  // drop bytes that are before the next unassembled byte
  if ( seg_end <= first_unassembled_ ) {
    // entirely already assembled
    // still may need to close if this was the last substring
    if ( eof_received_ && first_unassembled_ >= eof_index_ ) {
      output_.writer().close();
    }
    return;
  }

  if ( seg_begin < first_unassembled_ ) { seg_begin = first_unassembled_; }

  if ( seg_begin >= first_unacceptable_ ) {
    // entirely beyond acceptable window -> ignore
    return;
  }

  if ( seg_end > first_unacceptable_ ) {
    seg_end = first_unacceptable_;
  }

  // store bytes into circular buffer and mark occupied with index tags
  for ( uint64_t idx = seg_begin; idx < seg_end; ++idx ) {
    const uint64_t buf_pos = idx % cap;
    buffer_[buf_pos] = data[ idx - first_index ];
    occupied_[buf_pos] = true;
    index_of_[buf_pos] = idx;
  }

  // attempt to assemble contiguous bytes starting at first_unassembled_

  // To implement a correct advancement respecting Writer::available_capacity(), redo assembly
  // using a writer-aware approach: repeatedly form a chunk up to available_capacity() and push it.
  while ( true ) {
    uint64_t avail = output_.writer().available_capacity();
    if ( avail == 0 ) break;
    if ( !occupied_[ first_unassembled_ % cap ] ) break;
    if ( index_of_[ first_unassembled_ % cap ] != first_unassembled_ ) break; // not the next byte yet

    // build up to avail bytes or until gap
    string chunk;
    uint64_t cur = first_unassembled_;
    while ( chunk.size() < avail && occupied_[ cur % cap ] && index_of_[ cur % cap ] == cur ) {
      chunk.push_back( buffer_[ cur % cap ] );
      ++cur;
    }

    if ( chunk.empty() ) break;

    output_.writer().push( chunk );
    // clear occupied bits for those positions and advance first_unassembled_
    for ( uint64_t i = 0; i < chunk.size(); ++i ) {
      occupied_[ ( first_unassembled_ + i ) % cap ] = false;
    }
    first_unassembled_ += chunk.size();
  }

  // update first_unacceptable_
  first_unacceptable_ = first_unpopped_ + cap;

  // If we've received EOF and have assembled up to eof_index_, close the writer
  if ( eof_received_ && first_unassembled_ >= eof_index_ ) {
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  debug( "count_bytes_pending() called" );
  // Count bytes stored in the reassembler within the current acceptable window and not yet assembled
  const uint64_t cap = output_.writer().available_capacity() + output_.reader().bytes_buffered();
  const uint64_t first_unpopped_now = output_.reader().bytes_popped();
  const uint64_t first_unacceptable_now = first_unpopped_now + cap;
  uint64_t cnt = 0;
  for ( size_t pos = 0; pos < occupied_.size(); ++pos ) {
    if ( occupied_[pos] ) {
      const uint64_t idx = index_of_[pos];
      if ( idx >= first_unassembled_ && idx < first_unacceptable_now ) ++cnt;
    }
  }
  return cnt;
}

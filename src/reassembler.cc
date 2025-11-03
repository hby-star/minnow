#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // If this substring signals EOF, remember the index of the end-of-stream
  if ( is_last_substring ) {
    eof_received_ = true;
    eof_index_ = first_index + data.size();
  }

  const uint64_t cap = output_.capacity();
  const uint64_t avail = output_.writer().available_capacity();
  first_unpopped_ = output_.reader().bytes_popped();
  first_unacceptable_ = first_unpopped_ + cap;

  uint64_t seg_begin = first_index;
  uint64_t seg_end = first_index + data.size();

  // drop bytes that are before the next unassembled byte
  if ( seg_end <= first_unassembled_ ) {
    if ( eof_received_ && first_unassembled_ >= eof_index_ ) {
      output_.writer().close();
    }
    return;
  }
  if ( seg_begin < first_unassembled_ ) { 
    seg_begin = first_unassembled_; 
  }

  // drop bytes that are beyond the acceptable window
  if ( seg_begin >= first_unacceptable_ ) {
    return;
  }
  if ( seg_end > first_unacceptable_ ) {
    seg_end = first_unacceptable_;
  }

  // store bytes into buffer
  for ( uint64_t idx = seg_begin; idx < seg_end; ++idx ) {
    const uint64_t buf_pos = idx % cap;
    buffer_[buf_pos].byte = data[ idx - first_index ];
    buffer_[buf_pos].occupied = true;
    buffer_[buf_pos].index = idx;
  }

  // attempt to assemble contiguous bytes starting at first_unassembled_
  if (!avail == 0 && 
      buffer_[ first_unassembled_ % cap ].occupied && 
      buffer_[ first_unassembled_ % cap ].index == first_unassembled_ ){
    // build up to avail bytes or until gap
    string chunk;
    uint64_t cur = first_unassembled_;
    while ( chunk.size() < avail && buffer_[ cur % cap ].occupied && buffer_[ cur % cap ].index == cur ) {
      chunk.push_back( buffer_[ cur % cap ].byte );
      ++cur;
    }

    output_.writer().push( chunk );
    // clear occupied bits for those positions and advance first_unassembled_
    for ( uint64_t i = 0; i < chunk.size(); ++i ) {
      buffer_[ ( first_unassembled_ + i ) % cap ].occupied = false;
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
  // Count bytes stored in the reassembler within the current acceptable window and not yet assembled
  uint64_t cnt = 0;
  for ( size_t pos = 0; pos < buffer_.size(); ++pos ) {
    if ( buffer_[pos].occupied ) {
      const uint64_t idx = buffer_[pos].index;
      if ( idx >= first_unassembled_ && idx < first_unacceptable_ ) ++cnt;
    }
  }
  return cnt;
}

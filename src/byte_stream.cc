#include "byte_stream.hh"
#include "debug.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

// Push data to stream, but only as much as available capacity allows.
void Writer::push( string data )
{
  // Check if stream is closed or data is empty
  if ( is_closed_ || data.empty() ) {
    return;
  }

  // Check available capacity
  if ( available_capacity() < data.size() ) {
    if( available_capacity() == 0 ) {
      return;
    }
    // Adjust data size to fit available capacity
    data = data.substr( 0, available_capacity() );
  }

  // Push data to internal buffer
  buffer_.append( data );
  // Update state variables
  total_bytes_pushed_ += data.size();
}

// Signal that the stream has reached its ending. Nothing more will be written.
void Writer::close()
{
  is_closed_ = true;
  if( buffer_.empty() ) {
    is_finished_ = true;
  }
}

// Has the stream been closed?
bool Writer::is_closed() const
{
  return is_closed_;
}

// How many bytes can be pushed to the stream right now?
uint64_t Writer::available_capacity() const
{
  return capacity_ - buffer_.size();
}

// Total number of bytes cumulatively pushed to the stream
uint64_t Writer::bytes_pushed() const
{
  return total_bytes_pushed_;
}

// Peek at the next bytes in the buffer -- ideally as many as possible.
// It's not required to return a string_view of the *whole* buffer, but
// if the peeked string_view is only one byte at a time, it will probably force
// the caller to do a lot of extra work.
string_view Reader::peek() const
{
  // Check if stream is finished or buffer is empty
  if ( is_finished_ || buffer_.empty() ) {
    return {};
  }

  return { buffer_.data(), buffer_.size() };
}

// Remove `len` bytes from the buffer.
void Reader::pop( uint64_t len )
{
  // Check if stream is finished or buffer is empty
  if ( is_finished_ || buffer_.empty() ) {
    return;
  }

  // Adjust len if it exceeds the current buffer size
  if ( len > buffer_.size() ) {
    len = buffer_.size();
  }

  // Remove `len` bytes from the buffer
  buffer_.erase( 0, len );
  // Update state variables
  total_bytes_popped_ += len;

  // Check if stream is finished
  if ( is_closed_ && buffer_.empty() ) {
    is_finished_ = true;
  }
}

// Is the stream finished (closed and fully popped)?
bool Reader::is_finished() const
{
  return is_finished_;
}

// Number of bytes currently buffered (pushed and not popped)
uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}

// Total number of bytes cumulatively popped from stream
uint64_t Reader::bytes_popped() const
{
  return total_bytes_popped_;
}

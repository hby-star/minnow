#pragma once

#include "byte_stream.hh"
#include <vector>

class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output )
    : output_( std::move( output ) ),
      buffer_( output_.writer().available_capacity(), 0 ),
      occupied_( buffer_.size(), false ),
      index_of_( buffer_.size(), 0 ),
      first_unpopped_( 0 ),
      first_unassembled_( 0 ),
      first_unacceptable_( buffer_.size() ),
      eof_received_( false ),
      eof_index_( 0 )
  {}

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  // This function is for testing only; don't add extra state to support it.
  uint64_t count_bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_;

  // buffer to hold bytes temporarily
  std::vector<char> buffer_;
  // occupancy bitmap for buffer_: true == byte present
  std::vector<bool> occupied_;
  // absolute index tag for each slot to avoid aliasing on wrap
  std::vector<uint64_t> index_of_;
  uint64_t first_unpopped_;
  uint64_t first_unassembled_;
  uint64_t first_unacceptable_;
  bool eof_received_;
  uint64_t eof_index_;
};

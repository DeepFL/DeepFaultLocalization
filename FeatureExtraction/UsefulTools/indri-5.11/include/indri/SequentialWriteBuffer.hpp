/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// SequentialWriteBuffer
//
// 15 November 2004 -- tds
//

#ifndef INDRI_SEQUENTIALWRITEBUFFER_HPP
#define INDRI_SEQUENTIALWRITEBUFFER_HPP

#include "indri/File.hpp"
#include "indri/InternalFileBuffer.hpp"
namespace indri
{
  namespace file
  {
    /*!
     * Wraps an instance of indri::file::File, and provides buffering support.
     * This class works much like the BufferedWriter class in Java; every write
     * is stored in a memory buffer until that buffer fills, then the data is
     * transferred to disk en masse.
     *
     * There are at least two good reasons to buffer data before writing.  The
     * first is to avoid system call overhead for little tiny writes.  If this
     * is all you care about, you can keep this write buffer fairly small; 
     * perhaps a few kilobytes.
     *
     * However, a write buffer can also keep the disk from seeking unnecessarily
     * if you're writing to a few files simultaenously.  For this kind of
     * application, consider using a megabyte or so.  This also helps
     * keep file fragmentation down.
     */
    class SequentialWriteBuffer {
    private:
      File& _file;
      InternalFileBuffer _current;
      UINT64 _position;
      UINT64 _eof;

    public:
      /*!
       * Constructs a new instance of the SequentialWriteBuffer class.
       * The file pointer is set to the end of the file.
       *
       * @param file The File object to wrap.
       * @param length Number of bytes to buffer.
       */
      SequentialWriteBuffer( File& file, size_t length ) :
        _file(file),
        _current(length),
        _position(0),
        _eof(0)
      {
        _eof = _file.size();
        _position = _eof;
      }

      /*!
       * Seek to position bytes into the file.
       */

      void seek( UINT64 position ) {
        // this only resets the file pointer; notice that data may not be written to
        // disk until write is called
        _position = position;
      }

      /*!
       * Allocate space in the buffer for some new data.  This is roughly equivalent to
       * memory-mapping the next length bytes of the file.
       *
       * This buffer space is valid until your next call to a SequentialWriteBuffer method.
       * 
       * @param length Number of bytes to write.
       * @return Returns a pointer to memory where you can write your data.
       */
     
      char* write( size_t length ) {
        UINT64 endBuffer = _current.filePosition + _current.buffer.size();
        UINT64 endBufferData = _current.filePosition + _current.buffer.position();
        UINT64 endWrite = length + _position;
        UINT64 startWrite = _position;
        UINT64 startBuffer = _current.filePosition;
        char* writeSpot;

        // if this write starts before the buffered data does, we have to flush
        bool writeStartsBeforeBuffer = startBuffer > startWrite;
        // if this write ends after the end of our buffer, we need to flush the buffer
        // to make room for this data.
        bool writeEndsAfterBuffer = endBuffer < endWrite;
        // if this write creates a "gap" in the buffer, we have to write data.

        // here's an example.  Suppose we have already written 1MB of 0's to 
        // the file.  Then, we seek to the beginning of the file and write 
        // 20 bytes of 1's.  Then, we seek forward to 100 bytes and try to write
        // more 0's.  There is enough space in the buffer for that write to 
        // succeed, but that would leave an 80 byte gap in the buffer.
        // when the buffer is flushed to disk, that 80 byte gap would clobber
        // the 0's that are already on disk.
        // Notice that there's no problem if the 'gap' is after the current end of
        // the file, because that space is undefined anyway.
        bool dataGap = (endBufferData < _eof && startWrite > endBufferData);

        if( writeStartsBeforeBuffer || writeEndsAfterBuffer || dataGap ) {
          flush();
          _current.filePosition = _position;

          startBuffer = _current.filePosition;
          endBuffer = _current.filePosition + _current.buffer.size();
          endBufferData = _current.filePosition + _current.buffer.position();
        }

        // There's a possibility that there isn't enough room to buffer this write,
        // even though we cleared it out.  In that case, make it bigger.
        if( endWrite > endBufferData ) {
          // need to move the buffer pointer to the end, potentially resizing buffer
          _current.buffer.write( size_t(endWrite - endBufferData) );
          endBufferData = _current.filePosition + _current.buffer.position();
        }

        assert( endWrite <= endBufferData && startWrite >= startBuffer );
        writeSpot = _current.buffer.front() + (_position - _current.filePosition);
        assert( writeSpot + length <= _current.buffer.front() + _current.buffer.position() );
        _position += length;

        return writeSpot;
      }

      /*! 
       * Writes length bytes of data from buffer into the file.
       *
       * @param buffer Buffer containing data to be written to the file.
       * @param length Bytes to copy from the buffer into the file.
       */

      void write( const void* buffer, size_t length ) {
        memcpy( write( length ), buffer, length );
      }
  
      /*!
       * Move the file pointer back by length bytes, and remove all of those bytes from the buffer.
       * It's as if you didn't write those length bytes at all.
       * 
       * @param length Number of bytes to unwrite.
       */

      void unwrite( size_t length ) {
        assert( length <= _current.buffer.position() );
        _current.buffer.unwrite( length );
        _position -= length;
      }

      /*!
       * @return Current file position.
       */
  
      UINT64 tell() const {
        return _position;
      }

      /*!
       * Flushes data in the buffer out to the file.
       */

      void flush() {
        size_t bytes = _current.buffer.position();
        // write current buffered data out to the file
        _file.write( _current.buffer.front(), _current.filePosition, _current.buffer.position() );
        // clear out the data in the buffer
        _current.buffer.clear();
        _current.filePosition += bytes;
        // update the end of file marker if necessary
        _eof = lemur_compat::max( _current.filePosition, _eof );
      }

      void flushRegion( UINT64 start, UINT64 length ) {
        if( (start+length) >= _current.filePosition &&
            start <= _current.filePosition + _current.buffer.position() )
        {
          flush();
        }
      }
    };
  }
}

#endif // INDRI_SEQUENTIALWRITEBUFFER_HPP


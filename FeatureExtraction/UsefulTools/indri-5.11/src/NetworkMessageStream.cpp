
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
// NetworkMessageStream
//
// 23 March 2004 -- tds
//

#include "indri/NetworkMessageStream.hpp"
#include "indri/XMLWriter.hpp"
#include "indri/XMLReader.hpp"
#include "indri/indri-platform.h"
#include "lemur/Exception.hpp"
#include <iostream>

indri::net::XMLReplyReceiver::~XMLReplyReceiver() {
  delete _reply;
}

void indri::net::XMLReplyReceiver::wait( indri::net::NetworkMessageStream* stream ) {
  while( !done() && stream->alive() && !_exception.size() ) {
    stream->read(*this);
  }
  
  if( _exception.size() )
    LEMUR_THROW( LEMUR_NETWORK_ERROR, _exception );
}

int indri::net::NetworkMessageStream::_findEOL() {
  for( int i=_readPosition; i<_writePosition; i++ ) {
    if( _buffer.front()[i] == '\n' )
      return int(i);
  }

  return -1;
}

void indri::net::NetworkMessageStream::_cleanBuffer() {
  _buffer.remove( _readPosition );
  _writePosition -= _readPosition;
  _readPosition = 0;
}

int indri::net::NetworkMessageStream::_bufferLength() {
  return _writePosition - _readPosition;
}

indri::net::NetworkMessageStream::NetworkMessageStream( indri::net::NetworkStream* stream ) :
  _stream(stream)
{
  _readPosition = 0;
  _writePosition = 0;

  // give the buffer some space so we don't thrash around
  // allocating in little chunks
  _buffer.grow( 65536 );
}

bool indri::net::NetworkMessageStream::alive() {
  return _stream->alive();
}

void indri::net::NetworkMessageStream::read( MessageStreamHandler& handler ) {
  int endOfLine = _findEOL();
  int bytesRead = -1;

  while( endOfLine < 0 && bytesRead != 0 ) {
    _cleanBuffer();
    bytesRead = _stream->read( _buffer.front() + _writePosition, _buffer.size() - _writePosition );
    
    if( bytesRead <= 0 ) {
      bytesRead = 0;
      break;
    }

    _buffer.write( bytesRead );
    _writePosition += bytesRead;
    endOfLine = _findEOL();
  }

  if( endOfLine < 0 ) {
    // broke out of loop because we didn't receive a command, but
    // the socket returned 0 (other side closed connection)
    _stream->close();
    return;
  }

  assert( _readPosition <= _writePosition );

  int length = endOfLine - _readPosition;
  if( length < 4 )
    LEMUR_THROW( LEMUR_NETWORK_ERROR, "Malformed network packet: header line less than 4 bytes" );

  if( strncmp( "XREQ", _buffer.front() + _readPosition, 4 ) == 0 ) {
    int length = strtol( _buffer.front() + _readPosition + 4, 0, 10 );
    _readPosition = endOfLine + 1;
    
    if( _bufferLength() < length ) {
      _cleanBuffer();
      _buffer.grow( length );
          int toRead = length - _bufferLength();
      bytesRead = _stream->blockingRead( _buffer.front() + _writePosition, length - _bufferLength() );

      if( bytesRead <= 0 ) {
        _stream->close();
        return;
      }
          if( bytesRead != toRead )
                LEMUR_THROW( LEMUR_NETWORK_ERROR, "Didn't read enough data" );

      _buffer.write(bytesRead);
      _writePosition += bytesRead;
    }

    indri::xml::XMLReader reader;
    indri::xml::XMLNode* node = reader.read( _buffer.front() + _readPosition, length );
    handler.request( node );
    delete node;
    _readPosition += length;
  } else if( strncmp( "XRPY", _buffer.front() + _readPosition, 4 ) == 0 ) {
    int length = strtol( _buffer.front() + _readPosition + 4, 0, 10 );
    _readPosition = endOfLine + 1;
    
    if( _bufferLength() < length ) {
      _cleanBuffer();
      _buffer.grow( length );
          int toRead = length - _bufferLength();
      bytesRead = _stream->blockingRead( _buffer.front() + _writePosition, length  - _bufferLength() );
      if( bytesRead <= 0 ) {
        _stream->close();
        return;
      }

          if( bytesRead != toRead )
                LEMUR_THROW( LEMUR_NETWORK_ERROR, "Didn't read enough data" );
      
      _buffer.write(bytesRead);
      _writePosition += bytesRead;
    }

    indri::xml::XMLReader reader;
    indri::xml::XMLNode* node = reader.read( _buffer.front() + _readPosition, length );
    handler.reply( node );
    _readPosition += length;
  } else if( strncmp( "BRPY", _buffer.front() + _readPosition, 4 ) == 0 ) {
    char* next;
    int length = strtol( _buffer.front() + _readPosition + 4, &next, 10 );
    _readPosition = endOfLine + 1;

    std::string name;
    name.assign( next + 1, _buffer.front() + endOfLine );

    if( _bufferLength() < length ) {
      _cleanBuffer();
      _buffer.grow( length );
        int toRead = length - _bufferLength();
      bytesRead = _stream->blockingRead( _buffer.front() + _writePosition, length - _bufferLength() );
      if( bytesRead <= 0 ) {
        _stream->close();
        return;
      } 
          if( bytesRead != toRead )
                LEMUR_THROW( LEMUR_NETWORK_ERROR, "Didn't read enough data" );

      _buffer.write(bytesRead);
      _writePosition += bytesRead;
    }

    handler.reply( name, _buffer.front() + _readPosition, length );
    _readPosition += length;
  } else if( strncmp( "RFIN", _buffer.front() + _readPosition, 4 ) == 0 ) {
    _readPosition = endOfLine + 1;
    handler.replyDone();
  } else if( strncmp( "ERR", _buffer.front() + _readPosition, 3 ) == 0 ) {
    std::string errorString;
    errorString.assign( _buffer.front() + _readPosition + 4, _buffer.front() + endOfLine );
    handler.error( errorString );
    _readPosition = endOfLine + 1;
    // close on error
    _stream->close();
  } else {
    _readPosition = endOfLine + 1;
    // unrecognized command
    _stream->close();
  }

  assert( _readPosition <= _writePosition );
}

void indri::net::NetworkMessageStream::request( indri::xml::XMLNode* messageNode ) {
  indri::xml::XMLWriter writer(messageNode);
  std::string body;
  writer.write( body );
  
  std::string header = "XREQ ";
  header += i64_to_string( body.size() );
  header += "\n";

  _stream->write( header.c_str(), header.length() );
  _stream->blockingWrite( body.c_str(), body.length() );
}

void indri::net::NetworkMessageStream::reply( indri::xml::XMLNode* replyNode ) {
  indri::xml::XMLWriter writer(replyNode);
  std::string body;
  writer.write(body);

  std::string header = "XRPY ";
  header += i64_to_string( body.size() );
  header += "\n";

  _stream->write( header.c_str(), header.size() );
  _stream->blockingWrite( body.c_str(), body.length() );
}

void indri::net::NetworkMessageStream::reply( const std::string& name, const void* buffer, unsigned int size ) {
  std::string header = "BRPY ";
  header += i64_to_string( size );
  header += " ";
  header += name;
  header += "\n";

  _stream->write( header.c_str(), header.length() );
  _stream->blockingWrite( buffer, size );
}

void indri::net::NetworkMessageStream::replyDone() {
  _stream->write( "RFIN\n", 5 );
}

void indri::net::NetworkMessageStream::error( const std::string& errorMessage ) {
  std::string fullMessage;
  fullMessage = "ERR ";
  fullMessage += errorMessage;
  fullMessage += "\n";

  _stream->write( fullMessage.c_str(), fullMessage.length() );
}

indri::thread::Lockable& indri::net::NetworkMessageStream::mutex() {
  return _lock;
}



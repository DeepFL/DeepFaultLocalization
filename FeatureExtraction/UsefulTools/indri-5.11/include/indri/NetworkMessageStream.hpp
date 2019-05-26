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

#ifndef INDRI_NETWORKMESSAGESTREAM_HPP
#define INDRI_NETWORKMESSAGESTREAM_HPP

#include "indri/NetworkStream.hpp"
#include "indri/Buffer.hpp"
#include "indri/Mutex.hpp"
#include "indri/XMLNode.hpp"
namespace indri
{
  /// indri network components
  namespace net
  {
    
    class MessageStreamHandler {
    public:
      virtual void request( indri::xml::XMLNode* node ) = 0;
      virtual void reply( indri::xml::XMLNode* node ) = 0;
      virtual void reply( const std::string& name, const void* buffer, unsigned int length ) = 0;
      virtual void replyDone() = 0;
      virtual void error( const std::string& e ) = 0;
    };

    class XMLReplyReceiver : public MessageStreamHandler {
    private:
      bool _done;
      std::string _exception;
      indri::xml::XMLNode* _reply;

    public:
      XMLReplyReceiver() : _done(false), _reply(0) {}
      ~XMLReplyReceiver();

      void request( indri::xml::XMLNode* node ) {
        _exception = "XMLReplyReceiver: doesn't accept requests";
        _done = true;
      }

      void reply( indri::xml::XMLNode* reply ) {
        _reply = reply;
      }

      void reply( const std::string& name, const void* buffer, unsigned int length ) {
        _exception = "XMLReplyReceiver: should only get xml replies";
        _done = true;
      }

      void replyDone() {
        _done = true;
      }

      void error( const std::string& e ) {
        _exception = e;
        _done = true;
      }

      indri::xml::XMLNode* getReply() {
        return _reply;
      }

      bool done() {
        return _done;
      }

      void wait( class NetworkMessageStream* stream );
    };

    class NetworkMessageStream {
    private:
      indri::thread::Mutex _lock;
      indri::utility::Buffer _buffer;
      NetworkStream* _stream;
      int _readPosition;
      int _writePosition;

      int _findEOL();
      void _cleanBuffer();
      int _bufferLength();

    public:
      NetworkMessageStream( NetworkStream* stream );
      void read( MessageStreamHandler& handler );
      void request( indri::xml::XMLNode* messageNode );
      void reply( indri::xml::XMLNode* replyNode );
      void reply( const std::string& name, const void* buffer, unsigned int size );
      void replyDone();
      void error( const std::string& errorMessage );
      bool alive();
      indri::thread::Lockable& mutex();
    };
  }
}

#endif // INDRI_NETWORKMESSAGESTREAM_HPP

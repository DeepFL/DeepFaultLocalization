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
// indrid
//
// 23 March 2004 -- tds
//
// Listens on a socket (on a port defined with the -port parameter)
// for requests from an Indri query client, and processes them as they arrive.
//
/*! \page IndriDaemon IndriDaemon query server
<p> This application opens an Indri Repository and listens on a socket
for requests from an Indri query client, and processes them as they arrive. 

<H3>IndriDaemon Parameters</H3>
<dl>
<dt>index</dt>
<dd> path to the Indri Repository to act as server for. Specified as
&lt;index&gt;/path/to/repository&lt;/index&gt; in the parameter file and
as <tt>-index=/path/to/repository</tt> on the command line.
</dd>
<dt>port</dt>
<dd> an integer value specifying the port number to use.Specified as
&lt;port&gt;number&lt;/port&gt; in the parameter file and as
<tt>-port=number</tt> on the command line. </dd> 
</dl>
 */

#include "indri/indri-platform.h"
#include "lemur/lemur-compat.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/NetworkStream.hpp"
#include "indri/NetworkListener.hpp"
#include "indri/NetworkServerStub.hpp"
#include "indri/Parameters.hpp"
#include "lemur/Exception.hpp"
#include "indri/Mutex.hpp"
#include "indri/ScopedLock.hpp"
#include <time.h>

//
// verbose
//

static bool verbose = false;
static indri::thread::Mutex loglock;

//
// log_message
//

void log_message( const char* peer, const char* message ) {
  indri::thread::ScopedLock lock( loglock );
  time_t utc;

  time( &utc );
  struct tm* now = localtime( &utc );
  std::string timestring = asctime(now);
  
  // trim ending if necessary
  if( timestring[ timestring.size()-1 ] == '\n' ) {
    timestring.resize( timestring.size()-1 );    
  }

  std::cout << peer << " [" << timestring << "]: " << message << std::endl;
}

//
// connection_info
//

struct connection_info {
  bool active;
  indri::thread::Thread* thread;
  indri::net::NetworkStream* stream;
  indri::server::LocalQueryServer* server;
};

//
// connection_thread
//

void connection_thread( void* c ) {
  connection_info* info = (connection_info*) c;

  indri::net::NetworkMessageStream messageStream( info->stream );
  indri::net::NetworkServerStub stub( info->server, &messageStream );
  std::string peer = info->stream->peer();

  log_message( peer.c_str(), "connected" );
  
  while( messageStream.alive() ) {
    log_message( peer.c_str(), "received request" );
    messageStream.read( stub ); 
    log_message( peer.c_str(), "sent response" );
  }

  info->active = false;
  log_message( peer.c_str(), "disconnected" );
}

//
// build_connection
//

connection_info* build_connection( indri::net::NetworkStream* stream, indri::server::LocalQueryServer* server ) {
  connection_info* info = new connection_info;
  
  info->stream = stream;
  info->server = server;
  info->active = true;

  indri::thread::Thread* thread = new indri::thread::Thread( connection_thread, info );
  info->thread = thread;

  return info;
}

//
// clean_connections
//

void clean_connections( std::list<connection_info*>& connections ) {
  std::list<connection_info*>::iterator iter;
  std::list<connection_info*>::iterator removeMe;
  
  for( iter = connections.begin(); iter != connections.end(); ) {
    if( (*iter)->active == false ) {
      removeMe = iter;
      iter++;

      connection_info* info = (*removeMe);
      info->thread->join();
      delete info->stream;
      delete info->thread;
      delete info;

      connections.erase( removeMe );
    } else {
      iter++;
    }
  }
}

//
// wait_connections
//

void wait_connections( std::list<connection_info*>& connections ) {
  while( connections.size() ) {
    clean_connections( connections );
    indri::thread::Thread::yield();
  }
}

//
// main
//

int main( int argc, char* argv[] ) {
  try {
    indri::api::Parameters& parameters = indri::api::Parameters::instance();
    parameters.loadCommandLine( argc, argv );

    indri::net::NetworkListener listener;
    int port = parameters.get( "port", INDRID_PORT );
    verbose = parameters.get( "verbose", false );
    std::string repositoryPath = parameters["index"];

    // wrap the index in a local server that the stub can talk to
    indri::collection::Repository* repository = new indri::collection::Repository();
    // pass in parameters, in case anyone wants to do query side stopping.
    repository->openRead( repositoryPath, &parameters );
    indri::server::LocalQueryServer server( *repository );

    // open for business
    listener.listen( port );
    indri::net::NetworkStream* connection;

    std::list<connection_info*> connections;

    // this handles the threading issue by only allowing one
    // connection at a time; for our current uses this is fine
    while( (connection = listener.accept()) ) {
      connection_info* info = build_connection( connection, &server );
      connections.push_back( info );

      clean_connections( connections );
    }

    wait_connections( connections );
    repository->close();
    delete repository;
    return 0;
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  }
}



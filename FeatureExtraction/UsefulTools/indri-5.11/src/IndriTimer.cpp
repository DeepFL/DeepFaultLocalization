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
// IndriTimer
//
// 13 August 2004 -- tds
//

#include "indri/IndriTimer.hpp"
#include "lemur/lemur-compat.hpp"
#include <iomanip>
#ifndef WIN32
#include <sys/time.h>
#endif

//
// IndriTimer
//

indri::utility::IndriTimer::IndriTimer()
  :
  _start(0),
  _elapsed(0),
  _stopped(true)
{
}

//
// currentTime
//

UINT64 indri::utility::IndriTimer::currentTime() {
#ifdef WIN32
  FILETIME filetime;
  ::GetSystemTimeAsFileTime( &filetime );

  // this time is now in 100 nanosecond increments
  UINT64 result = filetime.dwHighDateTime;
  result <<= 32;
  result += filetime.dwLowDateTime;

  return result / 10;
#else
  struct timeval tv;
  gettimeofday(&tv, 0);
  UINT64 seconds = tv.tv_sec;
  UINT64 million = 1000000;
  UINT64 microseconds = tv.tv_usec;

  return seconds * million + microseconds;
#endif
}

//
// start
//

void indri::utility::IndriTimer::start() {
  _stopped = false;
  _start = currentTime();
}

//
// stop
//

void indri::utility::IndriTimer::stop() {
  _elapsed += (currentTime() - _start);
  _start = 0;
  _stopped = true;
}

//
// reset
//

void indri::utility::IndriTimer::reset() {
  _stopped = true;
  _start = 0;
  _elapsed = 0;
}

//
// elapsedTime
//

UINT64 indri::utility::IndriTimer::elapsedTime() const {
  UINT64 total = _elapsed;

  if( !_stopped ) {
    total += (currentTime() - _start);
  }

  return total;
}

//
// printElapsedMicroseconds
//

void indri::utility::IndriTimer::printElapsedMicroseconds( std::ostream& out ) const {
  UINT64 elapsed = elapsedTime();
  const UINT64 million = 1000000;

  int minutes = int(elapsed / (60 * million));
  int seconds = int(elapsed/million - 60*minutes);
  int microseconds = int(elapsed % million);
  
  out << minutes
      << ":"
      << std::setw(2) << std::setfill('0')
      << seconds
      << "."
      << std::setw(6) << std::setfill('0')
      << microseconds;
}

//
// printElapsedSeconds
//

void indri::utility::IndriTimer::printElapsedSeconds( std::ostream& out ) const {
  UINT64 elapsed = elapsedTime();
  const UINT64 million = 1000000;

  int minutes = int(elapsed / (60 * million));
  int seconds = int(elapsed/million - 60*minutes);
  int microseconds = int(elapsed % million);
  
  out << minutes
      << ":"
      << std::setw(2) << std::setfill('0')
      << seconds;
}


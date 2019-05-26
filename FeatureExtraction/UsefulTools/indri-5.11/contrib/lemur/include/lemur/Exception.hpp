/*==========================================================================
 * Copyright (c) 2001 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

#ifndef _EXCEPTION_HPP
#define _EXCEPTION_HPP
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

namespace lemur 
{
  namespace api 
  {
    /// Error codes.
    typedef unsigned long LemurErrorType; 
    /// Default Exception class
    /*!
      This is the base Exception class that all other exceptions should
      inherit from. 
      Some guidelines for using Exception:

  
      (1) Define your main function as "int AppMain(int argc, char *argv[])"
      rather than the normal "main" function. 
      (2) Use the LEMUR_THROW and LEMUR_RETHROW macros to throw the exception, and
      to pass it along to the next level's handler, respectively.
      To use LEMUR_THROW, include the appropriate Lemur error code, and whatever
      string description you want.   File and line information are automatically
      included.
      The error code can be from the standard Lemur list (see below).
      (3)  Here's an example of how you might use LEMUR_THROW to generate an exception,
      and LEMUR_RETHROW to pass it along to the next handler.

      <PRE> 
      try { 
      ... ... 
      // within the index open routine, this exception might be thrown:
      LEMUR_THROW(LEMUR_IO_ERROR, "The index file does not exist");
      }
      catch  (Exception &ex) {
      LEMUR_RETHROW(ex, "Could not start retrieval program.");
      }

      </PRE>
      In general, consistent use of LEMUR_THROW and LEMUR_RETHROW will result in a 
      nested series of exception messages, showing successively lower
      levels of exception information, allowing easy tracing of the failure path.

      (4) If the exception is not caught in the application, it will be
      caught be the main function in the lemur toolkit. The default exception handler prints the following
      message on stderr and terminates the program. 
      <PRE>
      Exception FileName.cpp(#linenum): The index file does not exist
      Program aborted due to exception
      </PRE>

      (5) You can define more specific exceptions by subclassing Exception.
      All exceptions will be caught by the default main function if not caught by an application.


    */

    class Exception {
    public:
      Exception(const char *throwerString=" unknown thrower", const char *whatString="unknown exception") {
        _what = throwerString;
        _what += ": ";
        _what += whatString;
      }

      Exception( const std::string& whoString, int whereLine, 
                 const std::string& whatString, LemurErrorType code )
      {
        std::stringstream lineString;
        lineString << whereLine;

        _what = whoString + "(" + lineString.str() + ")" + ": " + whatString;
        _code = code;
      }

      Exception( const std::string& whoString, int whereLine,
                 const std::string& whatString, const Exception& inner )
      {
        std::stringstream lineString;
        lineString << whereLine;

        _what = whoString + "(" + lineString.str() + "): " + whatString + "\n\t" + inner.what();
        _code = inner.code();
      }

      ~Exception() {}

      inline void writeMessage(std::ostream &os = std::cerr)
      {
        os << "Exception [code = " << _code << "]" << std::endl << _what << std::endl;
      }

      const std::string& what() const {
        return _what;
      }

      LemurErrorType code() const {
        return _code;
      }

    private:
      std::string _what;
      LemurErrorType _code;
    };

#define LEMUR_ABORT( e )                  { std::cerr << e.what() << std::endl; exit(-1); }
#define LEMUR_THROW_LINE( code, text, file, line )  throw lemur::api::Exception( file, line, std::string() + text, (code) )
#define LEMUR_THROW(code, text)  LEMUR_THROW_LINE(code, text, __FILE__, __LINE__)
#define LEMUR_RETHROW_LINE( e, text, file, line )   throw lemur::api::Exception( file, line, (std::string() + text), (e) )
#define LEMUR_RETHROW( e, text)  LEMUR_RETHROW_LINE(e, text, __FILE__, __LINE__)

#define LEMUR_GENERIC_ERROR               ((lemur::api::LemurErrorType)0xFFFFFFFF)
#define LEMUR_MISSING_PARAMETER_ERROR     ((lemur::api::LemurErrorType)0xFFFFFFFE)
#define LEMUR_BAD_PARAMETER_ERROR         ((lemur::api::LemurErrorType)0xFFFFFFF7)
#define LEMUR_PARSE_ERROR                 ((lemur::api::LemurErrorType)0xFFFFFFFD)
#define LEMUR_KEYFILE_IO_ERROR            ((lemur::api::LemurErrorType)0xFFFFFFFC)
#define LEMUR_IO_ERROR                    ((lemur::api::LemurErrorType)0xFFFFFFFB)
#define LEMUR_RUNTIME_ERROR               ((lemur::api::LemurErrorType)0xFFFFFFFA)
#define LEMUR_NETWORK_ERROR               ((lemur::api::LemurErrorType)0xFFFFFFF9)
#define LEMUR_INTERNAL_ERROR              ((lemur::api::LemurErrorType)0xFFFFFFF8)
  }
}

#endif

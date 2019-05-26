/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// NumericFieldAnnotator
//
// 25 May 2004 -- tds
//

#ifndef INDRI_NUMERICFIELDANNOTATOR_HPP
#define INDRI_NUMERICFIELDANNOTATOR_HPP
namespace indri
{
  namespace parse
  {

    class NumericFieldAnnotator : public Transformation {
    private:
      ObjectHandler<indri::api::ParsedDocument>* _handler;
      std::string& _field;
      bool _foundNonNumeric;
      // a buffer for copying the number
      int _numberCopyLength;
      char * _numberCopy;
      
    public:
      NumericFieldAnnotator( std::string& field ) :
        _handler(0),
        _field(field),
        _foundNonNumeric(false),
        _numberCopyLength(1024) // should be large enough
      {
        _numberCopy = new char[ _numberCopyLength + 1 ];
      }
      
      ~NumericFieldAnnotator() {
        delete [] _numberCopy;
      }

      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document ) {
        for( size_t i=0; i<document->tags.size(); i++ ) {
          _foundNonNumeric = false;
          TagExtent * extent = document->tags[i];

          if( _field == extent->name && extent->begin != extent->end ) {
            char* numberText = document->terms[ extent->begin ]; 
            // check for non-numeric characters
            char * begin = numberText;
            char * end = numberText;
            // find the first acceptable character 
            for ( begin = numberText; *begin != '\0'; begin++ ) {
              if ( *begin == '-' ||
                   (*begin >= '0' && *begin <= '9') ) {
                break;
              } else {
                if ( _foundNonNumeric == false ) {
                  _foundNonNumeric = true;
                }
              }
            }
            // find the last acceptable numeric character
            for ( end = begin; *end != '\0'; end++ ) {
              if (! ( *end == '-' ||
                      // *end == '.' || // for now, the recognizer only handles integers
                      (*end >= '0' && *end <= '9') 
                      ) ) {
                break;
              }
            }
            INT64 value = 0;
            int len = end - begin;
            if ( len > 0 ) {
              // make a copy
              if ( len > _numberCopyLength ) {
                len = _numberCopyLength;
              }
              _numberCopy[ len ] = '\0';
              strncpy( _numberCopy, begin, len );
              // convert the number
              value = string_to_i64( _numberCopy );
            }
            extent->number = value;
          }
        }

        return document;
      }

      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler ) {
        _handler = &handler;
      }

      void handle( indri::api::ParsedDocument* document ) {
        _handler->handle( transform( document ) );
      }
    };
 
  }
}

#endif // INDRI_NUMERICFIELDANNOTATOR_HPP



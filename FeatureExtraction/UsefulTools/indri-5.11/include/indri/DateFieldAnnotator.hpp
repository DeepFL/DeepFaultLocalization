/*==========================================================================
 * Copyright (c) 2003-2007 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// DateFieldAnnotator
//
// 20 Mar 2007 -- dmf
//
//FIX the href if necessary.
/*! \page DateFields Date Fields

<p>The Indri DateFieldAnnotator converts date strings in a numeric field to the number of days since 01/01/1600. This enables the use of the <a href="http://www.lemurproject.org/lemur/IndriQueryLanguage.php#numeric">#date operators in the Indri query language</a> when the field being indexed is named "date".

Acceptable date formats:
<ul>
<li>11-01-2004 (DD-MM-YYYY)
<li>11-JAN-2004 (DD-Month-YYYY)
<li>2004-01-11 (YYYY-MM-DD)
<li>January 11 2004 (Month DD YYYY)
<li>11 January 2004 (DD Month YYYY)
<li>01/11/2004 (MM/DD/YYYY)
<li>2004/01/11 (YYYY/MM/DD)
<li>20040111 (YYYYMMDD)
</ul>
<p>Four digit years are required. The leading 0 can be ommited in all formats except YYYYMMDD, eg, 1/11/2004. Month names may be an abbreviation or the full name.

<p>Specify in the index build parameters file with:
<pre>
<field>
   <name>date</name>
   <numeric>true</numeric>
   <parserName>DateFieldAnnotator</parserName>
</field>
<pre>
 */

#ifndef INDRI_DATEFIELDANNOTATOR_HPP
#define INDRI_DATEFIELDANNOTATOR_HPP
#include "indri/DateParse.hpp"
namespace indri
{
  namespace parse
  {

    class DateFieldAnnotator : public Transformation {
    private:
      ObjectHandler<indri::api::ParsedDocument>* _handler;
      std::string& _field;
      
      void _parseDate(const std::string &date, TagExtent *extent) const {
        try {
          std::string day, month, year;
          // is it a slash-date, dash-date, or space-date, or a single number?
          if (extent->begin == extent->end-1) {
            // single number date YYYYMMDD
            year = date.substr( 0, 4 ); 
            month = date.substr( 4, 2 );
            day = date.substr( 6, 2 );
            extent->number = indri::parse::DateParse::convertDate( year, month, day );
          } else {
            bool swapMonth = false;
            std::string delim = "/";
            int firstDash;        
            int secondDash;
            if ((firstDash = date.find(delim)) == std::string::npos) {
              delim = "-";
              if ((firstDash = date.find(delim)) == std::string::npos) {
                delim = " ";
                if ((firstDash = date.find(delim)) == std::string::npos) 
                  // nothing to parse
                  return;
                else {
                  // space date is Month DD YYYY or DD Month YYYY
                  if (firstDash > 2) swapMonth = true;
                }
              }
            } else {
              // slash date is MM/DD/YYYY
              swapMonth = true;
            }

            secondDash = date.find(delim, firstDash+1);
            day = date.substr( 0, firstDash ); 
            month = date.substr( firstDash+1, secondDash-firstDash-1 );
            year = date.substr( secondDash+1 );
          
            if (firstDash == 4)
              // YYYY-MM-DD or YYYY/MM/DD
              extent->number = indri::parse::DateParse::convertDate( day, month, year );
            else {
              // hack for 2 digit years in WSJ
              if (year.length() == 2) year = "19" + year;
              if (swapMonth)
                //  Month DD YYYY MM-DD-YY
                extent->number = indri::parse::DateParse::convertDate( year, day, month );
              else  
                extent->number = indri::parse::DateParse::convertDate( year, month, day );
            }
          }
        } catch (std::out_of_range ex) {
          // silently ignore malformed data
          std::cerr << "Ignoring invalid date field data: "<<date<< std::endl;
        }
      }
      
    public:
      DateFieldAnnotator( std::string& field ) :
        _handler(0), _field(field) {
      }
      
      ~DateFieldAnnotator() {
      }

      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document ) {
        for( size_t i=0; i<document->tags.size(); i++ ) {
          TagExtent * extent = document->tags[i];
          // reparse from the document text
          if( _field == extent->name ) {
            int dateStart = document->positions[extent->begin].begin; 
            int dateEnd = document->positions[extent->end-1].end;
            int dateLen = dateEnd - dateStart ;
            std::string date;
            date.assign(document->text + dateStart, dateLen);
            _parseDate(date, extent);
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

#endif // INDRI_DATEFIELDANNOTATOR_HPP



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
// DateParse
//
// 13 April 2004 -- tds
//

#ifndef INDRI_DATEPARSE_HPP
#define INDRI_DATEPARSE_HPP
namespace indri
{
  namespace parse
  {
    
    class DateParse {
    private:
      static int _parseYear( const std::string& year ) {
        return atoi( year.c_str() );
      }

      static int _parseDay( const std::string& day ) {
        return atoi( day.c_str() );
      }

      static int _parseMonth( const std::string& month ) {
        if( month[0] >= '0' && month[0] <= '9' )
          return atoi( month.c_str() );

        char prefix[4];
        memset( prefix, 0, 4 );

        for( unsigned int i=0; i<4 && i<month.size(); i++ ) {
          prefix[i] = tolower( month[i] );
        }
    
        // j f m a m j j a s o n d
        if( prefix[0] == 'j' ) {
          if( prefix[1] == 'a' ) return 1; // january
          if( prefix[2] == 'n' ) return 6; // june
          if( prefix[2] == 'l' ) return 7; // july
          return 0;
        } else if( prefix[0] == 'f' ) {
          return 2; // february
        } else if( prefix[0] == 'a' ) {
          if( prefix[1] == 'p' ) return 4; // april
          if( prefix[1] == 'u' ) return 8; // august
          return 0;
        } else if( prefix[0] == 'm' ) {
          if( prefix[2] == 'r' ) return 3; // march
          if( prefix[2] == 'y' ) return 5; // may
          return 0;
        } else if( prefix[0] == 's' ) {
          return 9; // september
        } else if( prefix[0] == 'o' ) {
          return 10; // october
        } else if( prefix[0] == 'n' ) {
          return 11; // november
        } else if( prefix[0] == 'd' ) {
          return 12;
        }

        return 0;
      }

    public:
      // converts year, month, and day from parser to the number of days since 1600
      static UINT64 convertDate( const std::string& year, const std::string& month, const std::string& day ) {
        int numYear = _parseYear( year );
        int numMonth = _parseMonth( month );
        int numDay = _parseDay( day );

        int monthCumulativeDays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
        if( numMonth == 0 || numYear < 1601 || numDay == 0 )
          return 0;

        // let's do days since 1600 here
        UINT64 totalDays = 0;
        UINT64 yearsSince = numYear - 1600;
    
        // every 4 years is a leap year, except every 100 years is not, except every 400 years actually is...
        UINT64 leapDays = yearsSince / 4 -
          yearsSince / 100 +
          yearsSince / 400 + 
          1; // 1 leap day in 1600

        // if this year is a leap year and it's past febuary, add an extra year on
        if( numMonth > 2 && (numYear % 4) == 0 && ( ((numYear % 100) != 0) || (numYear % 400) == 0) )
          leapDays++;

        return (numDay-1) + monthCumulativeDays[numMonth-1] + yearsSince*365 + leapDays;
      }
    };
  }
}

#endif // INDRI_DATEPARSE_HPP


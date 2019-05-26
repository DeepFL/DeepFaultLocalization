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
// StopperTransformation
//
// 13 May 2004 -- tds
//

#ifndef INDRI_STOPPERTRANSFORMATION_HPP
#define INDRI_STOPPERTRANSFORMATION_HPP

#include "indri/Transformation.hpp"
#include <string>
#include <vector>
#include "indri/Parameters.hpp"

#ifdef WIN32
#include <hash_set>
#else
// Move this somewhere
#ifndef HAVE_GCC_VERSION
#define HAVE_GCC_VERSION(MAJOR, MINOR) \
  (__GNUC__ > (MAJOR) || (__GNUC__ == (MAJOR) && __GNUC_MINOR__ >= (MINOR)))
#endif /* ! HAVE_GCC_VERSION */
#if HAVE_GCC_VERSION(4,3)
// if GCC 4.3+
#include <tr1/unordered_set>
#else
#include <ext/hash_set>
#endif
// 3.3 does not use __gnu_cxx, 3.4+ does.
using namespace __gnu_cxx;
#endif

namespace indri
{
  namespace parse
  {
    
    class StopperTransformation : public Transformation {
    private:
      ObjectHandler<indri::api::ParsedDocument>* _handler;
#ifdef WIN32
      struct ltstr {
        bool operator()( const char* s1,  const char* s2) const {
          return (strcmp(s1, s2) < 0);
        }
      };
      //studio 7 hash_set provides hash_compare, rather than hash
      // needing an < predicate, rather than an == predicate.
      typedef stdext::hash_set< const char *, stdext::hash_compare< const char *, ltstr> > dictTable;
#else
      struct eqstr {
        bool operator()(char* s1, char* s2) const {
          return strcmp(s1, s2) == 0;
        }
      };
#if HAVE_GCC_VERSION(4,3)
      typedef std::tr1::unordered_set<char *, std::tr1::hash<std::string>, eqstr> dictTable;
#else
      typedef hash_set<char *, hash<char *>, eqstr> dictTable;
#endif
#endif

      dictTable _table;

    public:
      StopperTransformation();
      StopperTransformation( const std::vector<std::string>& stopwords );
      StopperTransformation( const std::vector<const char*>& stopwords );
      StopperTransformation( const std::vector<char*>& stopwords );
      StopperTransformation( indri::api::Parameters& stopwords );
      ~StopperTransformation();

      void read( const std::vector<std::string>& stopwords );
      void read( const std::vector<const char*>& stopwords );
      void read( const std::vector<char*>& stopwords );
      void read( const std::string& filename );
      void read( indri::api::Parameters& stopwords );

      indri::api::ParsedDocument* transform( indri::api::ParsedDocument* document );

      void handle( indri::api::ParsedDocument* document );
      void setHandler( ObjectHandler<indri::api::ParsedDocument>& handler );
    };
  }
}

#endif // INDRI_STOPPERTRANSFORMATION_HPP


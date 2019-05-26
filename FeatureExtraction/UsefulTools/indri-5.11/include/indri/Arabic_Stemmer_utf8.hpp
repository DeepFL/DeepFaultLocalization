/*==========================================================================
 * Copyright (c) 2012 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

/**************************************************************************/
/**************************************************************************/
/**************            ARABIC STEMMER HEADER FILE         *************/
/**************************************************************************/
/**************************************************************************/

/*

Copyright (c) 2001 UMASS CIIR All rights reserved.
Written by Nick Dufresne (nickd@cs.brandeis.edu)
	
08/01/2005 -- rewrite as standalone thread safe clase.	
*/


#ifndef _ARABIC_STEMMER_UTF8_HPP
#define _ARABIC_STEMMER_UTF8_HPP
#include <iostream>
#include <string>
#include <cstring>
#include <set>
#ifdef WIN32
#include <hash_map>
#else
// Move this somewhere
#ifndef HAVE_GCC_VERSION
#define HAVE_GCC_VERSION(MAJOR, MINOR) \
  (__GNUC__ > (MAJOR) || (__GNUC__ == (MAJOR) && __GNUC_MINOR__ >= (MINOR)))
#endif /* ! HAVE_GCC_VERSION */
#if HAVE_GCC_VERSION(4,3)
// if GCC 4.3+
#include <tr1/unordered_map>
#else
#include <ext/hash_map>
#endif
// 3.3 does not use __gnu_cxx, 3.4+ does.
using namespace __gnu_cxx;
#endif

#include "indri/UTF8Transcoder.hpp"
#include "indri/uint64comp.hpp"

namespace indri {
  namespace parse {
    class Arabic_Stemmer_utf8 {
    public:
      Arabic_Stemmer_utf8(std::string stemFunc);
      ~Arabic_Stemmer_utf8();
      // stem a term.
      void stemTerm(char *, char *);

    private:
      UTF8Transcoder _transcoder;
      void createStemmerTransitionTables();
      void (Arabic_Stemmer_utf8::*stem_fct_unicode)(UINT64 *, UINT64 *) ;

      // New stem functions, work on UINT64, null-terminated arrays
      void arabic_stop_unicode(UINT64 *, UINT64 *);  // only removes stops
      void no_stem_unicode(UINT64 *, UINT64 *) ;	   // doesn't do anything

      // Normalize arabic word
      void arabic_norm2_unicode(UINT64 *, UINT64 *);
      void arabic_norm2_stop_unicode(UINT64 *, UINT64 *);
      void arabic_light10_unicode(UINT64 *, UINT64 *);
      void arabic_light10_stop_unicode(UINT64 *, UINT64 *);

      // stopwords hash table.
      struct ltstr_unicode {
        bool operator()(const UINT64* s1, const UINT64* s2) const {
          return Uint64Comp::u_strcmp(s1, s2) < 0;
        }
      };

      std::set<const UINT64 *, ltstr_unicode> stop_words_set_unicode;

      struct eqstr {
        bool operator()(const UINT64 s1, const UINT64 s2) const {
          return (s1 == s2);
        }
      };

      // Hash function adapted from  http://drdobbs.com/cpp/198800559?pgno=2
      struct uint64Hasher {
        size_t operator()( const UINT64& itemToHash ) const {
            return size_t( itemToHash );
        }
      };
#ifdef WIN32
      //FIXME: requires extra testing in newest studio.
      struct ltstr {
        bool operator()(const UINT64 s1, const UINT64 s2) const {
          return Uint64Comp::u_strcmp(&s1, &s2) < 0;
        }
      };

      typedef stdext::hash_map<UINT64, UINT64, stdext::hash_compare<UINT64, ltstr> > transitionTable;
#else
// ifdefs copied from Krovetz Stemmer, without windows part - and transformed to use UINT64
#if HAVE_GCC_VERSION(4,3)
      typedef std::tr1::unordered_map<UINT64, UINT64, uint64Hasher, eqstr> transitionTable;
#else
      typedef hash_map<UINT64, UINT64, uint64Hasher, eqstr> transitionTable;
#endif
#endif
      // Three different transition tables
      transitionTable normCharTable;
      transitionTable norm3CharTable;
      transitionTable arabicVowelTable;

      bool on_stop_list_unicode (UINT64 *word);

      // pointer to member function
      typedef struct {
        const char *option;
        void (Arabic_Stemmer_utf8::*stem_fct_unicode)(UINT64 *, UINT64 *) ;
      } stem_info_u_t;
      static stem_info_u_t stemtable_unicode[];

      static const UINT64 *allDefArticles[];
      static const UINT64 defArticle0[];
      static const UINT64 defArticle1[];
      static const UINT64 defArticle2[];
      static const UINT64 defArticle3[];
      static const UINT64 defArticle4[];
      static const UINT64 defArticle5[];

      static const UINT64 *allSuffixes[];
      static const UINT64 suffix0[];
      static const UINT64 suffix1[];
      static const UINT64 suffix2[];
      static const UINT64 suffix3[];
      static const UINT64 suffix4[];
      static const UINT64 suffix5[];
      static const UINT64 suffix6[];
      static const UINT64 suffix7[];
      static const UINT64 suffix8[];
      static const UINT64 suffix9[];


      static const UINT64 *allStopWords[];
      static const UINT64 stopWord0[];
      static const UINT64 stopWord1[];
      static const UINT64 stopWord2[];
      static const UINT64 stopWord3[];
      static const UINT64 stopWord4[];
      static const UINT64 stopWord5[];
      static const UINT64 stopWord6[];
      static const UINT64 stopWord7[];
      static const UINT64 stopWord8[];
      static const UINT64 stopWord9[];
      static const UINT64 stopWord10[];
      static const UINT64 stopWord11[];
      static const UINT64 stopWord12[];
      static const UINT64 stopWord13[];
      static const UINT64 stopWord14[];
      static const UINT64 stopWord15[];
      static const UINT64 stopWord16[];
      static const UINT64 stopWord17[];
      static const UINT64 stopWord18[];
      static const UINT64 stopWord19[];
      static const UINT64 stopWord20[];
      static const UINT64 stopWord21[];
      static const UINT64 stopWord22[];
      static const UINT64 stopWord23[];
      static const UINT64 stopWord24[];
      static const UINT64 stopWord25[];
      static const UINT64 stopWord26[];
      static const UINT64 stopWord27[];
      static const UINT64 stopWord28[];
      static const UINT64 stopWord29[];
      static const UINT64 stopWord30[];
      static const UINT64 stopWord31[];
      static const UINT64 stopWord32[];
      static const UINT64 stopWord33[];
      static const UINT64 stopWord34[];
      static const UINT64 stopWord35[];
      static const UINT64 stopWord36[];
      static const UINT64 stopWord37[];
      static const UINT64 stopWord38[];
      static const UINT64 stopWord39[];
      static const UINT64 stopWord40[];
      static const UINT64 stopWord41[];
      static const UINT64 stopWord42[];
      static const UINT64 stopWord43[];
      static const UINT64 stopWord44[];
      static const UINT64 stopWord45[];
      static const UINT64 stopWord46[];
      static const UINT64 stopWord47[];
      static const UINT64 stopWord48[];
      static const UINT64 stopWord49[];
      static const UINT64 stopWord50[];
      static const UINT64 stopWord51[];
      static const UINT64 stopWord52[];
      static const UINT64 stopWord53[];
      static const UINT64 stopWord54[];
      static const UINT64 stopWord55[];
      static const UINT64 stopWord56[];
      static const UINT64 stopWord57[];
      static const UINT64 stopWord58[];
      static const UINT64 stopWord59[];
      static const UINT64 stopWord60[];
      static const UINT64 stopWord61[];
      static const UINT64 stopWord62[];
      static const UINT64 stopWord63[];
      static const UINT64 stopWord64[];
      static const UINT64 stopWord65[];
      static const UINT64 stopWord66[];
      static const UINT64 stopWord67[];
      static const UINT64 stopWord68[];
      static const UINT64 stopWord69[];
      static const UINT64 stopWord70[];
      static const UINT64 stopWord71[];
      static const UINT64 stopWord72[];
      static const UINT64 stopWord73[];
      static const UINT64 stopWord74[];
      static const UINT64 stopWord75[];
      static const UINT64 stopWord76[];
      static const UINT64 stopWord77[];
      static const UINT64 stopWord78[];
      static const UINT64 stopWord79[];
      static const UINT64 stopWord80[];
      static const UINT64 stopWord81[];
      static const UINT64 stopWord82[];
      static const UINT64 stopWord83[];
      static const UINT64 stopWord84[];
      static const UINT64 stopWord85[];
      static const UINT64 stopWord86[];
      static const UINT64 stopWord87[];
      static const UINT64 stopWord88[];
      static const UINT64 stopWord89[];
      static const UINT64 stopWord90[];
      static const UINT64 stopWord91[];
      static const UINT64 stopWord92[];
      static const UINT64 stopWord93[];
      static const UINT64 stopWord94[];
      static const UINT64 stopWord95[];
      static const UINT64 stopWord96[];
      static const UINT64 stopWord97[];
      static const UINT64 stopWord98[];
      static const UINT64 stopWord99[];
      static const UINT64 stopWord100[];
      static const UINT64 stopWord101[];
      static const UINT64 stopWord102[];
      static const UINT64 stopWord103[];
      static const UINT64 stopWord104[];
      static const UINT64 stopWord105[];
      static const UINT64 stopWord106[];
      static const UINT64 stopWord107[];
      static const UINT64 stopWord108[];
      static const UINT64 stopWord109[];
      static const UINT64 stopWord110[];
      static const UINT64 stopWord111[];
      static const UINT64 stopWord112[];
      static const UINT64 stopWord113[];
      static const UINT64 stopWord114[];
      static const UINT64 stopWord115[];
      static const UINT64 stopWord116[];
      static const UINT64 stopWord117[];
      static const UINT64 stopWord118[];
      static const UINT64 stopWord119[];
      static const UINT64 stopWord120[];
      static const UINT64 stopWord121[];
      static const UINT64 stopWord122[];
      static const UINT64 stopWord123[];
      static const UINT64 stopWord124[];
      static const UINT64 stopWord125[];
      static const UINT64 stopWord126[];
      static const UINT64 stopWord127[];
      static const UINT64 stopWord128[];
      static const UINT64 stopWord129[];
      static const UINT64 stopWord130[];
      static const UINT64 stopWord131[];
      static const UINT64 stopWord132[];
      static const UINT64 stopWord133[];
      static const UINT64 stopWord134[];
      static const UINT64 stopWord135[];
      static const UINT64 stopWord136[];
      static const UINT64 stopWord137[];
      static const UINT64 stopWord138[];
      static const UINT64 stopWord139[];
      static const UINT64 stopWord140[];
      static const UINT64 stopWord141[];
      static const UINT64 stopWord142[];
      static const UINT64 stopWord143[];
      static const UINT64 stopWord144[];
      static const UINT64 stopWord145[];
      static const UINT64 stopWord146[];
      static const UINT64 stopWord147[];
      static const UINT64 stopWord148[];
      static const UINT64 stopWord149[];
      static const UINT64 stopWord150[];
      static const UINT64 stopWord151[];
      static const UINT64 stopWord152[];
      static const UINT64 stopWord153[];
      static const UINT64 stopWord154[];
      static const UINT64 stopWord155[];
      static const UINT64 stopWord156[];
      static const UINT64 stopWord157[];
      static const UINT64 stopWord158[];
      static const UINT64 stopWord159[];
      static const UINT64 stopWord160[];
      static const UINT64 stopWord161[];
      static const UINT64 stopWord162[];
      static const UINT64 stopWord163[];
      static const UINT64 stopWord164[];
      static const UINT64 stopWord165[];
      static const UINT64 stopWord166[];
      static const UINT64 stopWord167[];


      // 4 "null-terminated" arrays, they are all of the same size
      // In fact only the top two are in use; norm3Chars and arabicVowelChars are not in use
      static UINT64 allArabicChars[];
      static UINT64 normChars[];
      static UINT64 norm3Chars[];
      static UINT64 arabicVowelChars[];

//      int is_whitespace_unicode(const UINT64 c);
      void remove_definite_articles_unicode(UINT64 *word, UINT64 *result);
      void remove_all_suffixes_unicode(UINT64 *word, UINT64 *result, size_t lenlimit);

    };
  }
}

    
#endif

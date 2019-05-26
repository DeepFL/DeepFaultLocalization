/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */
// dmf
// C++ thread safe implementation of the Krovetz stemmer.
// requires no external data files.
// 07/29/2005
#ifndef _KROVETZ_STEMMER_H_
#define _KROVETZ_STEMMER_H_
#include <iostream>
#include <cstring>
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
#include "indri/Mutex.hpp"
#include "indri/ScopedLock.hpp"

namespace indri
{
  namespace parse 
  {
    class KrovetzStemmer 
    {
    public:
      KrovetzStemmer();
      ~KrovetzStemmer();
      /// maximum number of characters in a word to be stemmed.
      static const int MAX_WORD_LENGTH=25;
      /*!
        \brief stem a term using the Krovetz algorithm. 
        The stem returned may be longer than the input term.
        May return a pointer
        to the private attribute stem. Performs case normalization on its
        input argument. Return values should be copied before
        calling the method again.
        @param term the term to stem
        @return the stemmed term or the original term if no stemming was 
        performed.
       */
      char * kstem_stemmer(char *term);
      /*!
        \brief stem a term using the Krovetz algorithm into the specified
        buffer.
        The stem returned may be longer than the input term.
        Performs case normalization on its input argument. 
        @param term the term to stem
        @param buffer the buffer to hold the stemmed term. The buffer should
        be at MAX_WORD_LENGTH or larger.
        @return the number of characters written to the buffer, including
        the terminating '\\0'. If 0, the caller should use the value in term.
       */
      int kstem_stem_tobuffer(char *term, char *buffer);
      /*!
        \brief Add an entry to the stemmer's dictionary table.
        @param variant the spelling for the entry.
        @param word the stem to use for the variant. If "", the variant
        stems to itself.
        @param exc Is the word an exception to the spelling rules.
       */
      void kstem_add_table_entry(const char* variant, const char* word, 
                                 bool exc=false);
    private:
       /// lock for protecting stem calls 
      indri::thread::Mutex _stemLock;
      /// Dictionary table entry
      typedef struct dictEntry {
        /// is the word an exception to stemming rules?
        bool exception;      
        /// stem to use for this entry.
        const char *root;
      } dictEntry;
      /// Two term hashtable entry for caching across calls
      typedef struct cacheEntry {
        /// flag for first or second entry most recently used.
        char flag; 
        /// first entry variant
        char word1[MAX_WORD_LENGTH];
        /// first entry stem
        char stem1[MAX_WORD_LENGTH];
        /// second entry variant
        char word2[MAX_WORD_LENGTH];
        /// second entry stem
        char stem2[MAX_WORD_LENGTH];
      } cacheEntry;

      // operates on atribute word.
      bool ends(const char *s, int sufflen);
      void setsuff(const char *str, int length);
      dictEntry *getdep(char *word);
      bool lookup(char *word);
      bool cons(int i);
      bool vowelinstem();
      bool vowel(int i);
      bool doublec(int i);
      void plural();
      void past_tense();
      void aspect();
      void ion_endings();
      void er_and_or_endings ();
      void ly_endings ();
      void al_endings() ;
      void ive_endings() ;
      void ize_endings() ;
      void ment_endings() ;
      void ity_endings() ;
      void ble_endings() ;
      void ness_endings() ;
      void ism_endings();
      void ic_endings();
      void ncy_endings();
      void nce_endings();
      // maint.
      void loadTables();
#ifdef WIN32
      struct ltstr {
        bool operator()(const char* s1, const char* s2) const {
          return strcmp(s1, s2) < 0;
        }
      };
      //studio 7 hash_map provides hash_compare, rather than hash
      // needing an < predicate, rather than an == predicate.
      typedef stdext::hash_map<const char *, dictEntry, stdext::hash_compare<const char *, ltstr> > dictTable;
#else
      struct eqstr {
        bool operator()(const char* s1, const char* s2) const {
          return strcmp(s1, s2) == 0;
        }
      };
#if HAVE_GCC_VERSION(4,3)
      typedef std::tr1::unordered_map<const char *, dictEntry, std::tr1::hash<std::string>, eqstr> dictTable;
#else
      typedef hash_map<const char *, dictEntry, hash<const char *>, eqstr> dictTable;
#endif
#endif
      dictTable dictEntries;
      // this needs to be a bounded size cache.
      // kstem.cpp uses size 30013 entries.
      cacheEntry *stemCache;
      // size
      int stemhtsize;
      // state
      // k = wordlength - 1
      int k;
      // j is stemlength - 1
      int j;
      // pointer to the output buffer
      char *word;
      // used by kstem_stemmer to return a safe value.
      char stem[MAX_WORD_LENGTH];
    };
  }
}
#endif /* _KROVETZ_STEMMER_H_*/

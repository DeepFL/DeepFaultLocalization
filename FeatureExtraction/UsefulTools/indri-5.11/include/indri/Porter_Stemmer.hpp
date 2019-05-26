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


/* This is the Porter stemming algorithm, coded up in ANSI C by the
   author. It may be be regarded as cononical, in that it follows the
   algorithm presented in

   Porter, 1980, An algorithm for suffix stripping, Program, Vol. 14,
   no. 3, pp 130-137,

   only differing from it at the points maked --DEPARTURE-- below.

   See also http://www.omsee.com/~martin/stem.html

   The algorithm as described in the paper could be exactly replicated
   by adjusting the points of DEPARTURE, but this is barely necessary,
   because (a) the points of DEPARTURE are definitely improvements, and
   (b) no encoding of the Porter stemmer I have seen is anything like
   as exact as this version, even with the points of DEPARTURE!

   You can compile it on Unix with 'gcc -O3 -o stem stem.c' after which
   'stem' takes a list of inputs and sends the stemmed equivalent to
   stdout.

   The algorithm as encoded here is particularly fast.

   Release 1

   07/31/2005 -- Rewrite to be a standalone object.
*/


/* The main part of the stemming algorithm starts here. b is a buffer
   holding a word to be stemmed. The letters are in b[k0], b[k0+1] ...
   ending at b[k]. In fact k0 = 0 in this demo program. k is readjusted
   downwards as the stemming progresses. Zero termination is not in fact
   used in the algorithm.

   Note that only lower case sequences are stemmed. Forcing to lower case
   should be done before stem(...) is called.
*/
#ifndef _PORTER_STEMMER_H_
#define _PORTER_STEMMER_H_
#include "indri/Mutex.hpp"
#include "indri/ScopedLock.hpp"

namespace indri
{
  namespace parse 
  {
    class Porter_Stemmer 
    {
    private:
      indri::thread::Mutex _stemLock;
      char * b;       /* buffer for word to be stemmed */
      int k,k0,j;     /* j is a general offset into the string */

      /* cons(i) is TRUE <=> b[i] is a consonant. */
      bool cons(int i);

      /* m() measures the number of consonant sequences between k0 and j. if c is
         a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
         presence,

         <c><v>       gives 0
         <c>vc<v>     gives 1
         <c>vcvc<v>   gives 2
         <c>vcvcvc<v> gives 3
         ....
      */

      int m();

      /* vowelinstem() is TRUE <=> k0,...j contains a vowel */

      bool vowelinstem();

      /* doublec(j) is TRUE <=> j,(j-1) contain a double consonant. */

      bool doublec(int j);

      /* cvc(i) is TRUE <=> i-2,i-1,i has the form consonant - vowel - consonant
         and also if the second c is not w,x or y. this is used when trying to
         restore an e at the end of a short word. e.g.

         cav(e), lov(e), hop(e), crim(e), but
         snow, box, tray.

      */

      bool cvc(int i);

      /* ends(s) is TRUE <=> k0,...k ends with the string s. */

      bool ends(const char * s);

      /* setto(s) sets (j+1),...k to the characters in the string s, readjusting
         k. */

      void setto(const char * s);

      /* r(s) is used further down. */

      void r(const char * s);

      /* step1ab() gets rid of plurals and -ed or -ing. e.g.

      caresses  ->  caress
      ponies    ->  poni
      ties      ->  ti
      caress    ->  caress
      cats      ->  cat

      feed      ->  feed
      agreed    ->  agree
      disabled  ->  disable

      matting   ->  mat
      mating    ->  mate
      meeting   ->  meet
      milling   ->  mill
      messing   ->  mess

      meetings  ->  meet

      */

      void step1ab();

      /* step1c() turns terminal y to i when there is another vowel in the stem. */

      void step1c();


      /* step2() maps double suffices to single ones. so -ization ( = -ize plus
         -ation) maps to -ize etc. note that the string before the suffix must give
         m() > 0. */

      void step2();

      /* step3() deals with -ic-, -full, -ness etc. similar strategy to step2. */

      void step3();

      /* step4() takes off -ant, -ence etc., in context <c>vcvc<v>. */

      void step4();

      /* step5() removes a final -e if m() > 1, and changes -ll to -l if
         m() > 1. */

      void step5();
    public:
      /* In stem(p,i,j), p is a char pointer, and the string to be stemmed is from
         p[i] to p[j] inclusive. Typically i is zero and j is the offset to the last
         character of a string, (p[j+1] == '\0'). The stemmer adjusts the
         characters p[i] ... p[j] and returns the new end-point of the string, k.
         Stemming never increases word length, so i <= k <= j. To turn the stemmer
         into a module, declare 'stem' as extern, and delete the remainder of this
         file.
      */
      /*!
        \brief stem a term using the Porter algorithm. 
        Performs case normalization on its input argument. 
        @param p the term to stem
        @param i the starting index to stem from (typically 0).
        @param j the ending index (inclusive) to stem to 
        (typically the last character of the string).
        @return the new end point of the string.
       */

      int porter_stem(char * p, int i, int j);
    };
  }
}
#endif /* _PORTER_STEMMER_H_*/

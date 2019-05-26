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

#include <cstring>  /* for memmove */
#include "indri/Porter_Stemmer.hpp"

namespace indri
{
  namespace parse
  {
    bool Porter_Stemmer::cons(int i) {  
      switch (b[i]) {  
      case 'a': case 'e': case 'i': case 'o': case 'u': return false;
      case 'y': return (i==k0) ? true : !cons(i-1);
      default: return true;
      }
    }
    int Porter_Stemmer::m() {
      int n = 0;
      int i = k0;
      while(true) {
        if (i > j) return n;
        if (! cons(i)) break; i++;
      }
      i++;
      while(true) {
        while(true) {
          if (i > j) return n;
          if (cons(i)) break;
          i++;
        }
        i++;
        n++;
        while(true) {
          if (i > j) return n;
          if (! cons(i)) break;
          i++;
        }
        i++;
      }
    }

    bool Porter_Stemmer::vowelinstem() {
      int i; 
      for (i = k0; i <= j; i++) if (! cons(i)) return true;
      return false;
    }

    bool Porter_Stemmer::doublec(int j) {
      if (j < k0+1) return false;
      if (b[j] != b[j-1]) return false;
      return cons(j);
    }

    bool Porter_Stemmer::cvc(int i) {
      if (i < k0+2 || !cons(i) || cons(i-1) || !cons(i-2)) return false;
      int ch = b[i];
      if (ch == 'w' || ch == 'x' || ch == 'y') return false;
      return true;
    }

    bool Porter_Stemmer::ends(const char * s) {
      int length = s[0];
      if (s[length] != b[k]) return false; /* tiny speed-up */
      if (length > k-k0+1) return false;
      if (memcmp(b+k-length+1,s+1,length) != 0) return false;
      j = k-length;
      return true;
    }

    void Porter_Stemmer::setto(const char * s) {
      int length = s[0];
      memmove(b+j+1,s+1,length);
      k = j+length;
    }

    void Porter_Stemmer::r(const char * s) { if (m() > 0) setto(s); }

    void Porter_Stemmer::step1ab() {
      if (b[k] == 's') {
        if (ends("\04" "sses")) k -= 2; 
        else if (ends("\03" "ies")) setto("\01" "i"); 
        else if (b[k-1] != 's') k--;
      }
      if (ends("\03" "eed")) { 
        if (m() > 0) k--; 
      } else if ((ends("\02" "ed") || ends("\03" "ing")) && vowelinstem()) {
        k = j;
        if (ends("\02" "at")) setto("\03" "ate"); 
        else if (ends("\02" "bl")) setto("\03" "ble");
        else if (ends("\02" "iz")) setto("\03" "ize"); 
        else if (doublec(k)) {
          k--;
          int ch = b[k];
          if (ch == 'l' || ch == 's' || ch == 'z') k++;
        } else if (m() == 1 && cvc(k)) setto("\01" "e");
      }
    }

    void Porter_Stemmer::step1c() { 
      if (ends("\01" "y") && vowelinstem()) b[k] = 'i'; 
    }

    void Porter_Stemmer::step2() { 
      switch (b[k-1]) {
      case 'a': if (ends("\07" "ational")) { r("\03" "ate"); break; }
        if (ends("\06" "tional")) { r("\04" "tion"); break; }
        break;
      case 'c': if (ends("\04" "enci")) { r("\04" "ence"); break; }
        if (ends("\04" "anci")) { r("\04" "ance"); break; }
        break;
      case 'e': if (ends("\04" "izer")) { r("\03" "ize"); break; }
        break;
      case 'l': if (ends("\03" "bli")) { r("\03" "ble"); break; } /*-DEPARTURE-*/

        /* To match the published algorithm, replace this line with
           case 'l': if (ends("\04" "abli")) { r("\04" "able"); break; } */

        if (ends("\04" "alli")) { r("\02" "al"); break; }
        if (ends("\05" "entli")) { r("\03" "ent"); break; }
        if (ends("\03" "eli")) { r("\01" "e"); break; }
        if (ends("\05" "ousli")) { r("\03" "ous"); break; }
        break;
      case 'o': if (ends("\07" "ization")) { r("\03" "ize"); break; }
        if (ends("\05" "ation")) { r("\03" "ate"); break; }
        if (ends("\04" "ator")) { r("\03" "ate"); break; }
        break;
      case 's': if (ends("\05" "alism")) { r("\02" "al"); break; }
        if (ends("\07" "iveness")) { r("\03" "ive"); break; }
        if (ends("\07" "fulness")) { r("\03" "ful"); break; }
        if (ends("\07" "ousness")) { r("\03" "ous"); break; }
        break;
      case 't': if (ends("\05" "aliti")) { r("\02" "al"); break; }
        if (ends("\05" "iviti")) { r("\03" "ive"); break; }
        if (ends("\06" "biliti")) { r("\03" "ble"); break; }
        break;
      case 'g': if (ends("\04" "logi")) { r("\03" "log"); break; } /*-DEPARTURE-*/

      } 
    }

    void Porter_Stemmer::step3() { 
      switch (b[k]) {
      case 'e': if (ends("\05" "icate")) { r("\02" "ic"); break; }
        if (ends("\05" "ative")) { r("\00" ""); break; }
        if (ends("\05" "alize")) { r("\02" "al"); break; }
        break;
      case 'i': if (ends("\05" "iciti")) { r("\02" "ic"); break; }
        break;
      case 'l': if (ends("\04" "ical")) { r("\02" "ic"); break; }
        if (ends("\03" "ful")) { r("\00" ""); break; }
        break;
      case 's': if (ends("\04" "ness")) { r("\00" ""); break; }
        break;
      } 
    }

    void Porter_Stemmer::step4() {
      switch (b[k-1]) {
      case 'a': if (ends("\02" "al")) break; return;
      case 'c': if (ends("\04" "ance")) break;
        if (ends("\04" "ence")) break; return;
      case 'e': if (ends("\02" "er")) break; return;
      case 'i': if (ends("\02" "ic")) break; return;
      case 'l': if (ends("\04" "able")) break;
        if (ends("\04" "ible")) break; return;
      case 'n': if (ends("\03" "ant")) break;
        if (ends("\05" "ement")) break;
        if (ends("\04" "ment")) break;
        if (ends("\03" "ent")) break; return;
      case 'o': if (ends("\03" "ion") && (b[j] == 's' || b[j] == 't')) break;
        if (ends("\02" "ou")) break; return;
        /* takes care of -ous */
      case 's': if (ends("\03" "ism")) break; return;
      case 't': if (ends("\03" "ate")) break;
        if (ends("\03" "iti")) break; return;
      case 'u': if (ends("\03" "ous")) break; return;
      case 'v': if (ends("\03" "ive")) break; return;
      case 'z': if (ends("\03" "ize")) break; return;
      default: return;
      }
      if (m() > 1) k = j;
    }

    void Porter_Stemmer::step5() {
      j = k;
      if (b[k] == 'e') {
        int a = m();
        if (a > 1 || (a == 1 && !cvc(k-1))) k--;
      }
      if (b[k] == 'l' && doublec(k) && m() > 1) k--;
    }

    int Porter_Stemmer::porter_stem(char * p, int i, int j) {
      indri::thread::ScopedLock lock( _stemLock );
      b = p; k = j; k0 = i; /* copy the parameters into statics */
      if (k <= k0+1) return k; /*-DEPARTURE-*/

      /* With this line, strings of length 1 or 2 don't go through the
         stemming process, although no mention is made of this in the
         published algorithm. Remove the line to match the published
         algorithm. */
      step1ab(); step1c(); step2(); step3(); step4(); step5();
      return k;
    }
  }
}
#ifdef UNIT_TEST
#include <cstdio>
int main(int argc, char *argv[])
{
  char word[80];
  int ret;
  indri::parse::Porter_Stemmer * stemmer = new indri::parse::Porter_Stemmer();
   
  do  {
    gets(word);
    if (*word == '\0') break;
    ret = stemmer->porter_stem(word, 0, strlen(word) -1);
    word[ret+1] = 0;
    printf("%s -> %d\n", word, ret);
  } while(!feof(stdin));
  delete(stemmer);
  return(0);
}
#endif

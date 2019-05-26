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


#include <sstream>
#include "indri/QueryStopper.hpp"

#define NUM_STOP_WORDS 57

std::string indri::query::QueryStopper::transform(std::string query) {
  //# Based on "Intro. to IR", p.25
  //http://www-csli.stanford.edu/~hinrich/information-retrieval-book.html
  const std::string sw[NUM_STOP_WORDS] = {"a", "about", "am", "an", "and", 
                                          "are", "as", "at", "be", 
                                          "been", "being", "by", "did", "do", 
                                          "does", "doing", "done", "for", 
                                          "from", "had", "have", "has", "he", 
                                          "in", "if", "is", "it", "its", "of",
                                          "on", "or", "that", "th", "the", 
                                          "to", "was", "were", "will", "with"};
  std::string token;
  std::istringstream qstream(query);
  std::string result = "";  
  // tokenize
  // yes, there are better ways to do this...
  while ( getline(qstream, token, ' ') ) {
    bool stopped = false;
    for (int i = 0; i < NUM_STOP_WORDS; i++) {
      if (token == sw[i]){
        stopped = true;
        break;
      }
    }
    if (! stopped) {
      result += token;
      result += " ";
    }
  }
  return result;
}


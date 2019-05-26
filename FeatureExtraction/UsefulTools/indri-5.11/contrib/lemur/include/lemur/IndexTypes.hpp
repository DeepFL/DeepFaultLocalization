/*==========================================================================
 * Copyright (c) 2004 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

/* type definitions for index objects */
#ifndef _INDEXTYPES_H
#define _INDEXTYPES_H

#include <string>
namespace lemur
{
  namespace api
  {
    /// internal file id for mapping.
    typedef int   FILEID_T;
    // All four of TERMID_T, DOCID_T, LOC_T, and COUNT_T need to be
    // the same size for the Inv(FP) and Keyfile indexes to work
    // without rewrite. 10/05/2004 -- dmf
    /// internal term id encoding.
    typedef int TERMID_T;
    /// term position
    typedef TERMID_T LOC_T;
    /// internal document id encoding.
    typedef TERMID_T DOCID_T;
    /// integral frequencies
    typedef TERMID_T COUNT_T;
    /// floating point frequencies and/or scores.
    typedef float SCORE_T;
    /// external term encoding
    typedef std::string TERM_T;
    /// external document id encoding
    typedef std::string EXDOCID_T;
    /// pointer into collection of term positions
    typedef int POS_T;  // Used by DocLists and TermLists
  }
}

#endif

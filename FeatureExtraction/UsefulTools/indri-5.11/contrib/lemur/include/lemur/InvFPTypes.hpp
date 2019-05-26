/*==========================================================================
 * Copyright (c) 2001 Carnegie Mellon University.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


/* type definitions for objects we will use */
#ifndef _INVFPTYPES_H
#define _INVFPTYPES_H

#include "common_headers.hpp"
#include "IndexTypes.hpp"
#include <cstring>
namespace lemur 
{
  namespace index
  {
    
#define IND_VERSION "5.1"

    // suffixes for filenames
#define INVINDEX  ".invf"
#define INVFPINDEX ".invfp"
#define INVLOOKUP  ".invlookup"
#define DTINDEX  ".dt"
#define DTLOOKUP  ".dtlookup"
#define TERMIDMAP  ".tid"
#define TERMIDSTRMAP ".tidstr"
#define DOCIDMAP  ".did"
#define DOCIDSTRMAP ".didstr"
#define MAINTOC  ".inv"
#define INVFPTOC ".ifp"
#define DOCMGRMAP ".dm"

    // what to call out of vocabulary ids
#define INVALID_STR "[OOV]"

    // name for parameters
#define VERSION_PAR "VERSION"
#define NUMDOCS_PAR "NUM_DOCS"
#define NUMTERMS_PAR "NUM_TERMS"
#define NUMUTERMS_PAR "NUM_UNIQUE_TERMS"
#define AVEDOCLEN_PAR "AVE_DOCLEN"
#define INVINDEX_PAR  "INV_INDEX"
#define INVLOOKUP_PAR  "INV_LOOKUP"
#define DTINDEX_PAR  "DT_INDEX"
#define DTLOOKUP_PAR  "DT_LOOKUP"
#define TERMIDMAP_PAR  "TERMIDS"
#define TERMIDSTRMAP_PAR "TERMIDSTRS"
#define DOCIDMAP_PAR  "DOCIDS"
#define DOCIDSTRMAP_PAR "DOCIDSTRS"
#define NUMDT_PAR  "NUM_DTFILES"
#define NUMINV_PAR  "NUM_INVFILES"
#define DOCMGR_PAR  "DOCMGR_IDS"

    struct LocatedTerm { // pair of term and its location
      lemur::api::TERMID_T term;
      lemur::api::LOC_T loc;
    };

    struct LLTerm { // pair of term and list of locations
      lemur::api::TERMID_T term;
      vector<lemur::api::LOC_T> loc;
    };

    struct dt_entry {   // an entry in the lookup table for docterm lists index
      lemur::api::FILEID_T fileid;  // which file the word is in
      long offset;        // what the offset into the file is
      int length;         // the length of the inverted list
      int docmgr;         // the docmgr id of manager for this doc
    };

    struct inv_entry {   // an entry in the lookup table for docterm lists index
      lemur::api::FILEID_T fileid;  // which file the word is in
      long offset;        // what the offset into the file is
      lemur::api::COUNT_T ctf;            // collection term freq
      lemur::api::COUNT_T df;             // doc freq
    };

    struct ltstr
    {
      bool operator()(char* s1, char* s2) const{
        return strcmp(s1, s2) < 0;
      }
    };
  }
}

#endif

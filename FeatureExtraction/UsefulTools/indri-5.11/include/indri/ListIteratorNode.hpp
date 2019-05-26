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
// ListIteratorNode
//
// 26 January 2004 -- tds
//

#ifndef INDRI_LISTITERATORNODE_HPP
#define INDRI_LISTITERATORNODE_HPP

#include "indri/InferenceNetworkNode.hpp"
#include "indri/Extent.hpp"
#include <indri/greedy_vector>
namespace indri
{
  namespace infnet 
  {
    class ListIteratorNode : public InferenceNetworkNode {
    protected:
      indri::utility::greedy_vector<indri::index::Extent> _matches;

      // speedup for child siblings - 
      // stores last successful position of the matches(extent)
      // search; linear search assuming ascending order
      int _lastpos; 
      indri::index::Extent _lastExtent;

    public:
      /// initializes the last position pointer for matching child siblings
      void initpointer() {
        _lastpos=0;
      }

      /// sorts the extents by the parent IDs (if available)
      void sortparent(indri::utility::greedy_vector<indri::index::Extent>& extents) {
        int sorted=0;
        int lastbegin = 0; int lastend = extents.size();
        while(lastbegin<lastend){
          int i=lastbegin;
          int end=lastend-1;
          lastbegin=lastend;
          lastend=i;
          for (;i<end;i++){
            if (extents[i].parent > extents[i+1].parent){
              indri::index::Extent x(extents[i]);
              extents[i]=extents[i+1];extents[i+1]=x;
              if(lastbegin>i)lastbegin=i;
              if(lastend<i+1)lastend=i+1;
            }
          }
        }
      }

      /// sorts the extents by the beginning extent position (if available)
      void sortbegin(indri::utility::greedy_vector<indri::index::Extent>& extents){
        int sorted=0;
        int lastbegin = 0; int lastend = extents.size();
        while(lastbegin<lastend){
          int i=lastbegin;
          int end=lastend-1;
          lastbegin=lastend;
          lastend=i;
          for (;i<end;i++){
            if (extents[i].begin > extents[i+1].begin){
              indri::index::Extent x(extents[i]);
              extents[i]=extents[i+1];extents[i+1]=x;
              if(lastbegin>i)lastbegin=i;
              if(lastend<i+1)lastend=i+1;
            }
          }
        }
      }

      /// sets up as much as we can with just the document ID
      virtual void prepare( lemur::api::DOCID_T documentID ) = 0;

      /// returns a list of intervals describing positions of children
      virtual const indri::utility::greedy_vector<indri::index::Extent>& extents() = 0;

      /// annotate any results from this node from position begin to position end
      virtual void annotate( class Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) = 0;

      // Moved work of identifying matches of an extent from ExtentRestriction to here.
      // This allows the computation for the list of matches for an extent to be 
      // overridden.  ExtentParent, ExtentChild, and ExtentDescendant are among the
      // classes that need to do this, as the parent/child/descedant relationships
      // are not based on extent containment - these relationships may be among
      // arbitrary fields in a document.
      virtual const indri::utility::greedy_vector<indri::index::Extent>& matches( indri::index::Extent &extent ) {
        int begin = extent.begin;
        int end = extent.end;
        _matches.clear();
        const indri::utility::greedy_vector<indri::index::Extent>& exts = extents();

        // if there's no extents or we have no length - just return
        if (begin == end || exts.size()==0) return _matches;

        // if we are dealing with child extents, we need to reverse the
        // list pointer to the last good position
        while((_lastpos > 0) && (exts[_lastpos-1].begin >= begin)){
          _lastpos--;
        }

        // now, we make sure we're in the correct position
        // after this loop, _lastpos->begin >= begin
        while((_lastpos < exts.size()) && (exts[_lastpos].begin < begin)){
          _lastpos++;
        }

        // for default DocListIteratorNode, any extent: begin+1 == end.
        while((_lastpos < exts.size()) && (exts[_lastpos].begin < end)) { 
          if(exts[_lastpos].end <= end) {
            indri::index::Extent ext(exts[_lastpos]);
            _matches.push_back(ext);
          } // end if(_exts[_lastpos].end<=end)
          _lastpos++;
        } // end while(_lastpos<_exts.size()&&_exts[_lastpos].begin<end)

/***
 *** old method of matching child extents - deprecated 
 *
 *      for( size_t i = 0 ; i < exts.size(); i++ ) {
 *        if ( begin <= exts[i].begin && end >= exts[i].end ) {
 *          _matches.push_back( exts[i] );
 *        } else if ( exts[i].begin > end ) {
 *          break;
 *        }
 *      }
 **/
        return _matches;
      }
    };
  }
}

#endif // INDRI_LISTNODE_HPP


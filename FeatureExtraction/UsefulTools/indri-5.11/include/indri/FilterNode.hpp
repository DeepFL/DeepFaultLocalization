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
// FilterNode
//
// 21 July 2004 -- tds
//
// Restricts evaluation to only a subset of the documents
// in the collection.
//

#ifndef INDRI_FILTERNODE_HPP
#define INDRI_FILTERNODE_HPP

namespace indri
{
  namespace infnet
  {
    
    class FilterNode : public BeliefNode {
    private:
      BeliefNode* _belief;
      std::vector<lemur::api::DOCID_T> _documents;
      std::string _name;
      int  _index;
      indri::utility::greedy_vector<bool> _matches;
      
    public:
      FilterNode( const std::string& name, BeliefNode* child, const std::vector<lemur::api::DOCID_T>& documents )
        :
        _documents(documents)
      {
        _name = name;
        _belief = child;
        _index = -1;
        std::sort( _documents.begin(), _documents.end() );
      }

      virtual void setSiblingsFlag(int f) {
        bSiblings=f; // need to set the flag for the current node itself.
        if (_belief) {  _belief->setSiblingsFlag(f); }
      }

      lemur::api::DOCID_T nextCandidateDocument() {
          _index++;
    
        if( _index >= _documents.size() )
          return MAX_INT32;

        return _documents[_index];
      }

      void annotate( Annotator& annotator, lemur::api::DOCID_T documentID, indri::index::Extent &extent ) {
        return _belief->annotate( annotator, documentID, extent );
      }

      const indri::utility::greedy_vector<indri::api::ScoredExtentResult>& score( lemur::api::DOCID_T documentID, indri::index::Extent &extent, int documentLength ) {
        return _belief->score( documentID, extent, documentLength );
      }

      double maximumScore() {
        return _belief->maximumScore();
      }

      double maximumBackgroundScore() {
        return _belief->maximumBackgroundScore();
      }

      bool hasMatch( lemur::api::DOCID_T documentID ) {
        return _documents[_index] == documentID;
      }

      const indri::utility::greedy_vector<bool>& hasMatch( lemur::api::DOCID_T documentID, const indri::utility::greedy_vector<indri::index::Extent>& extents ) {
        _matches.clear();
        _matches.resize( extents.size(),  _documents[_index] == documentID);
        return _matches;
    }

      void indexChanged( indri::index::Index& index ) {
        // reset the _index to make sure we see newly added documents
        _index = -1;
      }

      const std::string& getName() const {
        return _name;
      }
    };
  }
}

#endif // INDRI_FILTERNODE_HPP


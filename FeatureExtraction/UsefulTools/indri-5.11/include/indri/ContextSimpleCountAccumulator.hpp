
//
// ContextSimpleCountAccumulator
//
// 14 December 2004 -- tds
//
// Unlike the ContextCountAccumulator, which counts occurrences of
// terms in a very general way, this node uses knowledge about the 
// query tree to extract counts directly from the index.
//
// This node is placed into the query tree by the
// ContextSimpleCountCollectorCopier.
//

#ifndef INDRI_CONTEXTSIMPLECOUNTACCUMULATOR_HPP
#define INDRI_CONTEXTSIMPLECOUNTACCUMULATOR_HPP

#include "indri/Index.hpp"
#include "indri/EvaluatorNode.hpp"
#include <string>
#include <vector>

namespace indri
{
  namespace infnet
  {
    
    class ContextSimpleCountAccumulator : public EvaluatorNode {
    private:
      std::string _name;

      std::vector<std::string> _terms;
      std::string _field;
      std::string _context;

      UINT64 _occurrences;
      UINT64 _size;
      lemur::api::DOCID_T _maximumDocument;
      int _documentOccurrences;
      int _documentCount;

      EvaluatorNode::MResults _results;

      void _computeCounts( indri::index::Index& index );

    public:
      ContextSimpleCountAccumulator( const std::string& nodeName,
                                     const std::vector<std::string>& terms,
                                     const std::string& field,
                                     const std::string& context );

      const std::string& getName() const;
      const EvaluatorNode::MResults& getResults();

      int getDocumentOccurrences() const ;
      int getDocumentCount() const ;

      void indexChanged( indri::index::Index& index );
      void evaluate( lemur::api::DOCID_T documentID, int documentLength );
      lemur::api::DOCID_T nextCandidateDocument();
    };
  }
}


#endif // INDRI_CONTEXTSIMPLECOUNTACCUMULATOR_HPP

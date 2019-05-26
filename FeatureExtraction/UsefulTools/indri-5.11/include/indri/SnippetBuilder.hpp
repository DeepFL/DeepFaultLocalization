

//
// SnippetBuilder
//
// This code is based largely on the code I wrote for the PHP and Java
// interfaces.
//
// 17 July 2006 -- tds
//            

#ifndef INDRI_SNIPPETBUILDER_HPP
#define INDRI_SNIPPETBUILDER_HPP

#include <vector>
#include <string>
#include "indri/QueryAnnotation.hpp"
#include "indri/ParsedDocument.hpp"

namespace indri {
  namespace api {
    class SnippetBuilder {
    private:
      bool _HTMLOutput;
      
    public:
      struct Region {
        int begin;
        int end;
        
        std::vector<indri::index::Extent> extents;
      };

    private:  
      void _getRawNodes( std::vector<std::string>& nodeNames, const indri::api::QueryAnnotationNode* node );
     
      std::vector< std::pair<indri::index::Extent, int> > _documentMatches( int document, 
                                                                            const std::map< std::string, std::vector<indri::api::ScoredExtentResult> >& annotations,
                                                                            const std::vector<std::string>& nodeNames );
      
      std::vector<Region> _buildRegions(
                                        std::vector< std::pair<indri::index::Extent, int> >& extents,
                                        int positionCount, int matchWidth, int windowWidth );

      Region _bestRegion( const std::vector< std::pair<indri::index::Extent, int> >& extents,
                          const std::vector< indri::api::SnippetBuilder::Region >& skipRegions,
                          int positionCount, int windowWidth );

      char* _sanitizeText( const char* text, int begin, int length );

      void _addEllipsis( std::string& snippet );
      void _addHighlightedRegion( std::string& snippet, char* region );
      void _addUnhighlightedRegion( std::string& snippet, char* region );
      void _completeSnippet( std::string& snippet );
      
    public:  
      SnippetBuilder( bool html ) : _HTMLOutput(html) {}
      std::string build( int documentID, const indri::api::ParsedDocument* document, indri::api::QueryAnnotation* annotation );
    };
  }
}

#endif // INDRI_SNIPPETBUILDER_HPP

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
// IndexEnvironment
//
// 19 July 2004 -- tds
//

#ifndef INDRI_INDEXENVIRONMENT_HPP
#define INDRI_INDEXENVIRONMENT_HPP

#include <string>
#include "indri/Parameters.hpp"
#include "indri/HTMLParser.hpp"
#include "indri/ConflationPattern.hpp"
#include "indri/Repository.hpp"
#include "indri/IndriParser.hpp"
#include "indri/IndriTokenizer.hpp"
#include "indri/DocumentIterator.hpp"
#include "indri/AnchorTextAnnotator.hpp"
#include "indri/OffsetAnnotationAnnotator.hpp"
#include "indri/OffsetMetadataAnnotator.hpp"
#include "indri/Transformation.hpp"
#include "indri/DocumentIteratorFactory.hpp"
#include "indri/ParserFactory.hpp"
#include "indri/FileClassEnvironmentFactory.hpp"
#include <map>
#include <set>
namespace indri 
{
  ///indri classes that provide services to applications programmers
  namespace api 
  {
    
    struct IndexStatus {
      enum action_code {
        FileOpen,
        FileSkip,
        FileError,
        FileClose,
        DocumentCount
      };

      virtual void operator () ( int code, const std::string& documentPath, const std::string& error, int documentsIndexed, int documentsSeen ) {
        status( code, documentPath, error, documentsIndexed, documentsSeen );
      }

      virtual void status( int code, const std::string& documentPath, const std::string& error, int documentsIndexed, int documentsSeen ) {};
    };

    /*! \brief Principal class for interacting with Indri indexes during index 
      construction. 
      Provides the API for opening or creating an index and its
      associated repository, setting indexing and text parsing parameters, and
      adding documents to the repository.
    */
    class IndexEnvironment {
    private:
      IndexStatus* _callback;
      Parameters* _options;

      std::string _repositoryPath;
      indri::collection::Repository _repository;
      int _documents;
      std::string _error;

      std::string _offsetAnnotationsRoot;
      std::string _offsetMetadataRoot;
      std::string _anchorTextRoot;
      std::string _documentRoot;

      Parameters _parameters;
      indri::parse::FileClassEnvironmentFactory _fileClassFactory;

      indri::parse::AnchorTextAnnotator _annotator;
      indri::parse::OffsetAnnotationAnnotator _oa_annotator;
      indri::parse::OffsetMetadataAnnotator _om_annotator;

      std::map<std::string, indri::parse::FileClassEnvironment*> _environments;

      int _documentsIndexed;
      int _documentsSeen;

      void _getParsingContext( indri::parse::Parser** parser,
                               indri::parse::Tokenizer** tokenizer,
                               indri::parse::DocumentIterator** iterator,
                               indri::parse::Conflater** conflater,
                               const std::string& extension );

      std::vector<indri::parse::Transformation*> _createAnnotators( const std::string& fileName, 
                                                                    const std::string& fileClass, 
                                                                    indri::parse::Conflater** conflater);

      ParsedDocument* _applyAnnotators( std::vector<indri::parse::Transformation*>& annotators, 
                                        ParsedDocument* parsed ); 

      std::set<std::string> _blackedDocs;

    public:
      friend class QueryEnvironment;

      IndexEnvironment();
      ~IndexEnvironment();

      /// Spam documents filtering
      /// Each line of blackListFile is a docno
      /// Returns the number of docnos blacked
      int setBlackList( const std::string& blackListFile);

      /// Set offset annotations root path.
      /// @param offsetAnnotationsRoot path to offset annotations root.
      void setOffsetAnnotationsPath( const std::string& offsetAnnotationsRoot );

      /// Set offset metadata root path.
      /// @param offsetMetadataRoot path to offset metadata root.
      void setOffsetMetadataPath( const std::string& offsetMetadataRoot );

      /// Set anchor text root path.
      /// @param anchorTextRoot path to anchor text root.
      void setAnchorTextPath( const std::string& anchorTextRoot );

      /// Set the document root path
      /// @param documentRoot path to document root.
      void setDocumentRoot( const std::string& documentRoot );

      /// Add parsing information for a file class. Data for these parameters
      /// is passed into the FileClassEnvironmentFactory
      /// @param name name of this file class, eg trecweb
      /// @param iterator document iterator for this file class
      /// @param parser document parser for this file class
      /// @param tokenizer document tokenizer for this file class
      /// @param startDocTag tag indicating start of a document
      /// @param endDocTag tag indicating the end of a document
      /// @param endMetadataTag tag indicating the end of the metadata fields
      /// @param include default tags whose contents should be included in the index
      /// @param exclude tags whose contents should be excluded from the index
      /// @param index tags that should be forwarded to the index for tag extents
      /// @param metadata tags whose contents should be indexed as metadata
      /// @param conflations tags that should be conflated
      void addFileClass( const std::string& name, 
                         const std::string& iterator,
                         const std::string& parser,
                         const std::string& tokenizer,
                         const std::string& startDocTag,
                         const std::string& endDocTag,
                         const std::string& endMetadataTag,
                         const std::vector<std::string>& include,
                         const std::vector<std::string>& exclude,
                         const std::vector<std::string>& index,
                         const std::vector<std::string>& metadata, 
                         const std::map<indri::parse::ConflationPattern*,std::string>& conflations );

      /// Get a named file class.
      /// @param name The name of the file class to retrieve.
      indri::parse::FileClassEnvironmentFactory::Specification *getFileClassSpec( const std::string& name) {
        return _fileClassFactory.getFileClassSpec(name);
      }

      /// Add a file class.
      /// @param spec The file class to add.
      void addFileClass( const indri::parse::FileClassEnvironmentFactory::Specification &spec ){
        _fileClassFactory.addFileClass(spec);
      }
  
      /// Set names of fields to be indexed.  This call indicates to the index that information about
      /// these fields should be stored in the index so they can be used in queries.  This does not
      /// affect whether or not the text in a particular field is stored in an index.
      /// @see addFileClass
      /// @param fieldNames the list of fields.
      void setIndexedFields( const std::vector<std::string>& fieldNames );

      /// Set the numeric property of a field. 
      /// @param fieldName the field.
      /// @param isNumeric true if the field is a numeric field, false if not.
      /// @param parserName The name of the Transformation to use to compute the numeric value of the field. Repository currently recognizes the name NumericFieldAnnotator.
      void setNumericField( const std::string& fieldName, bool isNumeric,
                            const std::string &parserName = "");

      /// Set the ordinal property of a field. 
      /// @param fieldName the field.
      /// @param isOrdinal true if the field is an ordinal field, false if not.
      void setOrdinalField( const std::string& fieldName, bool isOrdinal);

      /// Set the parental property of a field.
      /// @param fieldName the field.
      /// @param isParental true if the field stores its parent, false if not
      void setParentalField( const std::string& fieldName, bool isParental);


      /// Set names of metadata fields to be indexed for fast retrieval.
      /// The forward fields are indexed in a B-Tree mapping (documentID, metadataValue).
      /// If a field is not forward indexed, the documentMetadata calls will still work, but they
      /// will be slower (the document has to be retrieved, decompressed and parsed to get the metadata back,
      /// instead of just a B-Tree lookup).  The backward indexed fields store a mapping of (metadataValue, documentID).
      /// If a field is not backward indexed, the documentIDsFromMetadata and documentFromMetadata calls will not work.
      /// @param forwardFieldNames the list of fields to forward index.
      /// @param backwardFieldNames the list of fields to backward index.
      void setMetadataIndexedFields( const std::vector<std::string>& forwardFieldNames, const std::vector<std::string>& backwardFieldNames );

      /// set the list of stopwords
      /// @param stopwords the list of stopwords
      void setStopwords( const std::vector<std::string>& stopwords );

      /// set the stemmer to use
      /// @param stemmer the stemmer to use. One of krovetz, porter
      void setStemmer( const std::string& stemmer );

      /// set the amount of memory to use for internal structures
      /// @param memory the number of bytes to use.
      void setMemory( UINT64 memory );

      /// set normalization of case and some punctuation; default is true (normalize during indexing and at query time)
      /// @param flag True, if text should be normalized, false otherwise.
      void setNormalization( bool flag );

      /// set injection of URL text into document; default is true
      /// @param flag True, if URL should be injected, false otherwise.
      void setInjectURL( bool flag );

      /// set storing of ParsedDocuments; default is true
      /// @param flag true, if ParsedDocuments should be stored, false otherwise.
      void setStoreDocs( bool flag );

      /// provides the indexer with the hint strategy to use for speed optimizations for indexing offset annotations
      /// @param hintType the int type (of OffsetAnnotationIndexHint enum type)
      void setOffsetAnnotationIndexHint(indri::parse::OffsetAnnotationIndexHint hintType);

      /// create a new index and repository
      /// @param repositoryPath the path to the repository
      /// @param callback IndexStatus object to be notified of indexing progress.
      void create( const std::string& repositoryPath, IndexStatus* callback = 0 );

      /// open an existing index and repository
      /// @param repositoryPath the path to the repository
      /// @param callback IndexStatus object to be notified of indexing progress.
      void open( const std::string& repositoryPath, IndexStatus* callback = 0 );

      /// close the index and repository
      void close();
  
      /// Add the text in a file to the index and repository.  The fileClass of this file
      /// will be chosen based on the file extension.  If the file has no extension, it will
      /// be skipped.  Information about indexing progress will be passed to the callback.
      /// @see setCallback()
      /// @param fileName the file to add
      void addFile( const std::string& fileName );

      /// add a file of the specified file class to the index and repository
      /// @param fileName the file to add
      /// @param fileClass the file class to add (eg trecweb).
      void addFile( const std::string& fileName, const std::string& fileClass );

      /// Adds a string to the index and repository.  The documentString is assumed to contain the kind of
      /// text that would be found in a file of type fileClass.
      /// @param documentString the document to add
      /// @param fileClass the file class to add (eg trecweb).
      /// @param metadata the metadata pairs associated with the string.
      lemur::api::DOCID_T addString( const std::string& documentString, 
                     const std::string& fileClass, 
                     const std::vector<indri::parse::MetadataPair>& metadata );

      /// Adds a string to the index and repository.  The documentString is assumed to contain the kind of
      /// text that would be found in a file of type fileClass.
      /// @param documentString the document to add
      /// @param fileClass the file class to add (eg trecweb).
      /// @param metadata the metadata pairs associated with the string.
      /// @param tags offset annotations to be indexed as field
      /// data. The begin and end values of each TagExtent should
      /// specify byte (not character or token) offsets within the
      /// document string. These byte offsets are converted to token
      /// offsets after document string parsing.
      lemur::api::DOCID_T addString( const std::string& documentString, 
                     const std::string& fileClass, 
                     const std::vector<indri::parse::MetadataPair>& metadata, 
                     const std::vector<indri::parse::TagExtent *> &tags );
      
      /// add an already parsed document to the index and repository
      /// @param document the document to add
      lemur::api::DOCID_T addParsedDocument( ParsedDocument* document );

      /// Delete an existing document.
      /// @param documentID The document to delete.
      void deleteDocument( lemur::api::DOCID_T documentID );

      /// Returns the number of documents indexed so far in this session.
      int documentsIndexed();

      /// Returns the number of documents considered for indexing,
      /// which is the sum of the documents indexed and the documents
      /// skipped.
      int documentsSeen();

      /// Permanently deletes information for documents that have been
      /// deleted from the index and reclaims used disk space.
      void compact();

      /// Merges the contents of the indexes referenced in the
      /// inputIndexes list and creates a new index called outputIndex.
      /// The final index is compacted (contains no information about deleted documents).
      ///
      /// @param outputIndex The pathname to the index to create (should not exist yet).
      /// @param inputIndexes The pathnames to indexes to merge.  These indexes should not currently be open.
      static void merge( const std::string& outputIndex, const std::vector<std::string>& inputIndexes );
    };
  }
}

#endif // INDRI_INDEXENVIRONMENT_HPP


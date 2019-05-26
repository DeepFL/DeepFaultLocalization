/*==========================================================================
 * Copyright (c) 2003-2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

//
// FileClassEnvironmentFactory
//
// 23 August 2004 -- tds
//

#ifndef INDRI_FILECLASSENVIRONMENTFACTORY_HPP
#define INDRI_FILECLASSENVIRONMENTFACTORY_HPP

#include "indri/FileClassEnvironment.hpp"
#include <string>
/*#include "indri/HashTable.hpp"*/
namespace indri
{
  namespace parse
  {
    
    class FileClassEnvironmentFactory {

    public:
      /// \brief Parsing information for a file class. 
      /// Used to create a FileClassEnvironment.
      struct Specification {
        ///  name of this file class, eg trecweb
        std::string name;
        /// document parser for this file class
        std::string parser;
        /// document tokenizer for this file class
        std::string tokenizer;
        /// document iterator for this file class
        std::string iterator;
        /// tag indicating start of a document
        std::string startDocTag;
        /// tag indicating the end of a document
        std::string endDocTag;
        /// tag indicating the end of the metadata fields
        std::string endMetadataTag;
        /// \brief tags whose contents should be included in the index. 
        /// If empty, all tags are included.
        std::vector<std::string> include;
        /// tags whose contents should be excluded from the index
        std::vector<std::string> exclude;
        /// tags that should be forwarded to the index for tag extents, ie named fields. 
        std::vector<std::string> index;
        /// tags whose contents should be indexed as metadata
        std::vector<std::string> metadata;
        /// \brief tags that should be conflated. 
        /// The map is the of the form tag => conflated tag, eg h1 => heading.
        std::map<indri::parse::ConflationPattern*,std::string> conflations;
      };

      ~FileClassEnvironmentFactory();
      /*! \brief Make an instance of a named FileClassEnvironment. 
        Caller is responsible for deleting the pointer.
        @param name the name of the FileClassEnvironment
        @return an initialized instance of the named FileClassEnvironment, or
        NULL if the name is not found.
      */
      FileClassEnvironment* get( const std::string& name );
      /*!
        \brief Make an instance of a named Specification, suitable for modifying an existing FileClassEnvironment. 
        Caller is responsible
        for deleting the pointer.
        @param name the name of the Specification
        @return an initialized instance of the named Specification, or
        NULL if the name is not found.
      */
      FileClassEnvironmentFactory::Specification* getFileClassSpec( const std::string& name );

      /// \brief Add parsing information for a file class.
      /// @param spec the specification for this FileClassEnvironment 
      void addFileClass( const FileClassEnvironmentFactory::Specification &spec);

      // deprecate & remove.
      /*!
        \brief Add parsing information for a file class.
        @param name name of this file class, eg trecweb
        @param iterator document iterator for this file class
        @param parser document parser for this file class
        @param tokenizer document tokenizer for this file class
        @param startDocTag tag indicating start of a document
        @param endDocTag tag indicating the end of a document
        @param endMetadataTag tag indicating the end of the metadata fields
        @param include default tags whose contents should be included in the index
        @param exclude tags whose contents should be excluded from the index
        @param index tags that should be forwarded to the index for tag extents
        @param metadata tags whose contents should be indexed as metadata
        @param conflations tags that should be conflated
      */
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
    private:
      std::map<std::string, struct FileClassEnvironmentFactory::Specification*> _userTable;

    };
  }
}

#endif // INDRI_FILECLASSENVIRONMENTFACTORY_HPP


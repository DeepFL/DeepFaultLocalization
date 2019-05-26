/*==========================================================================
 * Copyright (c) 2008 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */

/*! \page ModifyFields Indri Repository Field Modification
<P> This application enables adding or removing indexed fields from an Indri
Repository without reindexing the entire collection. Adding a field requires
the use of offset annotations (see 
<a href="http://www.lemurproject.org/tutorials/begin_annotation-1.php">
Offset Annotations</a>) for specifying the field values.

<h3>Parameters</h3>

<dl>
</dd>
<dt>index</dt>
<dd>The indri Repostitory to modify.</dd>

<dt>fileclass</dt>
<dd>The FileClassEnvironment used to build the index (the corpus.class entry).
</dd>

<dt>addField</dt>
<dd>A list of fields to add. The subelements are:
<dl>
<dt>field</dt>
<dd>a complex element specifying the fields to index as data.
The subelements are:
<dl>
<dt>name</dt><dd>the field name</dd> 
<dt>numeric</dt><dd>the symbol <tt>true</tt> if the field contains
numeric data, otherwise the symbol <tt>false</tt>.</dd>
<dt>parserName</dt><dd>the name of the parser to use to convert a numeric
field to an unsigned integer value. If true, you should use the value
OffsetAnnotationAnnotator.</dd>
</dl> 
</dl>

<dt>annotations</dt>
<dd>The offset annotations file for the collection. Single file required.</dd>

<dt>removeField</dt>
<dd>A list of fields to remove. The subelements are:
<dl>
<dt>name</dt><dd>the field name</dd> 
</dl>
</dd>
</dl>
*/
#include "indri/Repository.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/OffsetAnnotationAnnotator.hpp"
#include "indri/FileClassEnvironmentFactory.hpp"
#include "indri/Conflater.hpp"
#include "indri/Parameters.hpp"
#include "indri/Path.hpp"
#include "indri/TagList.hpp"
#include "indri/IndriTimer.hpp"
#include "lemur/Exception.hpp" 

class FieldModifier {

private:
  indri::parse::TagList tl;
  indri::utility::IndriTimer g_timer;
  indri::api::ParsedDocument *parsed;
  indri::utility::Buffer termBuffer;
  indri::index::Index* _index;
  indri::index::TermListFileIterator * _docIter;
  indri::parse::FileClassEnvironment* fce;
  std::vector<std::string> removeNames;
  indri::api::Parameters repo;
  indri::api::Parameters addFields;
  bool adding;
  
  void _mergeFields() {
    if( repo.exists("field") ) {
      indri::api::Parameters result;
      indri::api::Parameters fields = repo["field"];
      for (size_t i = 0; i < fields.size(); i++) {
        indri::api::Parameters oldField = fields[i];
        std::string fieldName = oldField["name"];
        bool keep = true;
        for (size_t j = 0; j < removeNames.size(); j++) {
          if (removeNames[j] == fieldName) {
            keep = false;
            break;
          }
        }
        if (keep) {
          bool isNumeric = oldField.get("numeric", false);
          bool isOrdinal = oldField.get("ordinal", false);
          bool isParental = oldField.get("parental", false);
          std::string parserName = oldField.get("parserName", 
                                                isNumeric ? "NumericFieldAnnotator" : "");
          indri::api::Parameters field = result.append("field");
          field.set( "name", fieldName );
          field.set( "numeric", isNumeric );
          field.set( "parserName", parserName );
          field.set( "ordinal", isOrdinal );
          field.set( "parental", isParental );
        }
      }
      if (adding) {
        indri::api::Parameters newFields = addFields["field"];
        for (size_t i = 0; i < newFields.size(); i++) {
          indri::api::Parameters newField = newFields[i];
          std::string fieldName = newField["name"];
          bool dupe = false;
          for( size_t j = 0; j<fields.size(); j++ ) {
            std::string parameterFieldName = fields[j]["name"];
            if( parameterFieldName == fieldName ) {
              // it's already in there, skip it...
              dupe = true; 
              break;
            }
          }
          if (dupe) continue;

          bool isNumeric = newField.get("numeric", false);
          bool isOrdinal = newField.get("ordinal", false);
          bool isParental = newField.get("parental", false);
          std::string parserName = newField.get("parserName", 
                                                isNumeric ? "OffsetAnnotationAnnotator" : "");
          indri::api::Parameters field = result.append("field");
          field.set( "name", fieldName );
          field.set( "numeric", isNumeric );
          field.set( "parserName", parserName );
          field.set( "ordinal", isOrdinal );
          field.set( "parental", isParental );
        }
      }
      if (result.exists("field")) {
        repo.set("field");
        repo["field"] = result["field"];
      } else {
        // no fields in new index...
        if (repo.exists("field")) {
          repo.remove("field");
        }
      }
    } else {
      // no fields in original, just insert
      // the new ones. (test they exist...)
      if (adding) {
        repo.set("field");
        repo["field"] = addFields["field"];
      }
    }
  }

  void _mergeData() {
    // fill in tags from the document vector fields
    // unnecessary to filter out any fields to remove, as they will
    // be ignored when indexing.
    indri::utility::greedy_vector<indri::index::FieldExtent>& fields = _docIter->currentEntry()->fields() ;
    tl.clear();
    for (size_t i = 0; i < fields.size(); i++) {
      const indri::index::FieldExtent& field = fields[i];
      std::string fieldName = _index->field(fields[i].id);;
    
      tl.addTag(fieldName.c_str(), fieldName.c_str(), field.begin);
      tl.endTag(fieldName.c_str(), fieldName.c_str(), field.end);
    }
    //stuff it into the parsed doc
    tl.writeTagList(parsed->tags);

    // fill in terms from the document text so that they will stop/stem
    // correctly when added to the new repository.
    // Potentially issues with url injection here...
    // probably best not to do this with trecweb/html docs....
    // TODO: test this
    termBuffer.clear();
    termBuffer.grow( parsed->textLength * 2 );

    for (size_t i = 0; i < parsed->positions.size(); i++ ) {
      int start = parsed->positions[i].begin;
      int end = parsed->positions[i].end;
      int token_len = end - start;
      const char *token = parsed->text + start;
      char* write_loc = termBuffer.write( token_len + 1 );
      strncpy( write_loc, token, token_len );
      write_loc[token_len] = '\0';
      parsed->terms.push_back( write_loc );
    }
  }

public:
  FieldModifier() : fce(NULL), parsed(NULL), _docIter(NULL)
  {
    termBuffer.grow(1024*1024);
  }
  ~FieldModifier() 
  {
    delete(fce);
    delete (_docIter);
  }
  
  void processFields( indri::api::Parameters &param ) {
    g_timer.start();
    std::string index = param.get("index");
    std::cout << "Opening: " << index << std::endl;
    // make sure this path doesn't exist.
    std::string idx2 = index + ".new"; // temp target index.

    // presumes a single input oa file for the entire collection.
    std::string offsetAnnotationsPath = param.get("annotations");
      
    /// these need to be combined with existing.
    // fields to add
    // these need to supply numeric/parental/ordinal/etc...
    if (param.exists("addField"))
      addFields = param["addField"];
      
    // fields to remove
    // these only need to be a list of names.
    if (param.exists("removeField")) {
      indri::api::Parameters slice = param["removeField"];
      for (size_t i = 0; i < slice.size(); i++) {
        if( slice[i].exists("name") ) {
          removeNames.push_back( slice[i]["name"] );
        }
      }
    }
      
    // need to know the file class environment to get the 
    // conflations right.
    std::string className = param.get("fileclass", "");

    indri::collection::Repository sourceRepo;
    indri::collection::Repository targetRepo;
    indri::parse::OffsetAnnotationAnnotator oa_annotator;
    indri::parse::FileClassEnvironmentFactory _fileClassFactory;
            
    // Open source repo
    sourceRepo.openRead(index);
    // Copy its parameters, create target repo, adding or removing
    // fields.
    repo.loadFile( indri::file::Path::combine( index, "manifest" ) );
    int mem = param.get("memory", INT64(100*1024*1024));
      
    repo.set("memory", mem);
    adding = addFields.exists("field");
    _mergeFields();
    // Create the offset annotator.
    fce = _fileClassFactory.get( className );
    indri::parse::Conflater* conflater = 0;
    if( fce ) {
      conflater = fce->conflater;
    }
    if (adding) 
      {
        oa_annotator.setConflater( conflater );
        oa_annotator.open( offsetAnnotationsPath );
      }

    targetRepo.create(idx2, &repo);
      
    // for each document in the source repo, fetch ParsedDocument 
    // construct full rep, apply annotator, insert into
    // target repo.

    _index = sourceRepo.indexes()->front(); // presume 1
    _docIter = _index->termListFileIterator();
    _docIter->startIteration();
    // ought to deal with deleted documents here...
    // if there are deleted documents, regular add to collection
    // if not, only rewrite the indexes, then rename the collection.
    indri::index::DeletedDocumentList& deleted = sourceRepo.deletedList();
    UINT64 delCount = deleted.deletedCount();
    if (delCount > 0) 
      {
        // either warn, compact and then process, or 
        // do it the old way... FIXME!
        std::cerr << "Deleted documents detected... compact with dumpindex first." << std::endl;
        return;
      }
    
    for (UINT64 docid = 1; docid <= _index->documentCount(); docid++) 
      {
        if ((docid % 500) == 0)  {
          g_timer.printElapsedSeconds(std::cout);
          std::cout << ": " << docid << "\r";
          std::cout.flush();
        }

        parsed = sourceRepo.collection()->retrieve(docid);
        // combine field and term data with parsed document
        _mergeData();
        // apply annotator
        if (adding)
          parsed = oa_annotator.transform(parsed);
        targetRepo.addDocument(parsed, false);
        // TagList allocs memory for the tags...
        for (size_t i = 0; i < parsed->tags.size(); i++)
          delete(parsed->tags[i]);
        delete(parsed);
        _docIter->nextEntry();
      }
    std::cout << std::endl;
    g_timer.printElapsedSeconds(std::cout);
    std::cout << ": " << _index->documentCount() << std::endl;
    g_timer.printElapsedSeconds(std::cout);
    std::cout << ": closing"  << std::endl;

    targetRepo.close();
    sourceRepo.close();
    std::string oldcollectionPath = indri::file::Path::combine( index, "collection" );
    std::string newcollectionPath = indri::file::Path::combine( idx2, "collection" );
    // clone the collection
    indri::file::Path::remove(newcollectionPath);
    indri::file::Path::rename(oldcollectionPath, newcollectionPath);
    // rename target repo to source repo.
    indri::file::Path::remove(index);
    indri::file::Path::rename(idx2, index);

    g_timer.printElapsedSeconds(std::cout);
    std::cout << ": done"  << std::endl;
  }
};


int main( int argc, char* argv[] ) {
  try 
    {
      indri::api::Parameters param;
      FieldModifier * mod = new FieldModifier();
      param.loadCommandLine( argc, argv );
      mod->processFields(param);
      delete(mod);
    } catch( lemur::api::Exception& e ) {
    std::cout<< "# EXCEPTION: " << e.what() << std::endl;
  }
}

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

#include "indri/FileClassEnvironmentFactory.hpp"
#include "indri/DocumentIteratorFactory.hpp"
#include "indri/ParserFactory.hpp"
#include "indri/TokenizerFactory.hpp"
#include <ctype.h>
#include <map>
#include <vector>

struct file_class_environment_spec {
  const char* name;
  const char* parser;
  const char* tokenizer;
  const char* iterator;

  // iterator tag information
  const char* startDocTag;
  const char* endDocTag;
  const char* endMetadataTag;

  // parser tag information
  const char** includeTags;
  const char** excludeTags;
  const char** indexTags;
  const char** metadataTags;
  const char** conflations;
};


//
// Preconfigured environments
//

// For the conflations arrays, the format is groups of four: tag name,
// attribute name and value to match, followed by name of the tag to
// conflate to.  NULL is used as a wildcard in the conflations arrays.

// All tag names and attribute names entered here must be in lower
// case.  Values specified here can be in mixed case, since values are
// matched in a case-sensitive manner.

static const char* pdf_index_tags[] = { "title", "author", 0 };
static const char* pdf_metadata_tags[] = { "title", "author", 0 };
static const char* html_index_tags[] = { "title", "author", "h1", "h2", "h3", "h4", 0 };
static const char* html_metadata_tags[] = { "title", "author", 0 };
//static const char* html_conflations[] = { "h1", NULL, NULL, "heading", "h2", NULL, NULL, "heading", "h3", NULL, NULL, "heading", "h4", NULL, NULL, "heading", "bloghpno", NULL, NULL, "docno", 0, 0, 0, 0 };
/*
TODO:
A Meta Author tag declares the author of the HTML or XML document of a website. An example of a Meta Author tag is as follows:
<META NAME="Author" CONTENT="George Costanza, gcostanza@vandalayindustries.com">
"meta","name","author","author",  //make the parser understand META and grab the CONTENT implicitly
or make  html_attribute_conflations[]={"meta","name","author","content","author",0} this has maybe big impact on the interfaces
*/
static const char* html_conflations[] = { "h1", NULL, NULL, "heading", "h2", NULL, NULL, "heading", "h3", NULL, NULL, "heading", "h4", NULL, NULL, "heading", 0, 0, 0, 0 };
static const char* trec_include_tags[] = { "text", "hl", "head", "headline", "title", "ttl", "dd", "date", "date_time", "lp", "leadpara", 0 };
static const char* trecalt_include_tags[] = { "text", 0 };
static const char* trecalt_index_tags[] = { "text", 0 };
static const char* trec_metadata_tags[] = { "docno", "title", "author", 0 };
static const char* trec_conflations[] = { "hl", NULL, NULL, "headline", "head", NULL, NULL, "headline", "ttl", NULL, NULL, "title", "dd", NULL, NULL, "date", "date_time", NULL, NULL, "date", 0, 0, 0, 0 };
static const char* trec_index_tags[] = { "author", "hl", "head", "headline", "title", "ttl", "dd", "date_time", "date", 0 };
static const char* html_exclude_tags[] = { "script", "style", 0};

struct extension_conflations {
  const char **alternates;
};

// form is canonical extension alternate1 alternate2... NULL
static const char* _html[] = {"html", "htm", 0};
static const char* _txt[] =  {"txt", "text", 0};
static const char* _doc[] =  {"doc", "docx", 0};
static const char* _ppt[] =  {"ppt", "pptx", 0};

static extension_conflations extensions[] = {
  _html, _txt, _doc, _ppt, {0}
};

static std::string _canonicalExtension (const std::string &name) {
  std::string retval = name;
  for (int i = 0; extensions[i].alternates; i++) {
    for (int j = 0; extensions[i].alternates[j]; j++) {
      if( !strcmp( name.c_str(), extensions[i].alternates[j] ) ) {
        return extensions[i].alternates[0];
      }
    }
  }
  return name;
}
  
static file_class_environment_spec environments[] = {
  {
    "html",               // name
    "html",               // parser
    "word",               // tokenizer
    "text",               // iterator
    NULL,                 // startDocTag
    NULL,                 // endDocTag
    NULL,                 // endMetadataTag
    NULL,                 // includeTags
    html_exclude_tags,    // excludeTags
    html_index_tags,      // indexTags
    html_metadata_tags,   // metadataTags
    html_conflations      // conflations
  },
  {
    "xml",               // name
    "xml",               // parser
    "word",               // tokenizer
    "text",               // iterator
    NULL,                 // startDocTag
    NULL,                 // endDocTag
    NULL,                 // endMetadataTag
    NULL,                 // includeTags
    NULL,                 // excludeTags
    NULL,                 // indexTags
    NULL,                 // metadataTags
    NULL                  // conflations
  },
  {
    "warc",               // name
    "html",               // parser
    "word",               // tokenizer
    "warc",             // iterator
    NULL,              // startDocTag
    NULL,             // endDocTag
    NULL,          // endMetadataTag
    NULL,                 // includeTags
    html_exclude_tags,    // excludeTags
    html_index_tags,      // indexTags
    html_metadata_tags,   // metadataTags
    html_conflations      // conflations
  },
  {
    "warcchar",               // name
    "html",               // parser
    "char",               // tokenizer
    "warc",             // iterator
    NULL,              // startDocTag
    NULL,             // endDocTag
    NULL,          // endMetadataTag
    NULL,                 // includeTags
    html_exclude_tags,    // excludeTags
    html_index_tags,      // indexTags
    html_metadata_tags,   // metadataTags
    html_conflations      // conflations
  },
  {
    "trecweb",            // name
    "html",               // parser
    "word",               // tokenizer
    "tagged",             // iterator
    "<DOC>",              // startDocTag
    "</DOC>",             // endDocTag
    "</DOCHDR>",          // endMetadataTag
    NULL,                 // includeTags
    html_exclude_tags,    // excludeTags
    html_index_tags,      // indexTags
    html_metadata_tags,   // metadataTags
    html_conflations      // conflations
  },

  {
    "trectext",           // name
    "xml",                // parser
    "word",               // tokenizer
    "tagged",             // iterator
    "<DOC>",              // startDocTag
    "</DOC>",             // endDocTag
    NULL,                 // endMetadataTag
    trec_include_tags,    // includeTags
    NULL,                 // excludeTags
    trec_index_tags,      // indexTags
    trec_metadata_tags,   // metadataTags
    trec_conflations      // conflations
  },
  {
    "trecchar",           // name
    "xml",                // parser
    "char",               // tokenizer
    "tagged",             // iterator
    "<DOC>",              // startDocTag
    "</DOC>",             // endDocTag
    NULL,                 // endMetadataTag
    trec_include_tags,    // includeTags
    NULL,                 // excludeTags
    trec_index_tags,      // indexTags
    trec_metadata_tags,   // metadataTags
    trec_conflations      // conflations
  },
  {
    "trecalt",           // name
    "xml",                // parser
    "word",               // tokenizer
    "tagged",             // iterator
    "<DOC>",              // startDocTag
    "</DOC>",             // endDocTag
    NULL,                 // endMetadataTag
    trecalt_include_tags,    // includeTags
    NULL,                 // excludeTags
    trecalt_index_tags,      // indexTags
    trec_metadata_tags,   // metadataTags
    trec_conflations      // conflations
  },

  {
    "mbox",               // name
    "text",               // parser
    "word",               // tokenizer
    "mbox",               // iterator
    NULL,                 // startDocTag
    NULL,                 // endDocTag
    NULL,                 // endMetadataTag
    NULL,                 // includeTags
    NULL,                 // excludeTags
    NULL,                 // indexTags
    NULL,                 // metadataTags
    NULL                  // conflations
  },

#ifdef WIN32
  {
    "doc",                // name
    "html",               // parser
    "word",               // tokenizer
    "doc",                // iterator
    NULL,                 // startDocTag
    NULL,                 // endDocTag
    NULL,                 // endMetadataTag
    NULL,                 // includeTags
    NULL,                 // excludeTags
    NULL,                 // indexTags
    NULL,                 // metadataTags
    NULL                  // conflations
  },

  {
    "ppt",                // name
    "html",               // parser
    "word",               // tokenizer
    "ppt",                // iterator
    NULL,                 // startDocTag
    NULL,                 // endDocTag
    NULL,                 // endMetadataTag
    NULL,                 // includeTags
    NULL,                 // excludeTags
    NULL,                 // indexTags
    NULL,                 // metadataTags
    NULL                  // conflations
  },
#endif

  {
    "pdf",                // name
    "html",               // parser
    "word",               // tokenizer
    "pdf",                // iterator
    NULL,                 // startDocTag
    NULL,                 // endDocTag
    NULL,                 // endMetadataTag
    NULL,                 // includeTags
    NULL,                 // excludeTags
    pdf_index_tags,       // indexTags
    pdf_metadata_tags,    // metadataTags
    NULL                  // conflations
  },

  {
    "txt",                // name
    "text",               // parser
    "word",               // tokenizer
    "text",               // iterator
    NULL,                 // startDocTag
    NULL,                 // endDocTag
    NULL,                 // endMetadataTag
    NULL,                 // includeTags
    NULL,                 // excludeTags
    NULL,                 // indexTags
    NULL,                 // metadataTags
    NULL                  // conflations
  },

  // null ending marker
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

static void copy_strings_to_vector( std::vector<std::string>& vec, const char** array ) {
  if( array ) {
    for( unsigned int i=0; array[i]; i++ ) {
      vec.push_back(array[i]);
    }
  }
}

static void copy_string_tuples_to_map( std::map<indri::parse::ConflationPattern*,std::string>& m, const char** array ) {

  if ( array ) {
    for ( unsigned int i = 0; array[i] && array[i + 3]; i += 4 ) {

      // The strings in the array are assumed to be in appropriate
      // case: for tag name and attribute name to match, as well as
      // the name of the tag to conflate to, this means lowercase.
      // Attribute value to match may be in mixed case.  Attribute
      // name and value can be NULL, which stands for a "don't care"
      // value.

      indri::parse::ConflationPattern* key =
        new indri::parse::ConflationPattern;

      key->tag_name = array[i];
      key->attribute_name = array[i + 1];
      key->value = array[i + 2];

      std::string value = array[i + 3];
       m[key] = value;
     }
   }
}

static void cleanup_conflations_map( std::map<indri::parse::ConflationPattern*,std::string>&
                                     conflations ) {

  for ( std::map<indri::parse::ConflationPattern*,std::string>::iterator i =
          conflations.begin(); i != conflations.end(); i++ )
    delete (*i).first;
}


indri::parse::FileClassEnvironmentFactory::~FileClassEnvironmentFactory() {
  std::map<std::string, indri::parse::FileClassEnvironmentFactory::Specification*>::iterator iter;

  for( iter = _userTable.begin(); iter != _userTable.end(); iter++ ) {
    delete iter->second;
  }
}

indri::parse::FileClassEnvironment* build_file_class_environment( const file_class_environment_spec* spec ) {
  indri::parse::FileClassEnvironment* env = new indri::parse::FileClassEnvironment;
  env->iterator = indri::parse::DocumentIteratorFactory::get( spec->iterator, spec->startDocTag, spec->endDocTag, spec->endMetadataTag );

  std::vector<std::string> includeTags;
  std::vector<std::string> excludeTags;
  std::vector<std::string> indexTags;
  std::vector<std::string> metadataTags;
  std::map<indri::parse::ConflationPattern*, std::string> conflations;

  copy_strings_to_vector( includeTags, spec->includeTags );
  copy_strings_to_vector( excludeTags, spec->excludeTags );
  copy_strings_to_vector( indexTags, spec->indexTags );
  copy_strings_to_vector( metadataTags, spec->metadataTags );
  copy_string_tuples_to_map( conflations, spec->conflations );

  env->parser = indri::parse::ParserFactory::get( spec->parser, includeTags, excludeTags, indexTags, metadataTags, conflations );

  env->tokenizer = indri::parse::TokenizerFactory::get( spec->tokenizer );

  env->conflater = new indri::parse::Conflater( conflations );

  cleanup_conflations_map( conflations );

  return env;
}

indri::parse::FileClassEnvironment* build_file_class_environment( const indri::parse::FileClassEnvironmentFactory::Specification * spec ) {
  indri::parse::FileClassEnvironment* env = new indri::parse::FileClassEnvironment;
  env->iterator = indri::parse::DocumentIteratorFactory::get( spec->iterator,
                                                              spec->startDocTag.c_str(),
                                                              spec->endDocTag.c_str(),
                                                              spec->endMetadataTag.c_str() );

  env->parser = indri::parse::ParserFactory::get( spec->parser,
                                                  spec->include,
                                                  spec->exclude,
                                                  spec->index,
                                                  spec->metadata,
                                                  spec->conflations );

  env->tokenizer = indri::parse::TokenizerFactory::get( spec->tokenizer );

  env->conflater = new indri::parse::Conflater( spec->conflations );

  return env;
}

indri::parse::FileClassEnvironmentFactory::Specification* indri::parse::FileClassEnvironmentFactory::getFileClassSpec( const std::string& name ) {
  std::string canonicalName = _canonicalExtension(name);
  
  // look for a user-specified environment:
  std::map<std::string, indri::parse::FileClassEnvironmentFactory::Specification*>::iterator iter;
  iter = _userTable.find( canonicalName );

  if( iter != _userTable.end() ) {
    // copy and return;
    indri::parse::FileClassEnvironmentFactory::Specification* spec = new indri::parse::FileClassEnvironmentFactory::Specification;
    *spec = *(iter->second);
    return spec;
  }
  // look for a default environment
  const file_class_environment_spec* spec = 0;
  for( unsigned int i=0; environments[i].name; i++ ) {
    if( !strcmp( canonicalName.c_str(), environments[i].name ) ) {
      spec = &environments[i];
      break;
    }
  }

  if( spec ) {
    indri::parse::FileClassEnvironmentFactory::Specification* newSpec = new indri::parse::FileClassEnvironmentFactory::Specification;


    std::vector<std::string> includeTags;
    std::vector<std::string> excludeTags;
    std::vector<std::string> indexTags;
    std::vector<std::string> metadataTags;
    std::map<indri::parse::ConflationPattern*, std::string> conflations;

    copy_strings_to_vector( includeTags, spec->includeTags );
    copy_strings_to_vector( excludeTags, spec->excludeTags );
    copy_string_tuples_to_map( conflations, spec->conflations );
    copy_strings_to_vector(indexTags, spec->indexTags);
    copy_strings_to_vector(metadataTags, spec->metadataTags);

    newSpec->name = spec->name;
    newSpec->iterator = spec->iterator;
    newSpec->parser = spec->parser;
    newSpec->tokenizer = spec->tokenizer;
    newSpec->index = indexTags;
    newSpec->metadata = metadataTags;
    newSpec->include = includeTags;
    newSpec->exclude = excludeTags;
    newSpec->conflations = conflations;
    newSpec->startDocTag = spec->startDocTag ? spec->startDocTag : "";
    newSpec->endDocTag = spec->endDocTag ? spec->endDocTag : "" ;
    newSpec->endMetadataTag = spec->endMetadataTag ? spec->endMetadataTag : "";
    return newSpec;
  }
  return 0;
}

indri::parse::FileClassEnvironment* indri::parse::FileClassEnvironmentFactory::get( const std::string& name ) {
  std::string canonicalName = _canonicalExtension(name);
  
  // look for a user-specified environment:
  std::map<std::string, indri::parse::FileClassEnvironmentFactory::Specification*>::iterator iter;
  iter = _userTable.find( canonicalName );

  if( iter != _userTable.end() ) {
    return build_file_class_environment( iter->second );
  }

  // look for a default environment
  const file_class_environment_spec* spec = 0;

  for( unsigned int i=0; environments[i].name; i++ ) {
    if( !strcmp( canonicalName.c_str(), environments[i].name ) ) {
      spec = &environments[i];
      break;
    }
  }

  if( spec )
    return build_file_class_environment( spec );

  return 0;
}

void indri::parse::FileClassEnvironmentFactory::addFileClass( const std::string& name,
                                                              const std::string& iterator,
                                                              const std::string& parser,
                                                              const std::string& tokenizer,
                                                              const std::string& startDocTag,
                                                              const std::string& endDocTag,
                                                              const std::string& endMetadataTag,
                                                              const std::vector<std::string>& include,
                                                              const std::vector<std::string>& exclude,
                                                              const std::vector<std::string>&
                                                              index,
                                                              const std::vector<std::string>& metadata,
                                                              const std::map<indri::parse::ConflationPattern*,std::string>& conflations )
{
  indri::parse::FileClassEnvironmentFactory::Specification* spec = new indri::parse::FileClassEnvironmentFactory::Specification;

  spec->name = name;
  spec->iterator = iterator;
  spec->parser = parser;
  spec->tokenizer = tokenizer;

  spec->include = include;
  spec->exclude = exclude;
  spec->metadata = metadata;
  spec->index = index;
  spec->conflations = conflations;

  spec->startDocTag = startDocTag;
  spec->endDocTag = endDocTag;
  spec->endMetadataTag = endMetadataTag;

  _userTable[spec->name] = spec;
}

void indri::parse::FileClassEnvironmentFactory::addFileClass( const indri::parse::FileClassEnvironmentFactory::Specification &spec) {
  // make a copy
  indri::parse::FileClassEnvironmentFactory::Specification* newSpec = new indri::parse::FileClassEnvironmentFactory::Specification;
  *newSpec = spec;
  // see if there is already an entry for this name
  std::map<std::string, indri::parse::FileClassEnvironmentFactory::Specification*>::iterator iter;
  iter = _userTable.find( newSpec->name );
  // delete it.
  if( iter != _userTable.end() ) {
    delete(iter->second);
  }
  _userTable[newSpec->name] = newSpec;
}

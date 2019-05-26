/*==========================================================================
 * Copyright (c) 2005 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


//
// Conflater
//
// 15 September 2005 -- mwb
//

// Simple wrapper for tag conflation code, so that it can be called
// both from the Parser as it processes incoming text, and from the
// OffsetAnnotationsAnnotator as it reads from the offset annotations
// file.

#ifndef INDRI_CONFLATER_HPP
#define INDRI_CONFLATER_HPP

#include <vector>
#include <ctype.h>
#include <string.h>
#include "indri/Buffer.hpp"
#include "indri/TagExtent.hpp"
#include "indri/TagEvent.hpp"
#include "indri/ConflationPattern.hpp"
#include "indri/HashTable.hpp"
#include "indri/AttributeValuePair.hpp"

namespace indri {
  namespace parse {

    class Conflater {

    private:
      indri::utility::Buffer _stringBuf;
      const char *_strdup(const char * token) {
        size_t token_len = strlen(token);
        char* write_loc = _stringBuf.write( token_len + 1 );
        memcpy( write_loc, token, token_len + 1 );
        return write_loc;
      }
      
      
    protected:

      struct attribute_pattern {
        indri::utility::HashTable<const char*,const char*> values;
        bool match_attribute_name;   // true if pattern matches att name alone
        const char* conflation;      // must be set if above flag is true
      };

      struct tag_pattern {
        indri::utility::HashTable<const char*,attribute_pattern*> attributes;
        bool match_tag_name;         // true if pattern matches tag name alone
        const char* conflation;      // must be set if above flag is true
      };

      indri::utility::HashTable<const char *,tag_pattern*> _pattern_table;
      
      // The process of conflating a tag structure is O(nm) in the
      // number of attributes, n, and in the number of matching
      // attribute-value pattern, m.

      const char *_lookup( const char* name, indri::utility::greedy_vector<indri::parse::AttributeValuePair, 2>& attributes ) {

        tag_pattern** p = _pattern_table.find( name );
        tag_pattern* p_tag_pattern;

        if ( ! p ) return NULL; // No patterns match this tag name.

        p_tag_pattern = *p;

        // Now, we iterate through the attributes present in the
        // TagExtent to see if there are patterns that match any of them.

        attribute_pattern** q;
        attribute_pattern* p_attribute_pattern;

        for ( indri::utility::greedy_vector<AttributeValuePair, 2>::iterator i =
                attributes.begin(); i != attributes.end(); i++ ) {

          q = p_tag_pattern->attributes.find( (*i).attribute );

          if ( q ) {

            p_attribute_pattern = *q;

            // There is a pattern that matches this attribute name, so
            // iterate through the values in the attribute_pattern to
            // see if they match.
            
            for ( indri::utility::HashTable<const char*,const char*>::iterator
                    j = p_attribute_pattern->values.begin();
                  j != p_attribute_pattern->values.end(); j++ ) {

              if ( ! strcmp( *(*j).first, (*i).value ) ) {

                // We have an attribute-value match, so conflate tag.
                return *(*j).second;
              }
            }

            // At this point, none of the values matched exactly.  Can
            // we still conflate based on the presence of the
            // attribute alone?

            if ( p_attribute_pattern->match_attribute_name ) {

              return p_attribute_pattern->conflation;
            }
          }
        }

        // At this point, none of the attributes matched exactly.  Can
        // we still conflate based on the name of the tag?

        if ( p_tag_pattern->match_tag_name ) {

          return p_tag_pattern->conflation;
        }
        return NULL;
      }

    public:
      Conflater( const std::map<ConflationPattern*,std::string>& conflations ) {
        //allocate some space for the strings:
        _stringBuf.grow(10*1024); // 10K should be plenty.

        // Build _pattern_table.  The assumption is that patterns for
        // tag names and attribute names are already downcased, so
        // that they will match the strings coming out of the
        // Tokenizer exactly.

        for ( std::map<ConflationPattern*,std::string>::const_iterator i =
                conflations.begin(); i != conflations.end(); i++ ) {

          const ConflationPattern* p_cp = (*i).first;
          std::string conflation = (*i).second;

          tag_pattern** p =       
            _pattern_table.find( p_cp->tag_name );
          tag_pattern* p_tag_pattern;

          // Ensure tag_pattern exists for the tag_name we are dealing with.

          if ( ! p ) {

            p_tag_pattern = new tag_pattern;
            p_tag_pattern->match_tag_name = false;
            _pattern_table.insert( p_cp->tag_name, p_tag_pattern );

          } else {

            p_tag_pattern = *p;
          }

          // If the current ConflationPattern is a tag_name match only:

          if ( ( ! p_cp->attribute_name ) &&
               ( ! p_cp->value ) ) {

            p_tag_pattern->match_tag_name = true;
            p_tag_pattern->conflation = _strdup(conflation.c_str());
            continue;
          }

          attribute_pattern** q =
            p_tag_pattern->attributes.find( p_cp->attribute_name );
          attribute_pattern* p_attribute_pattern;

          // Ensure attribute_pattern exists for the attribute_name we
          // are dealing with:

          if ( ! q ) {

            p_attribute_pattern = new attribute_pattern;
            p_attribute_pattern->match_attribute_name = false;
            p_tag_pattern->attributes.insert( p_cp->attribute_name, 
                                              p_attribute_pattern );

          } else {

            p_attribute_pattern = *q;
          }

          // If the current ConflationPattern is an attribute_name match
          // only:

          if ( ! p_cp->value ) {

            p_attribute_pattern->match_attribute_name = true;
            p_attribute_pattern->conflation = _strdup(conflation.c_str());
            continue;
          }
          
          // Otherwise, it is an attribute-value match:

          p_attribute_pattern->values.insert( p_cp->value, _strdup(conflation.c_str()) );

        }       
      }

      ~Conflater() {

        // TODO: improve this by using delete_hash_table?

        // Clean up _pattern_table

        for ( indri::utility::HashTable<const char*,tag_pattern*>
                ::iterator i = _pattern_table.begin(); 
              i != _pattern_table.end(); i++ ) {

          tag_pattern* p_tag_pattern = *(*i).second;

          for ( indri::utility::HashTable<const char*,attribute_pattern*>
                  ::iterator j = p_tag_pattern->attributes.begin();
                j != p_tag_pattern->attributes.end(); j++ ) {

            attribute_pattern* p_attribute_pattern = *(*j).second;

            delete p_attribute_pattern;
          }
          
          delete p_tag_pattern;
        }
      }

      // Two convenient interfaces into the tag conflation algorithm.

      // When a tag is conflated, its attributes are wiped out
      // completely, and its tag name is changed to the conflation
      // string.

      void conflate( TagEvent* tev ) {

        const char *new_name = _lookup( tev->name, tev->attributes );

        if ( new_name ) {

          tev->name = new_name;
          tev->attributes.clear();
        }
      }

      void conflate( TagExtent* tex ) {

        const char *new_name = _lookup( tex->name, tex->attributes );

        if ( new_name ) {

          tex->name = new_name;
          tex->attributes.clear();
        }
      }

      /// return the conflation for this tag name, if one exists.
      /// otherwise, return input argument.
      /// Does not handle the case of attribute dependent conflations
      const char * conflate( const char* tagname ) {
        indri::utility::greedy_vector<AttributeValuePair,2> attributes;
        const char *new_name = _lookup( tagname, attributes );

        if ( ! new_name ) {
          new_name = tagname;
        }
        return new_name;
      }
    };
  }
}


#endif // INDRI_CONFLATER_HPP

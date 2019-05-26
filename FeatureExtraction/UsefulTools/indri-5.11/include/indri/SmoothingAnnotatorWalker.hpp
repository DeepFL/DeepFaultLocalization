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
// SmoothingAnnotatorWalker
//
// 27 April 2004 -- tds
//

#ifndef INDRI_SMOOTHINGANNOTATORWALKER_HPP
#define INDRI_SMOOTHINGANNOTATORWALKER_HPP

#include "indri/Parameters.hpp"
namespace indri
{
  namespace lang
  {
    
    class SmoothingAnnotatorWalker : public indri::lang::Walker {
    private:
      struct rule_type {
        std::string node;
        std::string field;
        std::string op;
        std::string smoothing;
      };

      std::vector<rule_type*> _rules;
      std::string _defaultSmoothing;

      void _loadSmoothingRules( indri::api::Parameters& parameters ) {
        if( !parameters.exists("rule") )
          return;

        indri::api::Parameters rules = parameters["rule"];

        for(size_t i=0; i<rules.size(); i++) {
          std::string ruleText = rules[i];

          int nextComma = 0;
          int nextColon = 0;
          int location = 0;

          rule_type* rule = new rule_type;
          rule->node = "RawScorerNode";
          rule->op = "*";
          rule->field = "*";

          for( location = 0; location < ruleText.length(); ) {
            nextComma = ruleText.find( ',', location );
            nextColon = ruleText.find( ':', location );

            std::string key = ruleText.substr( location, nextColon-location );
            std::string value = ruleText.substr( nextColon+1, nextComma-nextColon-1 );

            if( key == "node" ) {
              rule->node = value;
            } else if( key == "field" ) {
              rule->field = value;
            } else if( key == "operator" ) {
              rule->op = value;
            }  else {
              if( rule->smoothing.size() ) rule->smoothing += ",";
              rule->smoothing += key + ":" + value;
            }

            if( nextComma > 0 )
              location = nextComma+1;
            else
              location = ruleText.size();
          }

          _rules.push_back(rule);
        }
      }

      const std::string& _matchSmoothingRule( const std::string& node, const std::string& field, const std::string& op ) {
        for( int i=signed(_rules.size())-1; i >= 0; i-- ) {
          const rule_type& rule = *_rules[i];

          if( ( rule.node == node ) &&
              ( rule.field == field || rule.field == "*" ) &&
              ( rule.op == op || rule.op == "*" ) ) {
            return rule.smoothing;
          }
        }

        return _defaultSmoothing;
      }

    public:
      SmoothingAnnotatorWalker( indri::api::Parameters& parameters ) {
        _loadSmoothingRules( parameters );
        _defaultSmoothing = "method:dirichlet,mu:2500";
      }

      ~SmoothingAnnotatorWalker( ) {
        indri::utility::delete_vector_contents<rule_type*>( _rules );
      }

      void after( indri::lang::RawScorerNode* scorer ) {
        indri::lang::Node* context = scorer->getContext();
        indri::lang::Field* contextField = dynamic_cast<indri::lang::Field*>(context);
        indri::lang::ExtentOr* contextExtOr = dynamic_cast<indri::lang::ExtentOr*>(context);
        std::string fieldName;

        // there may be an ExtentOr around the field, so descend into it if necessary
        if( contextExtOr && contextExtOr->getChildren().size() == 1 ) {
          contextField = dynamic_cast<indri::lang::Field*>(contextExtOr->getChildren()[0]);
        }

        // if there's a field here, record its name
        if( contextField ) {
          fieldName = contextField->getFieldName();
        } else {
          fieldName = "?";
        }
    
        indri::lang::Node* raw = scorer->getRawExtent();
        indri::lang::Node* rawTerm = dynamic_cast<indri::lang::IndexTerm*>(raw);
        indri::lang::Node* rawODNode = dynamic_cast<indri::lang::ODNode*>(raw);
        indri::lang::Node* rawUWNode = dynamic_cast<indri::lang::UWNode*>(raw);
        indri::lang::Node* rawWeightedExtentOr = dynamic_cast<indri::lang::WeightedExtentOr*>(raw);

        std::string op;

        if( rawODNode || rawUWNode ) {
          op = "window";
        } else if( rawTerm || rawWeightedExtentOr ) {
          op = "term";
        } else {
          op = "?";
        }

        scorer->setSmoothing( _matchSmoothingRule( "RawScorerNode", fieldName, op ) );
      }

      void after( indri::lang::NestedRawScorerNode* scorer ) {
        after( (indri::lang::RawScorerNode *) scorer );
      }

      void after( indri::lang::ShrinkageScorerNode* scorer ) {
        after( (indri::lang::RawScorerNode *) scorer );

        for( int i=signed(_rules.size())-1; i >= 0; i-- ) {
          const rule_type& rule = *_rules[i];

          if( rule.node == "ShrinkageBelief" &&
              rule.op == "*" ) {
            if ( rule.field == "*" ) {
              scorer->addShrinkageRule( rule.smoothing );
            } else {
              std::string ruleString = "field:" + rule.field + "," + rule.smoothing;
              scorer->addShrinkageRule( ruleString );
            }
          }
        }
      }

      void after( indri::lang::LengthPrior* prior ) {
        std::string ruleText = _matchSmoothingRule( "LengthPrior", "*", "*" );
        double exponent = 0;

        int nextComma = 0;
        int nextColon = 0;
        int location = 0;
        
        for( location = 0; location < ruleText.length(); ) {
          nextComma = ruleText.find( ',', location );
          nextColon = ruleText.find( ':', location );

          std::string key = ruleText.substr( location, nextColon-location );
          std::string value = ruleText.substr( nextColon+1, nextComma-nextColon-1 );

          if( key == "exponent" ) {
            exponent = atof( value.c_str() );       
          } 
          if( nextComma > 0 )
            location = nextComma+1;
          else
            location = ruleText.size();
        }
        
        prior->setExponent( exponent );
      }
    };
  }
}

#endif // INDRI_SMOOTHINGANNOTATORWALKER_HPP


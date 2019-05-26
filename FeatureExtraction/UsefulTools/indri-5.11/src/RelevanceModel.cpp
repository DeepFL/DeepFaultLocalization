
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
// relevancemodel
//
// 23 June 2005 -- tds
//

#include "indri/RelevanceModel.hpp"
#include <math.h>

//
// RelevanceModel
//

indri::query::RelevanceModel::RelevanceModel( 
                  indri::api::QueryEnvironment& environment,
                  const std::string& smoothing,
                  int maxGrams,
                  int documents )
  :
  _environment(environment),
  _smoothing(smoothing),
  _documents(documents),
  _maxGrams(maxGrams)
{
}

//
// ~RelevanceModel
//

indri::query::RelevanceModel::~RelevanceModel() {
  HGram::iterator iter;

  for( iter = _gramTable.begin(); iter != _gramTable.end(); iter++ ) {
    delete *(iter->second);
  }
}

//
// getGrams
//

const std::vector<indri::query::RelevanceModel::Gram*>& indri::query::RelevanceModel::getGrams() const {
  return _grams;
}

//
// getQueryResults
//

const std::vector<indri::api::ScoredExtentResult>& indri::query::RelevanceModel::getQueryResults() const {
  return _results;
}

//
// _extractDocuments
//

void indri::query::RelevanceModel::_extractDocuments() {
  for( size_t i=0; i<_results.size(); i++ ) {
    _documentIDs.push_back( _results[i].document );
  }
}

//
// _countGrams
//
// Builds a hash table of grams, and counts the times that each
// gram occurs in each query result.
//

bool isValidWord(const string & word)
{
  size_t length = word.size();
  const char * chArray = word.c_str();
  size_t pos = 0;

  while (pos < length)
  { 
    if(isalnum((unsigned char)*(chArray+pos)) == 0)
    {
      return false;
    }
    pos ++;
  }
  return true;
}

void indri::query::RelevanceModel::_countGrams() {
  // for each query result
  for( size_t i=0; i<_results.size(); i++ ) {
    // run through the text, extracting n-grams
    indri::api::ScoredExtentResult& result = _results[i];
    indri::api::DocumentVector* v = _vectors[i];
    std::vector<int>& positions = v->positions();
    std::vector<std::string>& stems = v->stems();
    if (result.end == 0) result.end = positions.size();
    
    // for each word position in the text
    for( int j = result.begin; j < result.end; j++ ) {
      int maxGram = std::min( _maxGrams, result.end - j );

      // extract every possible n-gram that starts at this position
      // up to _maxGrams in length
      for( int n = 1; n <= maxGram; n++ ) {
        GramCounts* newCounts = new GramCounts;
        bool containsOOV = false;

        // build the gram
        for( int k = 0; k < n; k++ ) {
          if( positions[ k + j ] == 0 || (! isValidWord(stems[ positions[ k + j ] ])) ) {
            containsOOV = true;
            break;
          }

          newCounts->gram.terms.push_back( stems[ positions[ k + j ] ] );
        }

        if( containsOOV ) {
          // if this contanied OOV, all larger n-grams
          // starting at this point also will
          delete newCounts;
          break;
        }

        GramCounts** gramCounts = 0;        
        gramCounts = _gramTable.find( &newCounts->gram );

        if( gramCounts == 0 ) {
          _gramTable.insert( &newCounts->gram, newCounts );
          gramCounts = &newCounts;
        } else {
          delete newCounts;
        }

        if( (*gramCounts)->counts.size() && (*gramCounts)->counts.back().first == i ) {
          // we already have some counts going for this query result, so just add this one
          (*gramCounts)->counts.back().second++;
        } else {
          // no counts yet in this document, so add an entry
          (*gramCounts)->counts.push_back( std::make_pair( i, 1 ) );
        }
      }
    }
  }
}

//
// _scoreGrams
//

void indri::query::RelevanceModel::_scoreGrams() {
  HGram::iterator iter;
  double collectionCount = (double)_environment.termCount();
  indri::query::TermScoreFunction* function = 0;  

  // for each gram we've seen
  for( iter = _gramTable.begin(); iter != _gramTable.end(); iter++ ) {
    // gather the number of times this gram occurs in the collection
    double gramCount = 0;

    Gram* gram = *iter->first;
    GramCounts* gramCounts = *iter->second;

    if( _smoothing.length() != 0 ) {
      // it's only important to get background frequencies if
      // we're smoothing with them; otherwise we don't care.

      if( gram->terms.size() == 1 ) {
        gramCount = (double)_environment.stemCount( gram->terms[0] );
      } else {
        // notice that we're running a query here;
        // this is likely to be slow. (be warned)

        std::stringstream s;
        s << "#1( ";

        for( size_t i=0; i< gram->terms.size(); i++ ) {
          s << " \"" << gram->terms[i] << "\"" << std::endl;
        }

        s << ") ";
        gramCount = _environment.expressionCount( s.str() );
      }

      double gramFrequency = gramCount / collectionCount;
      //      function = indri::query::TermScoreFunctionFactory::get( _smoothing, gramFrequency );
      function = indri::query::TermScoreFunctionFactory::get( _smoothing, gramCount, collectionCount, 0 , 0 );
    }

    // now, aggregate scores for each retrieved item
    std::vector<indri::api::ScoredExtentResult>::iterator riter;
    double gramScore = 0;
    size_t c;
    size_t r;

    for( r = 0, c = 0; r < _results.size() && c < gramCounts->counts.size(); r++ ) {
      int contextLength = _results[r].end - _results[r].begin;
      // has been converted to a posterior probability.
      double documentScore = _results[r].score;
      double termScore = 0;
      double occurrences = 0;

      if( gramCounts->counts[c].first == r ) {
        // we have counts for this result
        occurrences = gramCounts->counts[c].second;
        c++;
      }

      // determine the score for this term
      if( function != 0 ) {
        // log probability here
        termScore = exp(function->scoreOccurrence( occurrences, contextLength ));
      } else {
        termScore = occurrences / double(contextLength);
      }
      //RMExpander weights this by 1/fbDocs
      // Unclear as to why.
      gramScore += documentScore * termScore;
      //gramScore += (1.0/_documents) * documentScore * termScore;
    }

    gram->weight = gramScore;
    delete function;
  }
}

//
// _sortGrams
//

void indri::query::RelevanceModel::_sortGrams() {
  // copy grams into a _grams vector
  HGram::iterator iter;
  _grams.clear();
  
  for( iter = _gramTable.begin(); iter != _gramTable.end(); iter++ ) {
    _grams.push_back( *(iter->first) );
  }

  std::sort( _grams.begin(), _grams.end(), Gram::weight_greater() );
}

// In:  log(x1) log(x2) ... log(xN)
// Out: x1/sum, x2/sum, ... xN/sum
//
// Extra care is taken to make sure we don't overflow
// machine precision when taking exp (log x)
// This is done by subtracting a constant K which cancels out
// Right now K is set to maximally preserve the highest value
// but could be altered to a min or average, or whatever...

static void _logtoposterior(std::vector<indri::api::ScoredExtentResult> &res) {
  if (res.size() == 0) return;
  std::vector<indri::api::ScoredExtentResult>::iterator iter;
  iter = res.begin();
  double K = (*iter).score;
  // first is max
  double sum=0;

  for (iter = res.begin(); iter != res.end(); iter++) {
    sum += (*iter).score=exp((*iter).score - K);
  }
  for (iter = res.begin(); iter != res.end(); iter++) {
    (*iter).score/=sum;
  }
}


//
// generate
//

void indri::query::RelevanceModel::generate( const std::string& query ) {
  try {
    // run the query, get the document vectors
    _results = _environment.runQuery( query, _documents );
    _logtoposterior(_results);
    _grams.clear();
    _extractDocuments();
    _vectors = _environment.documentVectors( _documentIDs );

    _countGrams();
    _scoreGrams();
    _sortGrams();
    for (unsigned int i = 0; i < _vectors.size(); i++)
      delete _vectors[i];
  } catch( lemur::api::Exception& e ) {
    LEMUR_RETHROW( e, "Couldn't generate relevance model for '" + query + "' because: " );
  }
}

//
// generate
//

void indri::query::RelevanceModel::generate( const std::string& query, const std::vector<indri::api::ScoredExtentResult>& results  ) {
  try {
    _results = results;
    _logtoposterior(_results);
    _grams.clear();
    _extractDocuments();
    _vectors = _environment.documentVectors( _documentIDs );

    _countGrams();
    _scoreGrams();
    _sortGrams();
    for (unsigned int i = 0; i < _vectors.size(); i++)
      delete _vectors[i];
  } catch( lemur::api::Exception& e ) {
    LEMUR_RETHROW( e, "Couldn't generate relevance model for '" + query + "' because: " );
  }
}


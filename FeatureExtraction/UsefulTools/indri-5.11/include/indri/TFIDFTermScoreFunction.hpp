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
// TFIDFScoreFunction
//
// 23 January 2004 -- tds
//

#ifndef INDRI_TFIDFTERMSCOREFUNCTION_HPP
#define INDRI_TFIDFTERMSCOREFUNCTION_HPP
#include <iostream>
#include "indri/TermScoreFunction.hpp"
#include <math.h>
namespace indri
{
  namespace query
  {
    
    class TFIDFTermScoreFunction : public TermScoreFunction {
    private:
      /// inverse document frequency (IDF) for this term
      double _inverseDocumentFrequency; 
      /// average document length in the collection
      double _averageDocumentLength;

      double _termWeight;

      // These are BM25 parameters
      double _k1;
      double _b;
      // okapi query term weight parameter
      double _k3;
      
      // The following values are precomputed so that score computation will go faster
      double _bOverAvgDocLength;
      double _k1TimesOneMinusB;
      double _idfTimesK1PlusOne;
      double _k1TimesBOverAvgDocLength;
      double _termWeightTimesIDFTimesK1;
      double _termWeightTimesidfTimesK1PlusOne;
      bool _okapi;
      
      void _precomputeConstants() {
        _idfTimesK1PlusOne = _inverseDocumentFrequency * ( _k1 + 1 );
        _k1TimesOneMinusB = _k1 * (1-_b);
        _bOverAvgDocLength = _b / _averageDocumentLength;
        _k1TimesBOverAvgDocLength = _k1 * _bOverAvgDocLength;
        _termWeightTimesIDFTimesK1 = _termWeight * _inverseDocumentFrequency * _k1; 
        _termWeightTimesidfTimesK1PlusOne = _termWeight * _idfTimesK1PlusOne;
      }

    public:
      TFIDFTermScoreFunction( double idf, double averageDocumentLength, int qTF = 1, double k1 = 1.2, double b = 0.75, bool okapi = false, double k3 = 7 ) {
        _okapi = okapi;
        _inverseDocumentFrequency = idf;
        _averageDocumentLength = averageDocumentLength;

        _k1 = k1;
        _b = b;
        _k3 = k3;
        // needs to be adjusted to _termWeight/_qTF to enable additive
        // scoring of terms when _qTF > 1 to get the values correct.
        _termWeight = queryTermWeight( 1000, 0, qTF ) / qTF;
        _precomputeConstants();
      }

      TFIDFTermScoreFunction( double idf, double averageDocumentLength, double qtw = 1.0, double k1 = 1.2, double b = 0.75, bool okapi = false, double k3 = 7 ) {
        _okapi = okapi;
        _inverseDocumentFrequency = idf;
        _averageDocumentLength = averageDocumentLength;

        _k1 = k1;
        _b = b;
        _k3 = k3;
        
        // if the weight is supplied, don't recompute it
        _termWeight = qtw;
        //        _termWeight = queryTermWeight( 1000, 0, qtw );
        _precomputeConstants();
      }

      double scoreOccurrence( double occurrences, int documentLength ) {
        if (_okapi) {
            
          // okapi
          //
          // Score function is:
          //                                                   (K1 + 1) * occurrences
          // score = termWeight * IDF * ------------------------------------------------------------------
          //                             occurrences + K1 * ( (1-B) + B * ( documentLength / avgDocLength) )
          //
          // Factored for constants:
          //                        (termWeight * IDF * (K1 + 1)) * occurrences
          // score = ------------------------------------------------------------------------
          //          occurrences + (K1 * (1-B)) + (K1 * B * 1/avgDocLength) * documentLength
          //
          double numerator = _termWeightTimesidfTimesK1PlusOne * occurrences;
          double denominator = occurrences + _k1TimesOneMinusB + _k1TimesBOverAvgDocLength * documentLength;
          return numerator / denominator; 
        } else {
          //simple tfidf
          //
          // Score function is:
          //                                                   K1 * occurrences
          // score = termWeight * IDF * ------------------------------------------------------------------
          //                             occurrences + K1 * ( (1-B) + B * ( documentLength / avgDocLength) )
          //
          // Factored for constants:
          //                        (termWeight * IDF * K1) * occurrences
          // score = ------------------------------------------------------------------------
          //          occurrences + (K1 * (1-B)) + (K1 * B * 1/avgDocLength) * documentLength
          //
            

          double numerator = _termWeightTimesIDFTimesK1 * occurrences;
          double denominator = occurrences + _k1TimesOneMinusB + _k1TimesBOverAvgDocLength * documentLength;
          return numerator / denominator;
        }
        
      }
      
      double scoreOccurrence( double occurrences, int contextSize, double documentOccurrences, int documentLength ) {
        return scoreOccurrence(occurrences, contextSize);
      }
      
      double maximumScore( int minimumDocumentLength, int maximumOccurrences ){
        return scoreOccurrence( maximumOccurrences, minimumDocumentLength );
      }

      double queryTermWeight( double queryK1, double queryB, double _qTF ) {
        if (_okapi)
          return (((_k3 + 1) * _qTF)/(_k3 + _qTF));
        else
          // lemur tfidf:
          //                  _qTF    queryK1  b |D| |D|_avg
          // idf[q] * BM25TF(rawTF,prm.bm25K1, 0, 1,  1)
          return ( _inverseDocumentFrequency * queryK1 * _qTF ) / ( _qTF + queryK1 );

          //          return ( _inverseDocumentFrequency * queryK1 ) / ( 1 + queryK1 * ( (1-queryB) + queryB * (1/_averageDocumentLength) ) );
      }
    };
  }
}

#endif // TFIDF_TERMSCOREFUNCTION_HPP


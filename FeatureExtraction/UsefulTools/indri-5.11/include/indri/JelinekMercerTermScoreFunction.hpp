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
// JelinekMercerTermScoreFunction
//
// 26 January 2004 - tds
//

#ifndef INDRI_JELINEKMERCERTERMSCOREFUNCTION_HPP
#define INDRI_JELINEKMERCERTERMSCOREFUNCTION_HPP

#include <math.h>
namespace indri
{
  /// indri query processing and scoring components.
  namespace query
  {
    
    class JelinekMercerTermScoreFunction : public TermScoreFunction {
    private:
      double _lambda;
      double _backgroundLambda;
      double _collectionFrequency;
      double _collectionComponent;
      double _oneLevelCollectionComponent;
      double _contextLambda;
      double _collectionLambda;
      double _documentLambda;
      double _foregroundLambda;

    public:
      JelinekMercerTermScoreFunction( double collectionFrequency, double collectionLambda, double documentLambda = 0.0 ) {
        _contextLambda = (1 - collectionLambda - documentLambda);
        _collectionFrequency = collectionFrequency;
        _collectionLambda = collectionLambda;
        _documentLambda = documentLambda;
        _foregroundLambda = (1 - _collectionLambda);

        assert( _documentLambda >= 0.0 && _documentLambda <= 1.0 );
        assert( _collectionLambda >= 0.0 && _collectionLambda <= 1.0 );
        assert( _contextLambda >= 0.0 && _contextLambda <= 1.0 );
    
        _collectionComponent = _collectionLambda * _collectionFrequency;
      }

      double scoreOccurrence( double occurrences, int contextSize ) {
        //
        //             [                      occurrences                                             ]
        // score = log [ foregroundLambda * ---------------  + collectionLambda * collectionFrequency ]
        //             [                      contextSize                                             ]
        //

        double contextFrequency = contextSize ? occurrences / double(contextSize) : 0.0;
        return log( _foregroundLambda * contextFrequency + _collectionComponent );
      }

      double scoreOccurrence( double occurrences, int contextSize, double documentOccurrences, int documentLength ) {
        double contextFrequency = contextSize ? occurrences / double(contextSize) : 0.0;
        double documentFrequency = documentLength ? documentOccurrences / double(documentLength) : 0.0;
        return log( _contextLambda * contextFrequency + _documentLambda * documentFrequency + _collectionComponent );
      }
    };
  }
}

#endif // INDRI_JELINEKMERCERTERMSCOREFUNCTION_HPP


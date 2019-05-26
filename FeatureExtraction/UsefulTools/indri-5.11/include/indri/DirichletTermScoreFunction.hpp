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
// DirichletTermScoreFunction
//
// 26 January 2004 - tds
//

#ifndef INDRI_DIRICHLETTERMSCOREFUNCTION_HPP
#define INDRI_DIRICHLETTERMSCOREFUNCTION_HPP

#include <math.h>
namespace indri
{
  /*! Query processing, smoothing, and scoring classes. */
  namespace query
  {
    
    class DirichletTermScoreFunction : public TermScoreFunction {
    private:
      double _mu;
      double _docmu;
      double _collectionFrequency;
      double _muTimesCollectionFrequency;

    public:
      DirichletTermScoreFunction( double mu, double collectionFrequency, double docmu=-1.0 ) {
        _collectionFrequency = collectionFrequency;
        _mu = mu;
        _muTimesCollectionFrequency = _mu * _collectionFrequency;
        _docmu = docmu;
      }

      double scoreOccurrence( double occurrences, int contextSize ) {
        double seen = ( double(occurrences) + _muTimesCollectionFrequency ) / ( double(contextSize) + _mu );
        return log( seen );
      }

      double scoreOccurrence( double occurrences, int contextSize, double documentOccurrences, int documentLength ) {
//two level Dir Smoothing!
//        tf_E + documentMu*P(t|D)
//P(t|E)= ------------------------
//         extentlen + documentMu
//                 mu*P(t|C) + tf_D
//where P(t|D)= ---------------------
//                  doclen + mu
        // if the _docmu parameter is the default, do collection level
        // smoothing only.
        if (_docmu < 0)
          return scoreOccurrence(occurrences, contextSize);
        else {
          double seen = (occurrences+_docmu*(_muTimesCollectionFrequency+documentOccurrences)/(double(documentLength)+_mu))/(double(contextSize)+_docmu);
          return log(seen);
        }
      }
    };
  }
}

#endif // INDRI_DIRICHLETTERMSCOREFUNCTION_HPP

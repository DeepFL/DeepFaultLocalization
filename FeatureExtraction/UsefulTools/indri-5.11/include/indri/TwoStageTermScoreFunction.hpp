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
// TwoStageTermScoreFunction.hpp
//
// 16 April 2004 -- tds
//

#ifndef INDRI_TWOSTAGETERMSCOREFUNCTION_HPP
#define INDRI_TWOSTAGETERMSCOREFUNCTION_HPP
namespace indri
{
  namespace query
  {
    
    class TwoStageTermScoreFunction : public TermScoreFunction {
    private:
      double _mu;
      double _lambda;
      double _collectionFrequency;

    public:
      TwoStageTermScoreFunction( double mu, double lambda, double collectionFrequency ) :
        _mu(mu),
        _lambda(lambda),
        _collectionFrequency(collectionFrequency) {
      }

      double scoreOccurrence( double occurrences, int contextSize ) {

        //                    [  c(w;d) + \mu * p(w|C)   ]
        //    ( 1 - \lambda ) [ ------------------------ ] + \lambda * p(w|C)
        //                    [       |d| + \mu          ]

        double dirichlet = ((double(occurrences) + _mu*_collectionFrequency) / (double(contextSize) + _mu));
        double p = ( 1-_lambda ) * dirichlet + _lambda * _collectionFrequency;
        return log(p);
      }

      double scoreOccurrence( double occurrences, int contextSize, double documentOccurrences, int documentLength ) {
        double documentFrequency = double(documentOccurrences) / double(documentLength);
        double dirichlet = ((double(occurrences) + _mu*documentFrequency) / (double(contextSize) + _mu));
        double p = ( 1-_lambda ) * dirichlet + _lambda * _collectionFrequency;
        return log(p);
      }
    };
  }
}

#endif // INDRI_TWOSTAGETERMSCOREFUNCTION_HPP


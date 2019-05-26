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
// NormalDistribution
//
// 20 July 2004 -- tds
//

#ifndef INDRI_NORMALDISTRIBUTION_HPP
#define INDRI_NORMALDISTRIBUTION_HPP

//
// The normal distribution class is used to generate document priors
// based on some integer attribute of a document.
//
namespace indri
{
  namespace query
  {
    
    class NormalDistribution {
    private:
      double _mu;
      double _sigma;

      double _cdf( double x ) {
        const double a_1 = 0.4361836;
        const double a_2 = -0.1201676;
        const double a_3 = 0.9372980;
        const double p = 0.33267;
        const double pi = 3.1415926535;

        double t = 1./(1.+p*x);
        double zx = ( 1. / sqrt(2*pi*_sigma) ) * exp( pow( -((x-_mu)/_sigma), 2 ) );

        double cdf = 1 - zx * ( a_1*t + a_2*pow(t,2) + a_3*pow(t,3) );
        return cdf;
      }

    public:
      NormalDistribution( double mu, double sigma ) {
        _mu = mu;
        _sigma = sigma;
      }

      // The value returned here corresponds to the probability mass
      // on the Gaussian curve between value-0.5 and value+0.5

      double operator () ( INT64 value ) {
        // compute lower bound cdf
        double low = _cdf( value-0.5 );
        double high = _cdf( value+0.5 );
        return high - low;
      }
    };
  }
}

#endif // INDRI_NORMALDISTRIBUTION_HPP


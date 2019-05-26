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
// TermScoreFunction
//
// 23 January 2004 -- tds
//

#ifndef INDRI_TERMSCOREFUNCTION_HPP
#define INDRI_TERMSCOREFUNCTION_HPP
namespace indri
{
  namespace query
  {
    
    /*! Abstract base class for all term scoring and smoothing functions. 
      See <a href="IndriParameters.html#rule">the <tt>rule</tt> parameter
      format</a> for a description of the rule parameter format. @see
      TermScoreFunctionFactory for a description of how to add a new scoring
      function.
    */
    class TermScoreFunction {
    public:
      virtual double scoreOccurrence( double occurrences, int contextLength ) = 0;
      virtual double scoreOccurrence( double occurrences, int contextLength, double documentOccurrences, int documentLength ) = 0;
    };
  }
}

#endif // INDRI_TERMSCOREFUNCTION_HPP


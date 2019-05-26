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
// QueryResponsePacker
//
// 23 March 2004 -- tds
//

#ifndef INDRI_QUERYRESPONSEPACKER_HPP
#define INDRI_QUERYRESPONSEPACKER_HPP

#include "indri/InferenceNetwork.hpp"
#include "lemur/lemur-compat.hpp"
namespace indri
{
  namespace net
  {
    
    class QueryResponsePacker {
    private:
      indri::infnet::InferenceNetwork::MAllResults& _results;

    public:
      QueryResponsePacker( indri::infnet::InferenceNetwork::MAllResults& results ) :
        _results(results)
      {
      }

      void write( NetworkMessageStream* stream ) {
        indri::infnet::InferenceNetwork::MAllResults::iterator iter;
        indri::infnet::EvaluatorNode::MResults::iterator nodeIter;

        for( iter = _results.begin(); iter != _results.end(); iter++ ) {
          const std::string& nodeName = iter->first;

          for( nodeIter = iter->second.begin(); nodeIter != iter->second.end(); nodeIter++ ) {
            const std::string& listName = nodeIter->first;
            std::string resultName = nodeName + ":" + listName;
            const std::vector<indri::api::ScoredExtentResult>& resultList = nodeIter->second;

            // send each chunk of 100 results in a separate chunk
            // const char resultSize = 20;
            const char resultSize=(sizeof(INT32)*5 + sizeof(double) + sizeof(INT64));
            char networkResults[resultSize * 100];
            size_t resultsSent = 0;

            while( resultList.size() > resultsSent ) {
              size_t sendChunk = lemur_compat::min<size_t>( resultList.size() - resultsSent, (size_t) 100 );

              for( size_t i=0; i<sendChunk; i++ ) {
                indri::api::ScoredExtentResult byteSwapped;
                const indri::api::ScoredExtentResult& unswapped = resultList[i + resultsSent];

                byteSwapped.begin = htonl(unswapped.begin);
                byteSwapped.end = htonl(unswapped.end);
                byteSwapped.document = htonl(unswapped.document );
                byteSwapped.score = lemur_compat::htond(unswapped.score);
#ifndef __APPLE__
                byteSwapped.number = lemur_compat::htonll(unswapped.number);
#else
                byteSwapped.number = htonll(unswapped.number);
#endif
                byteSwapped.ordinal = htonl(unswapped.ordinal);
                byteSwapped.parentOrdinal = htonl(unswapped.parentOrdinal);

                memcpy( networkResults + i*resultSize, &byteSwapped.score, sizeof(double) );
                memcpy( networkResults + i*resultSize + 8, &byteSwapped.document, sizeof(INT32) );
                memcpy( networkResults + i*resultSize + 12, &byteSwapped.begin, sizeof(INT32) );
                memcpy( networkResults + i*resultSize + 16, &byteSwapped.end, sizeof(INT32) );
                memcpy( networkResults + i*resultSize + 20, &byteSwapped.number, sizeof(INT64) );
                memcpy( networkResults + i*resultSize + 28, &byteSwapped.ordinal, sizeof(INT32) );
                memcpy( networkResults + i*resultSize + 32, &byteSwapped.parentOrdinal, sizeof(INT32) );
              }

              stream->reply( resultName, networkResults, int(sendChunk * resultSize) );
              resultsSent += sendChunk;
            }
          }
        }
    
        stream->replyDone();
      }
    };
  }
}

#endif // INDRI_QUERYRESPONSEPACKER_HPP


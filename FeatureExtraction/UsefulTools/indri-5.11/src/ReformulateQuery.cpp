/*==========================================================================
 * Copyright (c) 2012 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */
#include <iostream>
#include <sstream>
#include "indri/Parameters.hpp"
#include "indri/ReformulateQuery.hpp"

std::string indri::query::ReformulateQuery::downcase_string( const std::string& str ) {
  std::string result;
  result.resize( str.size() );
  for( size_t i=0; i<str.size(); i++ ) {
    result[i] = tolower(str[i]);
  }
  return result;
}

std::vector<std::string> indri::query::ReformulateQuery::split(std::string q,
                                                               char d) {
  std::vector<std::string> strs;
  std::string token;
  std::istringstream qstream(q);
  // tokenize
  // yes, there are better ways to do this...
  while ( getline(qstream, token, d) ) {
    if (token.size() > 0) 
      strs.push_back(token);
  }
  return strs;
}
std::string indri::query::ReformulateQuery::replaceAll(std::string result, const std::string& replaceWhat, const std::string& replaceWithWhat) {
  while(true) {
    const int pos = result.find(replaceWhat);
    if (pos == std::string::npos)
      break;
    result.replace(pos,replaceWhat.size(),replaceWithWhat);
  }
  return result;
}

std::string indri::query::ReformulateQuery::trim(std::string text) {
  char *s = new char[text.size()+1];
  strcpy(s, text.c_str());
  int start=0;
  while(start < text.size()) {
    if(s[start] == ' ')
      start++;
    else
      break;
  }
  int end = text.size()-1;
  while(end >= 0) {
    if(s[end] == ' ')
      end--;
    else
      break;
  }
  if(start < end) {
    s[end+1] = '\0';
    text.assign(s+start);
  }
  delete(s);
  text = indri::query::ReformulateQuery::replaceAll(text, "  ", " ");
  return text;
}

std::string indri::query::ReformulateQuery::makeIndriFriendly(std::string query) {
  static char forbidden[] = "`~!@#$%^&*()-_=+[]{}\\|;:'\",.<>/?e";
  char* s = new char[query.length()+1];
  strcpy(s, query.c_str());
  char* c = new char[query.length()+1];
  int i=0;
  int j=0;
  while(i < strlen(s)) {
    bool match = false;
    int k=0;
    while(forbidden[k] != 'e' && match==false) {
      if(s[i] == forbidden[k])
        match = true;
      else
        k++;
    }
    if(match == false)
      c[j++] = s[i];
    else
      c[j++] = ' ';
    i++;
  }
  c[j] = '\0';
  string res(c);
  delete[](s);
  delete[](c);
  return res;

}
std::string indri::query::ReformulateQuery::generateSDMQuery(std::vector<std::string> strs,  std::vector<indri::query::ReformulateQuery::weighted_field> fields) {
  if (fields.size() == 0) return generateSDMQuery(strs);
  if(strs.size() == 1)
    return "#combine(" + strs[0] + ")";
  // SDM weights wT, wO, wU.
  std::string wT = params->get("weightT", "0.85" );
  std::string wO = params->get("weightO", "0.1");
  std::string wU = params->get("weightU", "0.05");

  std::string queryT = "";
  std::string queryO = "";
  std::string queryU = "";
  for (int i = 0; i < strs.size(); i++) {
    queryT += "#wsum( ";
    for (int j = 0; j < fields.size(); j++) {
      queryT += fields[j].weight +  " " + strs[i] + ".(" + fields[j].field + ") "; 
    }
    queryT +=" )\n ";
  }
  
  for (int i = 0; i < strs.size()-1; i++) {
    std::string ow = "#1(" + strs[i] + " " + strs[i+1] + ")";
    std::string uw = "#uw8(" + strs[i] + " " + strs[i+1] + ")";
    queryO += "#wsum( ";
    queryU += "#wsum( ";
    for (int j = 0; j < fields.size(); j++) {
      queryO += fields[j].weight +  " " + ow + ".(" + fields[j].field + ") "; 
      queryU += fields[j].weight +  " " + uw + ".(" + fields[j].field + ") ";
    }
    queryO += " )\n";
    queryU += " )\n";
  }
  return "#weight( " + wT + " #combine(" + queryT + 
    ") " + wO + " #combine(" + queryO + 
    ") " + wU + " #combine(" + queryU + "))";
}
std::string indri::query::ReformulateQuery::generateSDMQuery(std::string q) {
  std::vector<std::string> strs = split(q, ' ');
  return generateSDMQuery(strs);
}

std::string indri::query::ReformulateQuery::generateSDMQuery(std::vector<std::string> strs) {
  if(strs.size() == 1)
    return "#combine(" + strs[0] + ")";
  // SDM weights wT, wO, wU.
  std::string wT = params->get("weightT", "0.85" );
  std::string wO = params->get("weightO", "0.1");
  std::string wU = params->get("weightU", "0.05");

  std::string t = "";
  std::string ow = "";
  std::string uw = "";
  for(int i = 0; i < strs.size()-1; i++) {
    t += strs[i] + " ";
    ow += "#1(" + strs[i] + " " + strs[i+1] + ") ";
    uw += "#uw8(" + strs[i] + " " + strs[i+1] + ") ";
  }
  t += strs[strs.size()-1];
  return "#weight( " + wT + " #combine(" + t + 
    ") " + wO + " #combine(" + ow + 
    ") " + wU + " #combine(" + uw + "))";
}
std::string indri::query::ReformulateQuery::generateCMUFDMQuery(std::string q){
  std::vector<std::string> strs = split(q, ' ');
  return generateCMUFDMQuery(strs);
}

std::string indri::query::ReformulateQuery::generateCMUFDMQuery(std::vector<std::string> tokens) {
  // parameters
  // mixture weights
  std::string w_mixture        = params->get("weightMixture", "1.0");
  std::string w_dependency     = params->get("weightDependency", "1.0");
  // field weights
  std::string w_mixture_url    = params->get("weightURL", "1.0");
  std::string w_mixture_title  = params->get("weightTitle", "1.0");
  std::string w_mixture_body   = params->get("weightBody", "1.0");
  std::string w_mixture_meta   = params->get("weightMeta", "1.0");
  std::string w_mixture_alt    = params->get("weightAlt", "1.0");
  std::string w_mixture_inlink = params->get("weightInlink", "1.0");
  
  std::string mixtureQuery;
  std::string fdmQuery;
  std::string resultQuery;
  
  // Begin mixture model
  mixtureQuery = " #combine ( ";
  for (int i = 0; i < tokens.size(); i++) {
    mixtureQuery += "#wsum (" + 
      w_mixture_url    + " " + tokens[i] + ".(url) "    +
      w_mixture_title  + " " + tokens[i] + ".(title) "  +
      w_mixture_body   + " " + tokens[i] + ".(body) "   +
      w_mixture_meta   + " " + tokens[i] + ".(meta) "   +
      w_mixture_alt    + " " + tokens[i] + ".(alt) "    +
      w_mixture_inlink + " " + tokens[i] + ".(inlink)" + ") ";
  }
  mixtureQuery += " ) "; 
  // End Mixture Model

  // Begin Dependency Model
  if (tokens.size() < 2 ) {
    std::string q = tokens[0];
    // pathological case, should just be the term.
    // SDM weights wT, wO, w.U  
    std::string wT = params->get("weightT", "0.85" );
    std::string wO = params->get("weightO", "0.1");
    std::string wU = params->get("weightU", "0.05");
    fdmQuery += " #weight  ( " + wT + " " + q + " ";
    fdmQuery += wO + " #1(" + q + ") ";
    fdmQuery += wU + " #uw4(" + q + ") ) ";
  } else {
    fdmQuery = generateFDMQuery(tokens);
  }
  resultQuery = "#weight( " + w_mixture + mixtureQuery 
    + w_dependency + " " + fdmQuery + " )"; 
  return resultQuery;
}


std::string indri::query::ReformulateQuery::generateFDMQuery(std::vector<std::string> strs,  std::vector<indri::query::ReformulateQuery::weighted_field> fields) {
  if (fields.size() == 0) return generateFDMQuery(strs);
  if(strs.size() == 1)
    return "#combine(" + strs[0] + ")";
  // SDM weights wT, wO, wU.
  std::string wT = params->get("weightT", "0.85" );
  std::string wO = params->get("weightO", "0.1");
  std::string wU = params->get("weightU", "0.05");

  std::string queryT = "";
  std::string queryO = "";
  std::string queryU = "";
  for (int i = 0; i < strs.size(); i++) {
    queryT += "#wsum( ";
    for (int j = 0; j < fields.size(); j++) {
      queryT += fields[j].weight +  " " + strs[i] + ".(" + fields[j].field + ") "; 
    }
    queryT +=" )\n ";
  }

  for (int start = 0; start < strs.size()-1; start++) {
    for (int end = start + 1; end < strs.size(); end++) {
      std::string t = "";
      int winSize = 4 * (end - start + 1);
      std::stringstream ss;
      ss << winSize;
      std::string win = ss.str();
      for (int i = start; i <= end; i++) {
        t += strs[i] + " ";
      }
      std::string ow = "#1(" + t+ ")";
      std::string uw = "#uw" + win +"(" + t + ")";
      queryO += "#wsum( ";
      queryU += "#wsum( ";
      for (int j = 0; j < fields.size(); j++) {
        queryO += fields[j].weight +  " " + ow + ".(" + fields[j].field + ") "; 
        queryU += fields[j].weight +  " " + uw + ".(" + fields[j].field + ") ";
      }
      queryO += " )\n";
      queryU += " )\n";
    }
  }
  return "#weight( " + wT + " #combine(" + queryT + 
    ") " + wO + " #combine(" + queryO + 
    ") " + wU + " #combine(" + queryU + "))";
}

std::string indri::query::ReformulateQuery::generateFDMQuery(std::string q) {
  std::vector<std::string> strs = split(q, ' ');
  return generateFDMQuery(strs);
}

std::string indri::query::ReformulateQuery::generateFDMQuery(std::vector<std::string> strs) {
  if(strs.size() == 1)
    return "#combine(" + strs[0] + ")";
  // SDM weights wT, wO, w.U  
  std::string queryT = "";
  std::string queryO = "";
  std::string queryU = "";
  std::string wT = params->get("weightT", "0.85" );
  std::string wO = params->get("weightO", "0.1");
  std::string wU = params->get("weightU", "0.05");

  for (int start = 0; start < strs.size(); start++) {
    queryT += strs[start] + " ";
  }
  for (int start = 0; start < strs.size()-1; start++) {
    for (int end = start + 1; end < strs.size(); end++) {
      std::string t = "";
      int winSize = 4 * (end - start + 1);
      std::stringstream ss;
      ss << winSize;
      std::string win = ss.str();
      for (int i = start; i <= end; i++) {
        t += strs[i] + " ";
      }
      std::string ow = "#1(" + t + ")\n";
      std::string uw = "#uw" + win +"(" + t + ")\n";
      queryO += ow;
      queryU += uw;
    }
  }
  return "#weight( " + wT + " #combine(" + queryT + 
    ") " + wO + " #combine(" + queryO + 
    ") " + wU + " #combine(" + queryU + "))";
}


std::string indri::query::ReformulateQuery::generateCombineQuery(std::vector<std::string> strs) {
  std::string result = "#combine( ";
  for (int i = 0; i < strs.size(); i++) {
    result += strs[i] + " ";
  }
  result += ")";
  return result;
}

std::string indri::query::ReformulateQuery::generateCombineQuery(std::string q) {
  return generateCombineQuery(split(q, ' '));
}
std::string indri::query::ReformulateQuery::transform(std::string queryText) {
  std::string reform;
  // other potential xforms/parameters here.
  bool stopStructures = params->get("stopStructures", true);
  bool liteStop = params->get("liteStop", true);
  bool genSDM = params->get("genSDM", false);
  bool genFDM = params->get("genFDM", false);
  bool genCMUFDM = params->get("genCMUFDM", false);
  // case normalize
  reform = downcase_string(queryText);
  // remove the stop structures [huston]
  if (stopStructures)
    reform = indri::query::StopStructureRemover::transform(reform);
  // lite stopping [bendersky]
  if (liteStop)
    reform = indri::query::QueryStopper::transform(reform);
  if (reform.size() == 0) return std::string("EMPTYQUERY");
  
  std::vector<std::string> queryTerms = split(reform, ' ');
  std::vector<indri::query::ReformulateQuery::weighted_field> fields;
  if (params->exists("queryField")) {
    indri::api::Parameters queryFields = (*params)[ "queryField" ];
    for( size_t i = 0; i < queryFields.size(); i++ ) {
      std::string name;
      std::string weight;
      name = (std::string) queryFields[i]["name"];
      weight = (std::string) queryFields[i]["weight"];
      fields.push_back(weighted_field(name, weight));
    }
  }
  // SDM or FDM with named page style weights for threadtitle
  if (genSDM)
    reform = generateSDMQuery(queryTerms, fields);
  else if (genFDM)
    reform = generateFDMQuery(queryTerms, fields);
  else if (genCMUFDM)
    reform = generateCMUFDMQuery(queryTerms);
  else 
    reform = generateCombineQuery(queryTerms);
  // additional reformulation steps...
  return reform;
}

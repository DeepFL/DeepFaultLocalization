#include "SingleResultItem.h"

SingleResultItem::SingleResultItem(string templateString) {
  thisTemplate=templateString;
}

SingleResultItem::~SingleResultItem() {

}

void SingleResultItem::setVariable(string variableName, string value) {
  replaceAll(&value, "/%7E", "/~");
  variables.put(variableName.c_str(), value.c_str());
}

void SingleResultItem::replaceAll(string *s, string variable, string value) {
  size_t currentPos=s->find(variable);
  while (currentPos!=std::string::npos) {
    s->replace(currentPos, variable.length(), value);
    currentPos=s->find(variable);
  }
}

void SingleResultItem::findAndReplace(string *s, string varName, string templateVariable) {
  char *value=variables.get(varName.c_str());
  if (value) {
    replaceAll(s, templateVariable, value);
  }
}

string SingleResultItem::toString() {
  string outputString=thisTemplate;

  // ResCachedURL
  string cachedURL="{$appPath}?d={$datasource}&i={$resID}";
  findAndReplace(&cachedURL, "scriptname", "{$appPath}");
  findAndReplace(&cachedURL, "datasource", "{$datasource}");
  findAndReplace(&cachedURL, "id", "{$resID}");
  replaceAll(&outputString, "{%ResCachedURL%}", cachedURL);

  // ResURL
  // if the URL is "" or missing, use the cached URL
  string URLStringToUse=cachedURL;
  if (variables.get("URL")) {
    URLStringToUse=variables.get("URL");

   if (URLStringToUse=="") {
      // see if we have an original URL set at least...
      URLStringToUse=variables.get("origURL");
      if (URLStringToUse=="") {
        // still nothing? default to the cache.
        URLStringToUse=cachedURL;
      }
    } else {
      // insert anything from the root add path... (only if not cached)
      URLStringToUse.insert(0, CGIConfiguration::getInstance().getRootAddPath());
      // ensure http:// is not included if it already exists...
      if ((URLStringToUse.find("http://")!=0) && (URLStringToUse.find("HTTP://")!=0)) {
        URLStringToUse="http://" + URLStringToUse;
      }
    }
  }
  // check the URL - change any %7E's to ~'s - 
  replaceAll(&URLStringToUse, "/%7E", "/~");
  replaceAll(&outputString, "{%ResURL%}", URLStringToUse);

  // ResTitle
  string thisTitle="(no title)";
  if (variables.get("title")) {
    thisTitle=variables.get("title");
    replaceAll(&thisTitle, "<", "&lt;");
    replaceAll(&thisTitle, ">", "&gt;");
  }
  replaceAll(&outputString, "{%ResTitle%}", thisTitle);

  // ResSummary
  findAndReplace(&outputString, "summary", "{%ResSummary%}");

  // ResScore
  findAndReplace(&outputString, "score", "{%ResScore%}");

  // ResOrigUrl
  findAndReplace(&outputString, "origURL", "{%ResOrigURL%}");

  return outputString;
}


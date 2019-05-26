#include "CGIConfiguration.h"

#include <sstream>

using namespace indri::xml;

CGIConfiguration *CGIConfiguration::_instance=0;

CGIConfiguration::CGIConfiguration() {
  templatePath=DEFAULT_TEMPLATE_PATH;
  currentRootPathSize=0;
  currentIndicesSize=0;
  rootAddPath="";
  stripRootPath=false;
}

CGIConfiguration::~CGIConfiguration() {
  vector<db_t*>::iterator vIter=indices.begin();
  while (vIter!=indices.end()) {
    if (*vIter) delete (*vIter);    
    vIter++;
  }
}

bool CGIConfiguration::loadConfiguration(string configPath) {
  readConfigFile((char*)configPath.c_str());
  return true;
}

bool CGIConfiguration::readConfigFile(char *filePath) {

  std::stringstream xmlFile;

  string line;
  std::ifstream infile (filePath);
  if (!infile.is_open()) return false;
  while (!infile.eof()) {
    getline (infile,line);
    xmlFile << line << std::endl;
  }
  infile.close();

  XMLReader xReader;
  XMLNode *rootNode=xReader.read(xmlFile.str());

  // root node must be <lemurconfig>
  if (rootNode->getName()!="lemurconfig") return false;
  const XMLNode *templateNode=rootNode->getChild("templatepath");
  if (templateNode) {
    templatePath=templateNode->getValue();
  }

  const XMLNode *rootPathList=rootNode->getChild("rootpaths");
  if (rootPathList) {
    string stripPathRoot=rootPathList->getAttribute("strippath");
    if (stripPathRoot=="true") {
      stripRootPath=true;
    }

    std::vector<XMLNode*> rootPathVec=rootPathList->getChildren();
    for (std::vector<XMLNode*>::iterator vIter=rootPathVec.begin(); vIter!=rootPathVec.end(); vIter++) {
      if ((*vIter)->getName()=="path") {
        string thisPath=(*vIter)->getValue();
        rootPaths.push_back(thisPath);
      }
    }
  }

  currentRootPathSize=(int)rootPaths.size();

  const XMLNode *rootAddPathNode=rootNode->getChild("addtorootpath");
  if (rootAddPathNode) {
    rootAddPath=rootAddPathNode->getValue();
  }

  const XMLNode *queryTimeoutValue=rootNode->getChild("querytimeout");
  if (queryTimeoutValue) {
    queryTimeout=string_to_int(queryTimeoutValue->getValue());
  }

  _useQueryLog=false;
  const XMLNode *useQueryLogNode=rootNode->getChild("querylog");
  if (useQueryLogNode) {
    _useQueryLog=true;
    queryLogPath=useQueryLogNode->getValue();
  }

  supportPageRank=false;
  const XMLNode *useSupportPRNode=rootNode->getChild("supportpagerank");
  if ((useSupportPRNode) && (useSupportPRNode->getValue()=="true")) {
    supportPageRank=true;      
  }  

  _useDuplicateResultAttribute=false;
  const XMLNode *duplicateResultAttributeNode=rootNode->getChild("duplicateresultattribute");
  if (duplicateResultAttributeNode) {
    duplicateResultAttribute=duplicateResultAttributeNode->getValue();
    _useDuplicateResultAttribute=true;
  }



  const XMLNode *indexesStart=rootNode->getChild("indexes");
  if (indexesStart) {
    std::vector<XMLNode*> indexes=indexesStart->getChildren();
    for (std::vector<XMLNode*>::iterator vIter=indexes.begin(); vIter!=indexes.end(); vIter++) {
      if ((*vIter)->getName()=="index") {
        const XMLNode *indexPathNode=(*vIter)->getChild("path");
        const XMLNode *descriptionNode=(*vIter)->getChild("description");
        const XMLNode *queryHostNode=(*vIter)->getChild("queryserver");
        if (indexPathNode) {
          db_t *newIndex=new db_t;
          newIndex->path=indexPathNode->getValue();
          if (descriptionNode) newIndex->name=descriptionNode->getValue();
          if (queryHostNode) { 
            // newIndex->queryHost=queryHostNode->getValue();
            std::stringstream ss(std::string(queryHostNode->getValue()));
            std::string item;
            while(std::getline(ss, item, ';')) {
              newIndex->queryHostVec.push_back(item);
            }
          } else {
            // newIndex->queryHost="";
          }
          indices.push_back(newIndex);
        }
      }
    }
  }

  currentIndicesSize=(int)indices.size();

  return true;
}

string CGIConfiguration::getTemplatePath() {
  return templatePath;
}


bool CGIConfiguration::getStripRootPathFlag() {
  return stripRootPath;
}

int CGIConfiguration::getNumRootPaths() {
  return currentRootPathSize;
}

string CGIConfiguration::getRootPath(int whichPath) {
  if ((whichPath > -1) && (whichPath < currentRootPathSize)) {
    return rootPaths[whichPath];
  }
  return string("");
}

string CGIConfiguration::getRootAddPath() {
  return rootAddPath;
}

int CGIConfiguration::getQueryTimeout() {
  return queryTimeout;
}

int CGIConfiguration::getNumIndices() {
  return currentIndicesSize;
}

string CGIConfiguration::getIndexPath(int whichIndex) {
  if ((whichIndex > -1) && (whichIndex < currentIndicesSize)) {
    return indices[whichIndex]->path;
  }
  return string("");
}

vector<string> CGIConfiguration::getQueryHostVec(const string &indexPath) {
  vector<string> retVec;

  for (int i=0; i < currentIndicesSize; i++) {
    if (indices[i]->path==indexPath) {
      return indices[i]->queryHostVec;
    }
  }

  return retVec;
}

string CGIConfiguration::getIndexDescription(int whichIndex) {
  if ((whichIndex > -1) && (whichIndex < currentIndicesSize)) {
    return indices[whichIndex]->name;
  }
  return string("");
}

void CGIConfiguration::putKVItem(string _key, string _value) {
  _kvPairs.put(_key.c_str(), _value.c_str());
}

string CGIConfiguration::getKVItem(string _key) {
  string retString="";
  char *retVal=_kvPairs.get(_key.c_str());
  if (retVal) {
    retString=retVal;
  }
  return retString;
}


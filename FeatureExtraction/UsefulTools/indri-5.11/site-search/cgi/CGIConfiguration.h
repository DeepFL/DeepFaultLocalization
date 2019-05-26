#ifndef _CGICONFIGURATION_H
#define _CGICONFIGURATION_H

#include <iostream>
#include <fstream>
#include <string>
#include <string>
#include <vector>
using std::string;
using std::vector;

#include "indri/XMLReader.hpp"
#include "DictionaryHash.h"
#include "LemurSearchCGIConstants.h"

/**
 * A singleton class that loads and holds the
 * configuration for an instance of the LemurCGI.<br />
 * &nbsp;<br />
 * A configuration file is essentially an XML file that defines the various
 * parameters for the CGI to run. The configuration file must be in the
 * directory as defined in LEMURCGI_CONFIG_PATH (in LemurSearchCGIConstants.h)
 * in order to be found - the default is the same directory as the executable.<br />
 * &nbsp;<br />
 * A sample configuration file looks like:<br />
 * <pre>
 * &lt;lemurconfig&gt;
 * &nbsp;&lt;templatepath&gt;./templates/&lt;/templatepath&gt;
 * &nbsp;&lt;rootpaths&nbsp;strippath=&quot;true&quot;&gt;
 * &nbsp;&nbsp;&lt;path&gt;/var/data/mirrored_data/&lt;/path&gt;
 * &nbsp;&lt;/rootpaths&gt;
 * &nbsp;&lt;addtorootpath&gt;path_to_add&lt;/addtorootpath&gt;
 * &nbsp;&lt;indexes&gt;
 * &nbsp;&nbsp;&lt;index&gt;
 * &nbsp;&nbsp;&nbsp;&lt;path&gt;/var/indices/testindex&lt;/path&gt;
 * &nbsp;&nbsp;&nbsp;&lt;description&gt;Test index description&lt;/description&gt;
 * &nbsp;&nbsp;&lt;/index&gt;
 * &nbsp;&lt;/indexes&gt;
 * &lt;/lemurconfig&gt;
 * </pre>
 *
 * @author Mark J. Hoy [http://www.cs.cmu.edu/~mhoy/]
 * @version 4/13/06
 *
 */
class CGIConfiguration {
protected:
  /** if we should strip the root paths or not */
  bool            stripRootPath;

  /** current number of root paths in the vector */
  int             currentRootPathSize;

  /** current number of indexes */
  int             currentIndicesSize;

  /** how long to wait for query response */
  int queryTimeout;

  bool  _useQueryLog;

  string queryLogPath;

  /** variables for using duplicate results attribute */
  string duplicateResultAttribute;
  bool	_useDuplicateResultAttribute;
  /** our root paths as defined by the configuration */
  vector<string>  rootPaths;

  /** a string (if any) to add to the front of standard URLs */
  string rootAddPath;

  /** our indices */
  vector<db_t*>   indices;

  /** the path to the template files */
  string          templatePath;

  /** dictionary hash for any values we may want to temporarily store **/
  DictionaryHash  _kvPairs;

  vector<string> _queryHostVector;

  /** support page rank as a prior **/
  bool supportPageRank;

  /** our instance variable */
  static CGIConfiguration *_instance;

  /**
   * Reads the configuration file
   * @param filePath the path to the configuration file
   * @return true on success, false on failure
   */
  bool readConfigFile(char *filePath);

  /**
   * Our protected constructor
   */
  CGIConfiguration();

public:

  /**
   * A static function to access the static instance
   * of this object for our singleton.
   *
   * @return reference to our object.
   */
  static CGIConfiguration &getInstance() {
    if (!_instance) {
      _instance=new CGIConfiguration();
    }
    return (*_instance);
  }

  /**
   * Destructor
   */
  ~CGIConfiguration();

  /**
   * Loads the configuration file.
   * @param configPath the full path and filename of the configuration file
   * @return true on success
   */
  bool loadConfiguration(string configPath);

  /**
   * Retireves the path to the templates.
   * The template path is defined as part of the configuration file.
   *
   * @return the path to the templates
   */
  string getTemplatePath();

  /**
   * asks the age old question of to whether or not to strip the root path from a URL
   *
   * @return true if we should strip the root path
   */
  bool getStripRootPathFlag();

  /**
   * Returns the number of possible root paths
   *
   * @return the number of root paths
   */
  int    getNumRootPaths();

  /**
   * Returns the query timeout (how long to wait for query processing)
   *
   * @return the query timeout
   */
  int getQueryTimeout();

  bool getSupportPageRankPrior() {
    return supportPageRank;
  }

  /**
   * Retrieves a single root path
   *
   * @param whichPath which one of the root paths to return
   * @return the root path (or an empty string if there is none)
   */
  string getRootPath(int whichPath=0);

  /**
   * Retrieves the rootAddPath (if any)
   *
   * @return the rootAddPath value
   */
  string getRootAddPath();

  /**
   * Retrives the number of indexes configured
   *
   * @return the number of indexes
   */
  int    getNumIndices();

  /**
   * Retrieves a path to an index
   *
   * @param whichIndex the index number (base 0) to return
   * @return the path to the index (or an empty string if none)
   */
  string getIndexPath(int whichIndex);

  // string getQueryHost(const string &indexPath);
  vector<string> getQueryHostVec(const string &indexPath);

  /**
   * Retrieves a description tag for an index
   *
   * @param whichIndex the index number (base 0) to return
   * @return the descriptions of the index (or an empty string if none)
   */
  string getIndexDescription(int whichIndex);

  /**
   * Sets a generic key/value pair
   *
   * @param _key the key of the kv pair
   * @param _value the value of the kv pair
   */
  void putKVItem(string _key, string _value);

  /**
   * Retrieves a value from a key/value pair
   *
   * @param _key the key of the kv pair
   * @return the value of the key
   */
  string getKVItem(string _key);

  bool useQueryLogging() {
    return _useQueryLog;
  }

  string getQueryLogPath() {
    return queryLogPath;
  }

  string getDuplicateResultAttribute() {
    return duplicateResultAttribute;
  }
  bool useDuplicateResultAttribute() {
    return _useDuplicateResultAttribute;
  }

}; // end class CGIConfiguration

#endif // _CGICONFIGURATION_H


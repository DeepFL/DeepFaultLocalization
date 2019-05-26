#ifndef _SINGLERESULTITEM_H
#define _SINGLERESULTITEM_H

#include <string>
using std::string;

#include "LemurSearchCGIConstants.h"
#include "CGIConfiguration.h"
#include "DictionaryHash.h"

/**
 * A class that represents a single result item for the results
 *
 * @author Mark J. Hoy [http://www.cs.cmu.edu/~mhoy/]
 * @version 2/28/06
 */
class SingleResultItem {
private:
  /** the preloaded template for this result item */
  string thisTemplate;

  /** A hashtable containing our various variables */
  DictionaryHash variables;

  /**
   * Replaces all the occurances of a string with another string.
   *
   * @param s our input string
   * @param variable the string to look for and replace
   * @param value the replacement value
   */
  void replaceAll(string *s, string variable, string value);

  /**
   * Replaces a variable from the template with a variable from the dictionary hash.
   *
   * @param s our input string, typically the tempalte
   * @param varName the name of the variable to replace
   * @param templateVariable the variable as defined within the template
   */
  void findAndReplace(string *s, string varName, string templateVariable);

public:
  /**
   * Constructor.
   *
   * @param templateString the preloaded template
   */
  SingleResultItem(string templateString);

  /**
   * Destructor.
   */
  ~SingleResultItem();
  
  /**
   * Sets a variable for this result item.
   *
   * @param variableName the name of the variable
   * @param value the value for the variable
   */
  void setVariable(string variableName, string value);

  /**
   * Returns the template with the filled-in variable values
   *
   * @return the filled-in template
   */
  string toString();

}; // class SingleResultItem

#endif // _SINGLERESULTITEM_H


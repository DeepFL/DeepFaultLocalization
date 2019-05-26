#ifndef _DICTIONARYHASH_H
#define _DICTIONARYHASH_H

#include <string.h>

#ifndef NULL
#define NULL 0L
#endif

/** the default starting hash table array size */
#define DICTIONARY_HASH_ARRAY_SIZE 125

/**
 * A basic key/value container based on a hashtable with less overhead than std::map
 *
 * @author Mark J. Hoy [http://www.cs.cmu.edu/~mhoy/]
 * @version 2/28/06
 */
class DictionaryHash {
private:
  struct dictionaryHashValue_t {
    char *key;
    char *value;
    dictionaryHashValue_t *next;
    
    dictionaryHashValue_t(char *cKey, char *cValue) {
      key=NULL;
      value=NULL;
      if (cKey) {
        key=strdup(cKey);
      }
      if (cValue) {
        value=strdup(cValue);
      }

      next=NULL;
    }

    ~dictionaryHashValue_t() {
      if (key) delete key;
      if (value) delete value;
    }
  };

  dictionaryHashValue_t **hashTable;

  int hash(char* input);
  int currentNumValues;

public:
  /**
   * Constructor
   */
  DictionaryHash();

  /**
   * Destructor
   */
  ~DictionaryHash();

  /**
   * Returns the current number of elements in the collection
   *
   * @return the number of elements in the collection
   */
  int size();

  /**
   * Clears out the entire hashmap dictionary
   */
  void clear();

  /**
   * Places a new key/value pair into the dictionary hashtable. If the key already exists,
   * the value is overwritten with the new one.
   *
   * @param key the key to put in
   * @param value the value to put in
   */
  void put(char *key, char *value);

  /**
   * Helper function to put from a constant char* key and value.
   *
   * @param key the key to put in
   * @param value the value to put in
   */
  void put(const char *key, const char *value) {
    put((char*)key, (char*)value);
  }

  /**
   * Retrieves a value from the hashtable
   *
   * @param key the key to get the value for
   * @return the value (or NULL if not found)
   */
  char *get(char *key);

  /**
   * Helper function for get from a constant char * key
   *
   * @param key the key to get the value for
   * @return the value (or NULL if not found)
   */
  char *get(const char *key) {
    return get((char*)key);
  }

}; // class DictionaryHash

#endif  // _DICTIONARYHASH_H


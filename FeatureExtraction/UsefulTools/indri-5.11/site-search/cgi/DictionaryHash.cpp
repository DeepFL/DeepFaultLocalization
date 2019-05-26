#include "DictionaryHash.h"

DictionaryHash::DictionaryHash() {
  currentNumValues=0;
  hashTable=new dictionaryHashValue_t*[DICTIONARY_HASH_ARRAY_SIZE];
  for (int i=0; i < DICTIONARY_HASH_ARRAY_SIZE; i++) {
    hashTable[i]=NULL;
  }
}

DictionaryHash::~DictionaryHash() {
  clear();
}

int DictionaryHash::hash(char* input) {
  if (!input) return 0;
  
  unsigned long cValue=0;
  char *c=input;
  while (*c) {
    cValue+=(*c);
    c++;
  }
  return (int)(cValue % DICTIONARY_HASH_ARRAY_SIZE);
}

int DictionaryHash::size() {
  return currentNumValues;
}

void DictionaryHash::clear() {
  for (int i=0; i < DICTIONARY_HASH_ARRAY_SIZE; i++) {
    dictionaryHashValue_t *cItem=hashTable[i];
    while (cItem) {
      dictionaryHashValue_t *nextItem=cItem->next;
      delete cItem;
      cItem=nextItem;
    }
    hashTable[i]=NULL;
  }
  currentNumValues=0;
}

void DictionaryHash::put(char *key, char *value) {
  int hashSlot=hash(key);  
  dictionaryHashValue_t *cItem=hashTable[hashSlot];
  if (cItem) {
    while (cItem) {
      if (strcmp(cItem->key, key)==0) {
        // it exists - overwrite...
        if (cItem->value) {
          delete cItem->value;
          cItem->value=NULL;
        }
        cItem->value=strdup(value);
        return;
      }
      cItem=cItem->next;
    }
    // not found... create new record...
    dictionaryHashValue_t *newItem=new dictionaryHashValue_t(key, value);
    newItem->next=hashTable[hashSlot];
    hashTable[hashSlot]=newItem;
  } else {
    hashTable[hashSlot]=new dictionaryHashValue_t(key, value);
  }
  currentNumValues++;
}

char *DictionaryHash::get(char *key) {
  if (!key) return NULL;

  int hashSlot=hash(key);  
  dictionaryHashValue_t *cItem=hashTable[hashSlot];
  while (cItem) {
    if (strcmp(cItem->key, key)==0) {
      return cItem->value;
    }
    cItem=cItem->next;
  }

  return NULL;
}


/*==========================================================================
 * Copyright (c) 2003 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
 */


#ifndef _LEMUR_KEYFILE_H
#define _LEMUR_KEYFILE_H
#include <string>
namespace lemur 
{
  namespace file 
  {
    
    /// \brief Provides C++ interface to the keyfile b-tree package. Arbitrary data can
    /// be stored using either int or char * keys. 
    class Keyfile {
    public:
      ///initialize to empty
      Keyfile() : _handleSize(0), _handle(NULL) {
      }

      /// Insert value into b-tree for the given key. Throws an Exception if
      /// the operation fails.
      void put( const char* key, const void* value, int valueSize );

      /// Insert value into b-tree for the given key. Throws an Exception if
      /// the operation fails.
      void put( int key, const void* value, int valueSize );

      /// Retrieve the value in the b-tree for the given key. 
      /// Returns false if key does not exist in the b-tree.
      bool get( const char* key, void* value, int& actualSize, int maxSize ) const;

      /// Retrieve the value in the b-tree for the given key. 
      /// Returns false if key does not exist in the b-tree.
      bool get( const char* key, char** value, int& actualSize ) const;

      /// Retrieve the value in the b-tree for the given key. 
      /// Returns false if key does not exist in the b-tree.
      bool get( int key, void* value, int& actualSize, int maxSize ) const;

      /// Retrieve the value in the b-tree for the given key. 
      /// Returns false if key does not exist in the b-tree.
      bool get( int key, char** value, int& actualSize ) const;

      // redundant with getNext... combine.
      bool next( char* key, int& keyLength, char* value, int& valueLength );
      bool next( int& key, char* value, int& valueLength );

      bool previous( char* key, int& keyLength, char* value, int& valueLength );
      bool previous( int& key, char* value, int& valueLength );

      /// Return the size of the data in the b-tree for the given key. 
      /// Returns -1 if key does not exist in the b-tree.
      int getSize( const char* key ) const;

      /// Return the size of the data in the b-tree for the given key. 
      /// Returns -1 if key does not exist in the b-tree.
      int getSize( int key ) const;

      /// Remove the entry in the b-tree for the given key.
      void remove( const char* key );

      /// Remove the entry in the b-tree for the given key.
      void remove( int key );

      /// Open a keyfile with the given filename, with cacheSize (default 1MB).
      void open( const std::string& filename, int cacheSize = 1024 * 1024, bool readOnly = false);

      /// Open a keyfile with the given filename, with cacheSize (default 1MB).
      void open( const char* filename, int cacheSize = 1024 * 1024, bool readOnly = false );

      void openRead( const std::string& filename, int cacheSize = 1024 * 1024 );

      /// Create a keyfile with the given filename, with cacheSize (default 1MB).
      void create( const std::string& filename, int cacheSize = 1024 * 1024 );

      /// Create a keyfile with the given filename, with cacheSize (default 1MB).
      void create( const char* filename, int cacheSize = 1024 * 1024 );

      /// Close a keyfile.
      void close();

      /// Initialize keyfile to first key for iteration
      void setFirst();

      ///get the next key and value pair from the keyfile.
      bool getNext( int& key, void* value, int& actualSize, int maxSize ) const;

      ///get the next key and value pair from the keyfile.
      bool getNext( char* key, int maxKeySize, void* value, 
                    int& actualSize, int maxSize ) const;

      ///get the next key and value pair from the keyfile.
      /// return size of key in actKeySize
      bool getNext( char* key, int& actKeySize, int maxKeySize, void* value, 
                    int& actualSize, int maxSize ) const;

      enum {
        /// Maximum length of any key in a keyfile.
        MAX_KEY_LENGTH = 512
      };
  
    private:
      char* _handle; // file control block of the keyfile
      int _handleSize; // sizeof _handle buffer

      void _buildHandle( int cacheSize );
      void _createKey( char* keyBuf, int number ) const;
      int _decodeKey( char* keyBuf ) const;
    };
  }
}

#endif // _LEMUR_KEYFILE_H

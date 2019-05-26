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
// HashTable
//
// 9 January 2004 - tds
//

#ifndef LEMUR_HASHTABLE_HPP
#define LEMUR_HASHTABLE_HPP

#include <utility>
#include "indri/RegionAllocator.hpp"
namespace indri
{
  namespace utility
  {
    
    //
    // GenericHash<_Key>
    //

    template<class _Key>
    class GenericHash { 
    public:
      size_t operator() ( const _Key& k ) const {
        return (size_t) k;
      }
    };

    //
    // GenericComparator<_Key>
    //

    template<class _Key>
    class GenericComparator {
    public:
      size_t operator() ( const _Key& one, const _Key& two ) const {
        return (size_t) (one - two);
      }
    };

    //
    // GenericHash<const char *>
    //

    template<>
    class GenericHash<const char*> {
    public:
      size_t operator() ( const char* const& kp ) const {
        // attributed to Dan Bernstein, comp.lang.c 
        size_t hash = 5381;
        const char* k = kp;
        char c;

        for( ; (c = *k); k++ ){
          hash = ((hash << 5) + hash) + c;
        }

        return hash;
      }
    };

    //
    // GenericComparator<const char *>
    //

    template<>
    class GenericComparator<const char*> {
    public:
      size_t operator () ( const char* const& one, const char* const& two ) const {
        return strcmp( one, two );
      }
    };

    //
    // GenericHash<std::string>
    // (formerly StringHash from TaggedTextParser.hpp)
    //

    template<>
    class GenericHash<std::string> {
    public:
      size_t operator() ( const std::string& key ) const {
        GenericHash<const char*> charHash;
        return charHash( key.c_str() );
      }
    };

    //
    // GenericComparator<std::string>
    // (formerly StringComparator from TaggedTextParser.hpp)
    //

    template<>
    class GenericComparator<std::string> {
    public:
      size_t operator() ( const std::string& one, const std::string& two ) const {
        return one.compare(two);
      }
    };

    //
    // HashBucket<_Key, _Value>
    //

    template<class _Key, class _Value>
    struct HashBucket {
      _Key key;
      _Value value;
      HashBucket<_Key, _Value>* next;

      HashBucket() : next( 0 ) {};
      HashBucket( const _Key& k, HashBucket<_Key, _Value>* n ) : key(k), next(n) {};
      HashBucket( const _Key& k, const _Value& v, HashBucket<_Key, _Value>* n ) : key(k), value(v), next(n) {};
    };

    //
    // HashTableIterator<_Key, _Value, _Comparator>
    //

    template<class _Key, class _Value, class _Comparator>
    class HashTableIterator {
    private:
      typedef HashBucket<_Key, _Value> bucket_type;
      bucket_type** _table;
      bucket_type* _currentEntry;
      size_t _currentBucket;
      size_t _totalBuckets;
      std::pair<_Key*, _Value*> _pair;

      void next() {
        // already at end
        if( _currentBucket == (size_t)-1 )
          return;

        // in a chain with more entries left
        if( _currentEntry && _currentEntry->next != 0 ) {
          _currentEntry = _currentEntry->next;
          return;
        }

        if( _currentEntry )
          _currentBucket++;

        for( ; _currentBucket < _totalBuckets; _currentBucket++ ) {
          _currentEntry = _table[_currentBucket];

          if( _currentEntry ) {
            return;
          }
        }

        // none left
        _currentEntry = 0;
        _currentBucket = (size_t)-1;
      }

    public:
      HashTableIterator() {
        _currentBucket = (size_t)-1;
        _currentEntry = 0;
      }

      HashTableIterator( bucket_type** table, size_t totalBuckets ) {
        _table = table;
        _totalBuckets = totalBuckets;

        _currentBucket = 0;
        _currentEntry = 0;
        next();
      }

      bool operator == ( const HashTableIterator& other ) {
        if( other._currentEntry == _currentEntry &&
            other._currentBucket == _currentBucket )
          return true;
        return false;
      }

      bool operator != ( const HashTableIterator& other ) {
        if( other._currentEntry == _currentEntry &&
            other._currentBucket == _currentBucket )
          return false;
        return true;
      }

      void operator++ ( int ) {
        next();
      }

      std::pair<_Key*, _Value*>& operator* () {
        _pair.first = &_currentEntry->key;
        _pair.second = &_currentEntry->value;
        return _pair;
      }

      std::pair<_Key*, _Value*>* operator-> () {
        return &(*(*this));
      }
    };

    template<class _Key, class _Value, class _HashFunction = GenericHash<_Key>, class _Comparator = GenericComparator<_Key> >
    class HashTable {
    public:
      friend class HashTableIterator<_Key, _Value, _Comparator>;

      typedef HashBucket<_Key, _Value> bucket_type;
      typedef _Key key_type;
      typedef _Value value_type;
      typedef _HashFunction hash_type;
      typedef _Comparator compare_type;
      typedef class HashTableIterator<_Key, _Value, _Comparator> iterator;

    private:
      indri::utility::RegionAllocator* _allocator;
      bucket_type** _table;
      hash_type _hash;
      compare_type _compare;
      size_t _buckets;
      iterator _end;
      size_t _count;

      void _deleteBucket( bucket_type* b ) {
        if( !_allocator )
          delete b;
      }

      bucket_type* _newBucket( const _Key& k, bucket_type* p ) {
        bucket_type* b = 0;

        if( _allocator ) {
          b = (bucket_type*) _allocator->allocate( sizeof (bucket_type) );
          new(b) bucket_type( k, p );
        } else {
          b = new bucket_type( k, p );
        }

        return b;
      }

      bucket_type* _newBucket( const _Key& k, const _Value& v, bucket_type* p ) {
        bucket_type* b = 0;

        if( _allocator ) {
          b = (bucket_type*) _allocator->allocate( sizeof (bucket_type) );
          new(b) bucket_type( k, v, p );
        } else {
          b = new bucket_type( k, v, p );
        }

        return b;
      }

      bucket_type** _parentBucket( const _Key& k ) const {
        size_t index = _hash(k) % _buckets;
        return &_table[index];
      }

    public:
      HashTable( size_t size = 16384, indri::utility::RegionAllocator* allocator = 0 ) {
        _allocator = allocator;
        _buckets = size / sizeof(bucket_type*);
        _table = reinterpret_cast<bucket_type**>(new char[_buckets * sizeof(bucket_type*)]);
        _count = 0;
    
        memset( _table, 0, _buckets * sizeof(bucket_type*) );
      }

      ~HashTable() {
        clear();
        delete[] reinterpret_cast<char*>(_table);
      }

      _Value* find( const _Key& k ) const {
        bucket_type* bucket = *_parentBucket(k);

        for( ; bucket; bucket = bucket->next ) {
          if( _compare( k, bucket->key ) == 0 ) {
            return &bucket->value;
          }
        }

        return 0;
      }

      _Value* insert( const _Key& k ) {
        bucket_type** bucket = _parentBucket(k);
        _count++;

        // go to the end of the chain
        while( *bucket )
        bucket = &(*bucket)->next;

        // insert a new item
        bucket_type* newItem = _newBucket( k, 0 );
        *bucket = newItem;

        return &newItem->value;
      }

      _Value* insert( const _Key& k, const _Value& v ) {
        bucket_type** bucket = _parentBucket(k);
        _count++;

        // go to the end of the chain
        while( *bucket )
        bucket = &(*bucket)->next;

        // insert a new item
        bucket_type* newItem = _newBucket( k, v, 0 );
        *bucket = newItem;

        return &newItem->value;
      }

      void remove( const _Key& k ) {
        bucket_type** bucket = _parentBucket(k);

        while( *bucket ) {
          if( _compare( k, (*bucket)->key ) == 0 ) {
            bucket_type* nextItem = (*bucket)->next;
            bucket_type* thisItem = (*bucket);

            *bucket = nextItem;
            _deleteBucket( thisItem );
            _count--;
            break;
          }

          bucket = &(*bucket)->next;
        }
      }

      void clear() {
        if( _allocator ) {
          memset( _table, 0, sizeof(bucket_type*) * _buckets );
        } else {
          for( size_t i=0; i<_buckets; i++ ) {
            bucket_type* item = _table[i];

            while( item ) {
              bucket_type* nextItem = item->next;
              _deleteBucket( item );
              item = nextItem;
            }

            _table[i] = 0;
          }
        }

        _count = 0;
      }

      const iterator& end() {
        return _end;
      }

      iterator begin() {
        return iterator( _table, _buckets );
      }

      size_t size() {
        return _count;
      }
    };
  }
}

#endif // LEMUR_HASHTABLE_HPP



//
// MetadataPairVector.i
//
// 24 August 2004 -- tds
//

#ifdef SWIGJAVA
%typemap(jni) const std::vector<indri::parse::MetadataPair>& "jobjectArray"
%typemap(jtype) const std::vector<indri::parse::MetadataPair>& "Map"
%typemap(jstype) const std::vector<indri::parse::MetadataPair>& "Map"

%typemap(in) const std::vector<indri::parse::MetadataPair>& ( std::vector<indri::parse::MetadataPair> mdin, indri::utility::Buffer mdbuf ) {
  // call map.entrySet()
  jclass mapClazz = jenv->GetObjectClass( $input );
  jmethodID mapEntrySetMethod = jenv->GetMethodID( mapClazz, "entrySet", "()Ljava/util/Set;" );
  jobject mapEntrySet = jenv->CallObjectMethod( $input, mapEntrySetMethod );

  // call entrySet.toArray()
  jclass entrySetClazz = jenv->GetObjectClass( mapEntrySet );
  jmethodID entrySetToArrayMethod = jenv->GetMethodID( entrySetClazz, "toArray", "()[Ljava/lang/Object;" );
  jobjectArray entryArray = (jobjectArray) jenv->CallObjectMethod( mapEntrySet, entrySetToArrayMethod );

  // get array length
  jsize arrayLength = jenv->GetArrayLength( entryArray );
  $1 = &mdin;

  jclass stringClazz = jenv->FindClass("java/lang/String");
  unsigned int i;

  for( i=0; i<arrayLength; i++ ) {
    jobject mapEntry = jenv->GetObjectArrayElement( entryArray, i );
    jclass mapEntryClazz = jenv->GetObjectClass( mapEntry );
    jmethodID mapEntryGetKeyMethod = jenv->GetMethodID( mapEntryClazz, "getKey", "()Ljava/lang/Object;" );
    jmethodID mapEntryGetValueMethod = jenv->GetMethodID( mapEntryClazz, "getValue", "()Ljava/lang/Object;" );

    jobject key = jenv->CallObjectMethod( mapEntry, mapEntryGetKeyMethod );
    jobject value = jenv->CallObjectMethod( mapEntry, mapEntryGetValueMethod );

    size_t keyOffset = mdbuf.position();
    const char* keyChars = jenv->GetStringUTFChars( (jstring) key, 0 );
    jsize keyLength = jenv->GetStringUTFLength( (jstring) key);
    std::string keyString = keyChars;
    char* keyPosition = mdbuf.write( keyLength+1 );
    strncpy( keyPosition, keyChars, keyLength );
    keyPosition[keyLength] = 0;
    
    size_t valueOffset = mdbuf.position();
    char* valuePosition = 0;
    jsize valueLength;

    if( jenv->IsInstanceOf( value, stringClazz ) ) {
      jstring valueString = (jstring) value;
      const char* valueChars = jenv->GetStringUTFChars( valueString, 0);
      valueLength = jenv->GetStringUTFLength( valueString );

      valuePosition = mdbuf.write( valueLength+1 );
      strncpy( valuePosition, valueChars, valueLength );
      valuePosition[valueLength] = 0;
      valueLength++;
      
      jenv->ReleaseStringUTFChars(valueString, valueChars);
    } else {
      // is byte array
      jbyteArray valueArray = (jbyteArray) value;
      jbyte* valueBytes = jenv->GetByteArrayElements( valueArray, 0 );
      valueLength = jenv->GetArrayLength( valueArray );
  
      valuePosition = mdbuf.write( valueLength+1 );
      memcpy( valuePosition, valueBytes, valueLength );
      valuePosition[valueLength] = 0;

      jenv->ReleaseByteArrayElements(valueArray, valueBytes, 0);
    }

    indri::parse::MetadataPair pair;
    pair.key = (char*) keyOffset;
    pair.value = (char*) valueOffset;
    pair.valueLength = valueLength;
    mdin.push_back(pair);

    jenv->ReleaseStringUTFChars( (jstring)key, keyChars);
  }
  
  // now we need to fix up the key and value positions
  for( i=0; i<arrayLength; i++ ) {
    mdin[i].key = mdbuf.front() + (size_t) mdin[i].key;
    mdin[i].value = mdbuf.front() + (size_t) mdin[i].value;
  }
}

%typemap(javain) const std::vector<indri::parse::MetadataPair>& "$javainput";

#endif

#ifdef SWIGCSHARP


SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(MetadataPair, indri::parse::MetadataPair)

%template(MetadataPairSTDVector) std::vector<indri::parse::MetadataPair>;

namespace indri {
  namespace parse 
  {
    struct MetadataPair {
      const char* key;
      const char* value;
      int valueLength;
    };
  }
}

#endif


//
// StringMap.i
//
// 24 August 2004 -- tds
//

#ifdef SWIGJAVA
%typemap(jni) const std::map<std::string,std::string>& "jobjectArray"
%typemap(jtype) const std::map<std::string,std::string>& "Map"
%typemap(jstype) const std::map<std::string,std::string>& "Map"

%typemap(in) const std::map<std::string,std::string>& ( std::map<std::string,std::string> mapin ) {
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
  $1 = &mapin;

  for( unsigned int i=0; i<arrayLength; i++ ) {
    jobject mapEntry = jenv->GetObjectArrayElement( entryArray, i );
    jclass mapEntryClazz = jenv->GetObjectClass( mapEntry );
    jmethodID mapEntryGetKeyMethod = jenv->GetMethodID( mapEntryClazz, "getKey", "()Ljava/lang/Object;" );
    jmethodID mapEntryGetValueMethod = jenv->GetMethodID( mapEntryClazz, "getValue", "()Ljava/lang/Object;" );

    jobject key = jenv->CallObjectMethod( mapEntry, mapEntryGetKeyMethod );
    jobject value = jenv->CallObjectMethod( mapEntry, mapEntryGetValueMethod );

    const char* keyChars = jenv->GetStringUTFChars( (jstring) key, 0 );
    std::string keyString = keyChars;
    jenv->ReleaseStringUTFChars( (jstring) key, keyChars );

    const char* valueChars = jenv->GetStringUTFChars( (jstring) value, 0 );
    std::string valueString = valueChars;
    jenv->ReleaseStringUTFChars( (jstring) value, valueChars );

    mapin[keyString] = valueString;
  }
}

%typemap(javain) const std::map<std::string,std::string>& "$javainput";

#endif


//
// ConflationPattern.i
//
// 26 Sept 2005 -- dmf
//
#ifdef SWIGJAVA

%typemap(jni) const std::map<indri::parse::ConflationPattern*,std::string>& "jobjectArray"
%typemap(jtype) const std::map<indri::parse::ConflationPattern*,std::string>& "Map"
%typemap(jstype) const std::map<indri::parse::ConflationPattern*,std::string>& "Map"

%typemap(javain) const std::map<indri::parse::ConflationPattern*,std::string>& "$javainput";

%typemap(in) const std::map<indri::parse::ConflationPattern*,std::string>&  (std::map<indri::parse::ConflationPattern*,std::string> map){
  // make a conflations map to go in it
  // get map class and entrySet method pointer
  jobject src = $input;
  jclass mapClazz = jenv->GetObjectClass(src);
  jmethodID mapEntrySet = jenv->GetMethodID(mapClazz, "entrySet", "()Ljava/util/Set;" );

  // call entry set function to set a Set of entries
  jobject entrySet = jenv->CallObjectMethod(src, mapEntrySet);
  jclass setClazz = jenv->GetObjectClass(entrySet);
  jmethodID setToArray = jenv->GetMethodID(setClazz, "toArray", "()[Ljava/lang/Object;" );

  // turn that set into an array of objects (entries)
  jobjectArray entryArray = (jobjectArray) jenv->CallObjectMethod(entrySet, setToArray);

  // get the array size
  jsize entryArrayLength = jenv->GetArrayLength(entryArray);

  jclass conflationClazz = jenv->FindClass("lemurproject/indri/ConflationPattern");
  jfieldID tag_nameField = jenv->GetFieldID(conflationClazz, "tag_name", "Ljava/lang/String;" );
  jfieldID attribute_nameField = jenv->GetFieldID(conflationClazz, "attribute_name", "Ljava/lang/String;"  );
  jfieldID valueField = jenv->GetFieldID(conflationClazz, "value", "Ljava/lang/String;" );


  for( int i=0; i<entryArrayLength; i++ ) {
    // get the key string
    jobject entryObject = (jstring) jenv->GetObjectArrayElement( entryArray, i );
    jclass mapEntryClazz = jenv->GetObjectClass(entryObject);
    jmethodID mapEntryGetKey = jenv->GetMethodID(mapEntryClazz, "getKey", "()Ljava/lang/Object;" );
    jmethodID mapEntryGetValue = jenv->GetMethodID(mapEntryClazz, "getValue", "()Ljava/lang/Object;" );

    jobject key = jenv->CallObjectMethod( entryObject, mapEntryGetKey );
    jobject value = jenv->CallObjectMethod( entryObject, mapEntryGetValue );
    
    indri::parse::ConflationPattern * pattern = new indri::parse::ConflationPattern();

    const char* valueChars = jenv->GetStringUTFChars( (jstring) value, 0 );
    std::string valueString = valueChars;
    jenv->ReleaseStringUTFChars( (jstring) value, valueChars );

    jstring fieldValue = (jstring) jenv->GetObjectField(key, tag_nameField);
    valueChars = jenv->GetStringUTFChars( (jstring) fieldValue, 0 );
    if (valueChars[0] == '\0') valueChars = NULL; // empty strings are NULL
    pattern->tag_name = valueChars;
  
    fieldValue = (jstring) jenv->GetObjectField(key, attribute_nameField);
    valueChars = jenv->GetStringUTFChars( (jstring) fieldValue, 0 );
    if (valueChars[0] == '\0') valueChars = NULL; // empty strings are NULL
    pattern->attribute_name = valueChars;

    fieldValue = (jstring) jenv->GetObjectField(key, valueField);
    valueChars = jenv->GetStringUTFChars( (jstring) fieldValue, 0 );
    if (valueChars[0] == '\0') valueChars = NULL; // empty strings are NULL
    pattern->value = valueChars;
    map[pattern] = valueString ;
  }
  $1 = &map;
}
#endif
#ifdef SWIGCSHARP
%typemap(ctype) indri::parse::ConflationPattern * "void *"
%typemap(imtype, out="IntPtr") indri::parse::ConflationPattern * "HandleRef"
%typemap(cstype) indri::parse::ConflationPattern * "ConflationPattern"

%template(ConfMap) std::map<indri::parse::ConflationPattern*, std::string>;

namespace indri {
  namespace parse {

    // The tag_name and attribute_name strings in the
    // ConflationPattern should always be downcased, but value should
    // appear as it does in the source document.
    
    struct ConflationPattern {
      const char* tag_name;
      const char* attribute_name;
      const char* value;
    };

  }
}

#endif

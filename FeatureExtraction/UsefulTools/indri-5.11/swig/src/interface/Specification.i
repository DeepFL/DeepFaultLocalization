//
// Specification.i
//
// Typemaps for FileClassEnvironmentFactory::Specification&
//
// 14 February 2005 -- dmf
//
#ifdef SWIGJAVA
%typemap(jni) const indri::parse::FileClassEnvironmentFactory::Specification& "jobject"
%typemap(jtype) const indri::parse::FileClassEnvironmentFactory::Specification& "Specification"
%typemap(jstype) const indri::parse::FileClassEnvironmentFactory::Specification& "Specification"

%typemap(jni) indri::parse::FileClassEnvironmentFactory::Specification* "jobject"
%typemap(jtype) indri::parse::FileClassEnvironmentFactory::Specification* "Specification"
%typemap(jstype) indri::parse::FileClassEnvironmentFactory::Specification* "Specification"

%{
  struct jni_specification_info {
    // Specification
    jclass specClazz;
    jmethodID specConstructor;
    jfieldID nameField;
    jfieldID tokenizerField;
    jfieldID parserField;
    jfieldID iteratorField;
    jfieldID startDocTagField;
    jfieldID endDocTagField;
    jfieldID endMetadataTagField;
    jfieldID includeField;
    jfieldID excludeField;
    jfieldID indexField;
    jfieldID metadataField;
    jfieldID conflationsField;
    // support classes
    jclass mapClazz;
    jmethodID mapConstructor;
    jmethodID putMethod;
  };
 
  void print_info(jni_specification_info& info) 
    {
      std::cerr << info.specClazz << std::endl;
      std::cerr << info.specConstructor << std::endl;
      // get all the fields
      std::cerr << info.nameField << std::endl;
      std::cerr << info.tokenizerField << std::endl;
      std::cerr << info.parserField << std::endl;
      std::cerr << info.iteratorField << std::endl;
      std::cerr << info.startDocTagField << std::endl;
      std::cerr << info.endDocTagField << std::endl;
      std::cerr << info.endMetadataTagField << std::endl;
      std::cerr << info.includeField << std::endl;
      std::cerr << info.excludeField << std::endl;
      std::cerr << info.indexField << std::endl;
      std::cerr << info.metadataField << std::endl;
      std::cerr << info.conflationsField << std::endl;
      std::cerr << info.mapClazz << std::endl;
      std::cerr << info.mapConstructor << std::endl;
      std::cerr << info.putMethod << std::endl;
    }
 
  void specification_init( JNIEnv* jenv, jni_specification_info& info ) {

    info.specClazz = jenv->FindClass("lemurproject/indri/Specification");
    info.specConstructor = jenv->GetMethodID(info.specClazz, "<init>", "()V" );
    // get all the fields
    info.nameField = jenv->GetFieldID(info.specClazz, "name", "Ljava/lang/String;");
    info.tokenizerField = jenv->GetFieldID(info.specClazz, "tokenizer", "Ljava/lang/String;");  
    info.parserField = jenv->GetFieldID(info.specClazz, "parser", "Ljava/lang/String;");
    info.iteratorField = jenv->GetFieldID(info.specClazz, "iterator", "Ljava/lang/String;");
    info.startDocTagField = jenv->GetFieldID(info.specClazz, "startDocTag", "Ljava/lang/String;");
    info.endDocTagField = jenv->GetFieldID(info.specClazz, "endDocTag", "Ljava/lang/String;");
    info.endMetadataTagField = jenv->GetFieldID(info.specClazz, "endMetadataTag", "Ljava/lang/String;");
    info.includeField = jenv->GetFieldID(info.specClazz, "include", "[Ljava/lang/String;");
    info.excludeField = jenv->GetFieldID(info.specClazz, "exclude", "[Ljava/lang/String;");
    info.indexField = jenv->GetFieldID(info.specClazz, "index", "[Ljava/lang/String;");
    info.metadataField = jenv->GetFieldID(info.specClazz, "metadata", "[Ljava/lang/String;");
    info.conflationsField = jenv->GetFieldID(info.specClazz, "conflations", "Ljava/util/Map;");

    info.mapClazz = jenv->FindClass("java/util/HashMap");
    info.mapConstructor = jenv->GetMethodID(info.mapClazz, "<init>", "()V" );
    info.putMethod = jenv->GetMethodID(info.mapClazz, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;" );
  }

  jobjectArray string_vector_copy(JNIEnv* jenv, std::vector<std::string> &vec) {
    jclass stringClazz = jenv->FindClass("java/lang/String" );
    // fill in array  
    jobjectArray stringArray = jenv->NewObjectArray(vec.size(), stringClazz,
                                                    NULL);
    for(int i = 0; i < vec.size(); i++ ) {
      jstring val = jenv->NewStringUTF(vec[i].c_str());
      jenv->SetObjectArrayElement(stringArray, i, val);
    }
    return stringArray;
  }
 
  jobject specification_copy( JNIEnv* jenv, jni_specification_info& info, 
                              indri::parse::FileClassEnvironmentFactory::Specification* thisSpec ) {
    jobject result = jenv->NewObject(info.specClazz, info.specConstructor);
    // initialize the fields
    jstring stringField;
    stringField = jenv->NewStringUTF(thisSpec->name.c_str());
    jenv->SetObjectField(result, info.nameField, stringField);
    stringField = jenv->NewStringUTF(thisSpec->tokenizer.c_str());
    jenv->SetObjectField(result, info.tokenizerField, stringField);
    stringField = jenv->NewStringUTF(thisSpec->parser.c_str());
    jenv->SetObjectField(result, info.parserField, stringField);
    stringField = jenv->NewStringUTF(thisSpec->iterator.c_str());
    jenv->SetObjectField(result, info.iteratorField, stringField);
    stringField = jenv->NewStringUTF(thisSpec->startDocTag.c_str());
    jenv->SetObjectField(result, info.startDocTagField, stringField);
    stringField = jenv->NewStringUTF(thisSpec->endDocTag.c_str());
    jenv->SetObjectField(result, info.endDocTagField, stringField);
    stringField = jenv->NewStringUTF(thisSpec->endMetadataTag.c_str());
    jenv->SetObjectField(result, info.endMetadataTagField, stringField);
    // make a conflations map to go in it

    jclass conflationClazz = jenv->FindClass("lemurproject/indri/ConflationPattern");
    jmethodID conflationConstructor = jenv->GetMethodID(conflationClazz, "<init>", "()V" ); 
    jfieldID tag_nameField = jenv->GetFieldID(conflationClazz, "tag_name", "Ljava/lang/String;" );
    jfieldID attribute_nameField = jenv->GetFieldID(conflationClazz, "attribute_name", "Ljava/lang/String;"  );
    jfieldID valueField = jenv->GetFieldID(conflationClazz, "value", "Ljava/lang/String;" );

    jobject mapObject = jenv->NewObject(info.mapClazz, info.mapConstructor);
    for( std::map<indri::parse::ConflationPattern *, std::string>::iterator iter = thisSpec->conflations.begin(); 
         iter != thisSpec->conflations.end(); iter++ ) {
      const indri::parse::ConflationPattern *thisKey = iter->first;
      const std::string &thisVal = iter->second;
      jobject patternObject = jenv->NewObject(conflationClazz, conflationConstructor);
      jstring patVal;
      const char *c_str = thisKey->tag_name;
      if (c_str == NULL) c_str = ""; // don't give NewStringUTF a NULL.
      patVal = jenv->NewStringUTF(c_str);
      jenv->SetObjectField(patternObject, tag_nameField, patVal);
      c_str = thisKey->attribute_name;
      if (c_str == NULL) c_str = ""; // don't give NewStringUTF a NULL.
      patVal = jenv->NewStringUTF(c_str);
      jenv->SetObjectField(patternObject, attribute_nameField, patVal);
      c_str = thisKey->value;
      if (c_str == NULL) c_str = ""; // don't give NewStringUTF a NULL.
      patVal = jenv->NewStringUTF(c_str);
      jenv->SetObjectField(patternObject, valueField, patVal);

      jstring val = jenv->NewStringUTF(thisVal.c_str());
      jenv->CallObjectMethod(mapObject, info.putMethod, patternObject, val);
    }
    jenv->SetObjectField(result, info.conflationsField, mapObject);
    jobjectArray stringArray = string_vector_copy(jenv, thisSpec->include);
    jenv->SetObjectField(result, info.includeField, stringArray);
    stringArray = string_vector_copy(jenv, thisSpec->exclude);
    jenv->SetObjectField(result, info.excludeField, stringArray);
    stringArray = string_vector_copy(jenv, thisSpec->index);
    jenv->SetObjectField(result, info.indexField, stringArray);
    stringArray = string_vector_copy(jenv, thisSpec->metadata);
    jenv->SetObjectField(result, info.metadataField, stringArray);
    return result;
  }

  // copy to string
  void copy_to_string(JNIEnv* jenv, jstring src, std::string &target) {
    jsize stringLength = jenv->GetStringUTFLength(src);
    const char* stringChars = jenv->GetStringUTFChars(src, 0);
    target.assign( stringChars, stringChars + stringLength );
  }

  // copy to string vector (stringvector.i)
  void copy_to_string_vector(JNIEnv* jenv, jobjectArray src, 
                             std::vector<std::string> &target) {
    jsize arrayLength = jenv->GetArrayLength(src);
    for( unsigned int i = 0; i < arrayLength; i++ ) {
      std::string stringCopy;
      jstring str = (jstring) jenv->GetObjectArrayElement(src, i);
      copy_to_string(jenv, str, stringCopy);    
      target.push_back(stringCopy);
    }
  }
 
  // copy to map (stringmap.i)
  void copy_to_map(JNIEnv* jenv, jobject src,
                   std::map<indri::parse::ConflationPattern*, std::string> &map) {

    // get map class and entrySet method pointer
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
  }
 
  %}

%typemap(in) const indri::parse::FileClassEnvironmentFactory::Specification& (indri::parse::FileClassEnvironmentFactory::Specification spec) {
  // look up information about Specification
  jni_specification_info info;
  specification_init(jenv, info);

  jstring tmpString;
  jobjectArray tmpArray;
  jobject tmpMap;
  
  // string name
  tmpString = (jstring) jenv->GetObjectField($input, info.nameField);
  copy_to_string(jenv, tmpString, spec.name);
  // string tokenizer
  tmpString = (jstring) jenv->GetObjectField($input, info.tokenizerField);
  copy_to_string(jenv, tmpString, spec.tokenizer);
  // string parser
  tmpString = (jstring) jenv->GetObjectField($input, info.parserField);
  copy_to_string(jenv, tmpString, spec.parser);
  // string interator
  tmpString = (jstring) jenv->GetObjectField($input, info.iteratorField);
  copy_to_string(jenv, tmpString, spec.iterator);
  // string startDocTag
  tmpString = (jstring) jenv->GetObjectField($input, info.startDocTagField);
  copy_to_string(jenv, tmpString, spec.startDocTag);
  // string endDocTag
  tmpString = (jstring) jenv->GetObjectField($input, info.endDocTagField);
  copy_to_string(jenv, tmpString, spec.endDocTag);
  // string endMetadataTag
  tmpString = (jstring) jenv->GetObjectField($input, info.endMetadataTagField);
  copy_to_string(jenv, tmpString, spec.endMetadataTag);
  // vector<string> include 
  tmpArray = (jobjectArray) jenv->GetObjectField($input, info.includeField);
  copy_to_string_vector(jenv, tmpArray, spec.include);
  // vector<string> exclude
  tmpArray = (jobjectArray) jenv->GetObjectField($input, info.excludeField);
  copy_to_string_vector(jenv, tmpArray, spec.exclude);
  // vector<string> index
  tmpArray = (jobjectArray) jenv->GetObjectField($input, info.indexField);
  copy_to_string_vector(jenv, tmpArray, spec.index);
  // vector<string> metadata
  tmpArray = (jobjectArray) jenv->GetObjectField($input, info.metadataField);
  copy_to_string_vector(jenv, tmpArray, spec.metadata);
  // map<string, string> conflations 
  tmpMap = (jobject) jenv->GetObjectField($input, info.conflationsField);
  copy_to_map(jenv, tmpMap, spec.conflations);

  $1 = &spec;
}

%typemap(javain) const indri::parse::FileClassEnvironmentFactory::Specification& "$javainput";


%typemap(out) indri::parse::FileClassEnvironmentFactory::Specification*
{
  // look up information about Specification
  jni_specification_info info;
  specification_init(jenv, info);
  //  print_info(info);
  $result = specification_copy(jenv, info, $1); 
  delete($1);
}

%typemap(javaout) indri::parse::FileClassEnvironmentFactory::Specification* {
  return $jnicall;
}

#endif

#ifdef SWIGCSHARP
typedef indri::parse::Specification indri::parse::FileClassEnvironmentFactory::Specification;

%typemap(ctype) indri::parse::FileClassEnvironmentFactory::Specification * "void *"
%typemap(imtype, out="IntPtr") indri::parse::FileClassEnvironmentFactory::Specification * "HandleRef"
%typemap(cstype) indri::parse::FileClassEnvironmentFactory::Specification * "Specification"

%typemap(ctype) indri::parse::FileClassEnvironmentFactory::Specification & "void *"
%typemap(imtype, out="IntPtr") indri::parse::FileClassEnvironmentFactory::Specification & "HandleRef"
%typemap(cstype) indri::parse::FileClassEnvironmentFactory::Specification & "Specification"

%typemap(ctype) const indri::parse::FileClassEnvironmentFactory::Specification & "void *"
%typemap(imtype, out="IntPtr") const indri::parse::FileClassEnvironmentFactory::Specification & "HandleRef"
%typemap(cstype) const indri::parse::FileClassEnvironmentFactory::Specification & "Specification"


%{
  namespace indri 
  {
    namespace parse
    {
      typedef indri::parse::FileClassEnvironmentFactory::Specification Specification ;
    }
  }

  //#define Specification FileClassEnvironmentFactory::Specification
  %}
namespace indri {
  namespace parse {

    struct Specification {
      ///  name of this file class, eg trecweb
      std::string name;
      /// document parser for this file class
      std::string parser;
      /// document tokenizer for this file class
      std::string tokenizer;
      /// document iterator for this file class
      std::string iterator;
      /// tag indicating start of a document
      std::string startDocTag;
      /// tag indicating the end of a document
      std::string endDocTag;
      /// tag indicating the end of the metadata fields
      std::string endMetadataTag;
      /// \brief tags whose contents should be included in the index. 
      /// If empty, all tags are included.
      std::vector<std::string> include;
      /// tags whose contents should be excluded from the index
      std::vector<std::string> exclude;
      /// tags that should be forwarded to the index for tag extents, ie named fields. 
      std::vector<std::string> index;
      /// tags whose contents should be indexed as metadata
      std::vector<std::string> metadata;
      /// \brief tags that should be conflated. 
      /// The map is the of the form tag => conflated tag, eg h1 => heading.
      std::map<indri::parse::ConflationPattern*,std::string> conflations;
    };
  }
}
#endif

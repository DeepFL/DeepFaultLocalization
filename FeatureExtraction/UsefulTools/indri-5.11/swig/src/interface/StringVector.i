
//
// StringVector.i
//
// 10 August 2004 -- tds
//
#ifdef SWIGJAVA
%typemap(jni) const std::vector<std::string>& "jobjectArray"
%typemap(jtype) const std::vector<std::string>& "String[]"
%typemap(jstype) const std::vector<std::string>& "String[]"

%typemap(jni) std::vector<std::string> "jobjectArray"
%typemap(jtype) std::vector<std::string> "String[]"
%typemap(jstype) std::vector<std::string> "String[]"

%typemap(out) std::vector<std::string> {
  std::vector<std::string>& vec = $1;
  jclass stringClazz = jenv->FindClass("java/lang/String");
  $result = jenv->NewObjectArray(vec.size(), stringClazz, NULL);

  for( unsigned int i=0; i<vec.size(); i++ ) {
    jstring str = jenv->NewStringUTF(vec[i].c_str());
    jenv->SetObjectArrayElement( $result, i, str);
  }
}

%typemap(in) const std::vector<std::string>& ( std::vector<std::string> strin ) {
  jsize arrayLength = jenv->GetArrayLength($input);
  $1 = &strin;

  for( unsigned int i=0; i<arrayLength; i++ ) {
    jstring str = (jstring) jenv->GetObjectArrayElement($input, i);
    jsize stringLength = jenv->GetStringUTFLength(str);
    const char* stringChars = jenv->GetStringUTFChars(str, 0);
    std::string stringCopy;
    stringCopy.assign( stringChars, stringChars + stringLength );
    $1->push_back(stringCopy);
  }
}

%typemap(in) std::vector<std::string> ( std::vector<std::string> strin ) {
  jsize arrayLength = jenv->GetArrayLength($input);
  $1 = &strin;

  for( unsigned int i=0; i<arrayLength; i++ ) {
    jstring str = (jstring) jenv->GetObjectArrayElement($input, i);
    jsize stringLength = jenv->GetStringUTFLength(str);
    const char* stringChars = jenv->GetStringUTFChars(str, 0);
    std::string stringCopy;
    stringCopy.assign( stringChars, stringChars + stringLength );
    $1->push_back(stringCopy);
  }
}

%typemap(javain) std::vector<std::string> "$javainput";
%typemap(javain) const std::vector<std::string>& "$javainput";

%typemap(javaout) std::vector<std::string> {
  return $jnicall;
}

#endif
#ifdef SWIGCSHARP
%template(StringVector) std::vector<std::string>;
#endif

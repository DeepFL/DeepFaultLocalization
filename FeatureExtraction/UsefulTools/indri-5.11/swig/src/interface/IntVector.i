
//
// IntVector.i
//
// 10 August 2004 -- tds
//

#ifdef SWIGJAVA
%typemap(jni) const std::vector<int>& "jintArray"
%typemap(jtype) const std::vector<int>& "int[]"
%typemap(jstype) const std::vector<int>& "int[]"

%typemap(jni) std::vector<int> "jintArray"
%typemap(jtype) std::vector<int> "int[]"
%typemap(jstype) std::vector<int> "int[]"

%typemap(in) const std::vector<int>& ( std::vector<int> typemapin ) {
  jsize arrayLength = jenv->GetArrayLength($input);
  jint* elements = jenv->GetIntArrayElements($input, 0);
  $1 = &typemapin;

  for( unsigned int i=0; i<arrayLength; i++ ) {
    $1->push_back(elements[i]);
  }

  jenv->ReleaseIntArrayElements($input, elements, 0);
}

%typemap(javain) const std::vector<int>& "$javainput";

%typemap(javaout) std::vector<int> {
  return $jnicall;
}
%typemap(out) std::vector<int>
{  
  std::vector<int> &input = $1;
  $result = jenv->NewIntArray(input.size()); 
  jint * body = jenv->GetIntArrayElements($result, 0);
  for( jsize i=0; i<input.size(); i++ ) {
    body[i] = input[i];
  }
  jenv->ReleaseIntArrayElements($result, body, 0);
}
#endif

#ifdef SWIGCSHARP

%template(IntVector) std::vector<int>;

#endif

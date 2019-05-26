
//
// ScoredExtentArray.i
//
// Copies a C++ ScoredExtentResult vector
// java ScoredExtentResult[] array.
//
// 10 August 2004 -- tds
//
#ifdef SWIGJAVA
%typemap(jni) std::vector<indri::api::ScoredExtentResult> "jobjectArray"
%typemap(jtype) std::vector<indri::api::ScoredExtentResult> "ScoredExtentResult[]"
%typemap(jstype) std::vector<indri::api::ScoredExtentResult> "ScoredExtentResult[]"

%typemap(jni) const std::vector<indri::api::ScoredExtentResult>& "jobjectArray"
%typemap(jtype) const std::vector<indri::api::ScoredExtentResult>& "ScoredExtentResult[]"
%typemap(jstype) const std::vector<indri::api::ScoredExtentResult>& "ScoredExtentResult[]"

%typemap(jni) std::vector<indri::api::ScoredExtentResult>& "jobjectArray"
%typemap(jtype) std::vector<indri::api::ScoredExtentResult>& "ScoredExtentResult[]"
%typemap(jstype) std::vector<indri::api::ScoredExtentResult>& "ScoredExtentResult[]"

%{
  jobjectArray java_build_scoredextentresult( JNIEnv* jenv, const std::vector<indri::api::ScoredExtentResult>& input ) {
    jclass clazz = jenv->FindClass("lemurproject/indri/ScoredExtentResult");
    jmethodID constructor = jenv->GetMethodID(clazz, "<init>", "()V" );
    jobjectArray result;

    result = jenv->NewObjectArray(input.size(), clazz, NULL);
    if (!result) {
      return 0;
    }

    jfieldID scoreField = jenv->GetFieldID(clazz, "score", "D" );
    jfieldID beginField = jenv->GetFieldID(clazz, "begin", "I" );
    jfieldID endField = jenv->GetFieldID(clazz, "end", "I" );
    jfieldID documentField = jenv->GetFieldID(clazz, "document", "I" );
    jfieldID numberField = jenv->GetFieldID(clazz, "number", "J");
    jfieldID ordField = jenv->GetFieldID(clazz, "ordinal", "I");
    jfieldID pOrdField = jenv->GetFieldID(clazz, "parentOrdinal", "I");

    for( jsize i=0; i<input.size(); i++ ) {
      // make a new scored extent result object
      jobject ser = jenv->NewObject(clazz, constructor);

      // fill in the fields
      jenv->SetDoubleField(ser, scoreField, input[i].score );
      jenv->SetIntField(ser, beginField, input[i].begin );
      jenv->SetIntField(ser, endField, input[i].end );
      jenv->SetIntField(ser, documentField, input[i].document );
      jenv->SetLongField(ser, numberField, input[i].number);
      jenv->SetIntField(ser, ordField, input[i].ordinal);
      jenv->SetIntField(ser, pOrdField, input[i].parentOrdinal);

      jenv->SetObjectArrayElement(result, i, ser);
    }

    return result;
  }
  %}

%typemap(out) std::vector<indri::api::ScoredExtentResult>
{
  $result = java_build_scoredextentresult( jenv, $1 );
}

%typemap(out) const std::vector<indri::api::ScoredExtentResult>& {
  $result = java_build_scoredextentresult( jenv, *($1) );
}

%typemap(in) const std::vector<indri::api::ScoredExtentResult>& ( std::vector<indri::api::ScoredExtentResult> resin )
{
  jsize size = jenv->GetArrayLength($input);

  jclass clazz = jenv->FindClass("lemurproject/indri/ScoredExtentResult");
  jfieldID scoreField = jenv->GetFieldID(clazz, "score", "D" );
  jfieldID beginField = jenv->GetFieldID(clazz, "begin", "I" );
  jfieldID endField = jenv->GetFieldID(clazz, "end", "I" );
  jfieldID documentField = jenv->GetFieldID(clazz, "document", "I" );
  jfieldID numberField = jenv->GetFieldID(clazz, "number", "J");
  jfieldID ordField = jenv->GetFieldID(clazz, "ordinal", "I");
  jfieldID pOrdField = jenv->GetFieldID(clazz, "parentOrdinal", "I");
  $1 = &resin;

  for( jsize i=0; i<size; i++ ) {
    jobject seobj  = jenv->GetObjectArrayElement($input, i);
    indri::api::ScoredExtentResult ser;

    ser.begin = jenv->GetIntField(seobj, beginField);
    ser.end = jenv->GetIntField(seobj, endField);
    ser.document = jenv->GetIntField(seobj, documentField);
    ser.score = jenv->GetDoubleField(seobj, scoreField);
    ser.number = jenv->GetLongField(seobj, numberField);
    ser.ordinal = jenv->GetIntField(seobj, ordField);
    ser.parentOrdinal = jenv->GetIntField(seobj, pOrdField);

    $1->push_back( ser );
  }
}

%typemap(javain) const std::vector<indri::api::ScoredExtentResult>& "$javainput"
%typemap(in) std::vector<indri::api::ScoredExtentResult>& ( std::vector<indri::api::ScoredExtentResult> resin )
{
  jsize size = jenv->GetArrayLength($input);

  jclass clazz = jenv->FindClass("lemurproject/indri/ScoredExtentResult");
  jfieldID scoreField = jenv->GetFieldID(clazz, "score", "D" );
  jfieldID beginField = jenv->GetFieldID(clazz, "begin", "I" );
  jfieldID endField = jenv->GetFieldID(clazz, "end", "I" );
  jfieldID documentField = jenv->GetFieldID(clazz, "document", "I" );
  jfieldID numberField = jenv->GetFieldID(clazz, "number", "J");
  jfieldID ordField = jenv->GetFieldID(clazz, "ordinal", "I");
  jfieldID pOrdField = jenv->GetFieldID(clazz, "parentOrdinal", "I");
  $1 = &resin;

  for( jsize i=0; i<size; i++ ) {
    jobject seobj  = jenv->GetObjectArrayElement($input, i);
    indri::api::ScoredExtentResult ser;

    ser.begin = jenv->GetIntField(seobj, beginField);
    ser.end = jenv->GetIntField(seobj, endField);
    ser.document = jenv->GetIntField(seobj, documentField);
    ser.score = jenv->GetDoubleField(seobj, scoreField);
    ser.number = jenv->GetLongField(seobj, numberField);
    ser.ordinal = jenv->GetIntField(seobj, ordField);
    ser.parentOrdinal = jenv->GetIntField(seobj, pOrdField);

    $1->push_back( ser );
  }
}
%typemap(javain) std::vector<indri::api::ScoredExtentResult>& "$javainput"
%typemap(javaout) std::vector<indri::api::ScoredExtentResult> {
  return $jnicall;
}

%typemap(javaout) const std::vector<indri::api::ScoredExtentResult>& {
  return $jnicall;
}

#endif

#ifdef SWIGCSHARP
SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(ScoredExtentResult, indri::api::ScoredExtentResult)
  %template(ScoredExtentResultVector) std::vector<indri::api::ScoredExtentResult>;

namespace indri {
  namespace api {
    struct ScoredExtentResult {
      double score;
      int document;
      int begin;
      int end;
      INT64 number;
      int ordinal;
      int parentOrdinal;
    };
  }
}
#endif

#ifdef SWIGJAVA

%typemap(jni) indri::api::QueryRequest & "jobject"
%typemap(jtype) indri::api::QueryRequest & "QueryRequest"
%typemap(jstype) indri::api::QueryRequest & "QueryRequest"

%typemap(in) indri::api::QueryRequest & (indri::api::QueryRequest req )
{
  $1 = &req;
  jclass qrClazz = jenv->FindClass("lemurproject/indri/QueryRequest");
  jfieldID queryField = jenv->GetFieldID(qrClazz, "query",  "Ljava/lang/String;");
  
  jfieldID formulatorsField = jenv->GetFieldID(qrClazz, "formulators", "[Ljava/lang/String;");
  jfieldID metadataField = jenv->GetFieldID(qrClazz, "metadata", "[Ljava/lang/String;");
  jfieldID resultsRequestedField = jenv->GetFieldID(qrClazz, "resultsRequested", "I");
  jfieldID startNumField = jenv->GetFieldID(qrClazz, "startNum", "I");
  jfieldID optionsField = jenv->GetFieldID(qrClazz, "options", "I");

  jstring query = (jstring) jenv->GetObjectField($input, queryField);

  jobjectArray formulators = (jobjectArray) jenv->GetObjectField($input, formulatorsField);

  jobjectArray metadata = (jobjectArray) jenv->GetObjectField($input, metadataField);
  jint resultsRequested = jenv->GetIntField($input, resultsRequestedField);
  jint startNum = jenv->GetIntField($input, startNumField);
  jint options = jenv->GetIntField($input, optionsField);

  // fill in the values
  const char *queryString = jenv->GetStringUTFChars(query, 0);
  req.query = queryString;
  jenv->ReleaseStringUTFChars(query, queryString);
  
  jsize formCount = formulators ? jenv->GetArrayLength(formulators) : 0;
  for (int i = 0; i < formCount; i++) {
    jstring form = (jstring) jenv->GetObjectArrayElement(formulators, i);
    const char *formString = jenv->GetStringUTFChars(form, 0);
    req.formulators.push_back(formString);
    jenv->ReleaseStringUTFChars(form, formString);
  }

  jsize metaCount = metadata ? jenv->GetArrayLength(metadata) : 0;
  for (int i = 0; i < metaCount; i++) {
    jstring meta = (jstring) jenv->GetObjectArrayElement(metadata, i);
    const char *metaString =  jenv->GetStringUTFChars(meta, 0);
    req.metadata.push_back(metaString);
    jenv->ReleaseStringUTFChars(meta, metaString);
  }
  req.resultsRequested = resultsRequested;
  req.startNum = startNum;
  req.options = (indri::api::QueryRequest::Options) options;
}

%typemap(javain) indri::api::QueryRequest & "$javainput";

#endif


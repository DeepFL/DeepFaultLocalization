#ifdef SWIGJAVA
%typemap(jni) indri::api::QueryResults "jobject"
%typemap(jtype) indri::api::QueryResults "QueryResults"
%typemap(jstype) indri::api::QueryResults "QueryResults"

%{
  jobject queryresults_copy(JNIEnv *jenv, indri::api::QueryResults &results) 
    {
      jobject result = NULL;
      // get java class
      jclass clazz = jenv->FindClass("lemurproject/indri/QueryResults");
      jmethodID constructor = jenv->GetMethodID(clazz, "<init>", "()V" );
      jfieldID parseTimeField = jenv->GetFieldID(clazz, "parseTime", "D" );
      jfieldID executeTimeField = jenv->GetFieldID(clazz, "executeTime", "D" );
      jfieldID documentsTimeField = jenv->GetFieldID(clazz, "documentsTime", "D" );
      jfieldID estMatchesField = jenv->GetFieldID(clazz, "estimatedMatches", "I" );
      jfieldID resultsField = jenv->GetFieldID(clazz, "results", "[Llemurproject/indri/QueryResult;" );

      result = jenv->NewObject(clazz, constructor);
      jenv->SetDoubleField(result, parseTimeField, results.parseTime );
      jenv->SetDoubleField(result, executeTimeField, results.executeTime );
      jenv->SetDoubleField(result, documentsTimeField, results.documentsTime );
      jenv->SetIntField(result, estMatchesField, results.estimatedMatches );

      jclass qrClazz = jenv->FindClass("lemurproject/indri/QueryResult");
      jmethodID qrConstructor = jenv->GetMethodID(qrClazz, "<init>", "()V" );
      jfieldID snippetField = jenv->GetFieldID(qrClazz, "snippet", "Ljava/lang/String;" );
      jfieldID docNameField = jenv->GetFieldID(qrClazz, "documentName", "Ljava/lang/String;" );
      jfieldID docidField = jenv->GetFieldID(qrClazz, "docid", "I" );
      jfieldID scoreField = jenv->GetFieldID(qrClazz, "score", "D" );
      jfieldID beginField = jenv->GetFieldID(qrClazz, "begin", "I" );
      jfieldID endField = jenv->GetFieldID(qrClazz, "end", "I" );
      jfieldID metadataField = jenv->GetFieldID(qrClazz, "metadata", "Ljava/util/Map;" );
      jobjectArray resultArray = jenv->NewObjectArray(results.results.size(), qrClazz, NULL);
      jenv->SetObjectField(result, resultsField, resultArray);

      jclass mapClazz = jenv->FindClass("java/util/HashMap");
      jmethodID mapConstructor = jenv->GetMethodID(mapClazz, "<init>", "()V" );
      jmethodID putMethod = jenv->GetMethodID(mapClazz, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;" );

      for (jsize i = 0; i < results.results.size(); i++) {
        jobject qr = jenv->NewObject(qrClazz, qrConstructor);
        //populate fields
        jstring snippet = jenv->NewStringUTF(results.results[i].snippet.c_str());
        jstring documentName = jenv->NewStringUTF(results.results[i].documentName.c_str());;
        jenv->SetObjectField(qr, snippetField, snippet);
        jenv->SetObjectField(qr, docNameField, documentName);
        jenv->SetIntField(qr, docidField, results.results[i].docid );

        jenv->SetDoubleField(qr, scoreField, results.results[i].score );
        jenv->SetIntField(qr, beginField, results.results[i].begin );
        jenv->SetIntField(qr, endField, results.results[i].end );

        // metadata is a Map, bleah... make a hashmap and populate it.
        jobject mapObject = jenv->NewObject(mapClazz, mapConstructor);

        // copy metadata information
        for( unsigned int j = 0; j < results.results[i].metadata.size(); j++) {
          indri::api::MetadataPair& pair = results.results[i].metadata[j];
          jstring key = jenv->NewStringUTF(pair.key.c_str());
          jstring value = jenv->NewStringUTF(pair.value.c_str());
          // put it in the map
          jenv->CallObjectMethod(mapObject, putMethod, key, value);
        }
        jenv->SetObjectField(qr, metadataField, mapObject);
        jenv->SetObjectArrayElement(resultArray, i, qr);
      }
      return result;
    }
%}

%typemap(out) indri::api::QueryResults {
  $result = queryresults_copy(jenv, $1);
}

%typemap(javaout) indri::api::QueryResults {
  return $jnicall;
}

#endif

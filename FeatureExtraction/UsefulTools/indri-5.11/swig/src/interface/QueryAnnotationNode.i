
//
// QueryAnnotationNode
//
// 10 August 2004 -- tds
//
#ifdef SWIGJAVA
%typemap(jni) indri::api::QueryAnnotationNode* "jobject"
%typemap(jtype) indri::api::QueryAnnotationNode* "QueryAnnotationNode"
%typemap(jstype) indri::api::QueryAnnotationNode* "QueryAnnotationNode"

%{

  jobject java_build_queryannotationnode( indri::api::QueryAnnotationNode* in,
                                          JNIEnv* jenv,
                                          jclass qanClazz,
                                          jmethodID qanConst ) {
    jobjectArray children = jenv->NewObjectArray(in->children.size(), qanClazz, NULL);

    for( unsigned int i=0; i<in->children.size(); i++ ) {
      jobject child = java_build_queryannotationnode( in->children[i], jenv, qanClazz, qanConst );
      jenv->SetObjectArrayElement(children, i, child);
    }

    jstring name = jenv->NewStringUTF(in->name.c_str());
    jstring type = jenv->NewStringUTF(in->type.c_str());
    jstring queryText = jenv->NewStringUTF(in->queryText.c_str());

    jobject node = jenv->NewObject(qanClazz, qanConst, name, type, queryText, children);

    return node;
  }

  %}

%typemap(out) indri::api::QueryAnnotationNode* {
  jclass qanClazz = jenv->FindClass("lemurproject/indri/QueryAnnotationNode" );
  const char* signature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Llemurproject/indri/QueryAnnotationNode;)V";
  jmethodID qanConst = jenv->GetMethodID(qanClazz, "<init>", signature );

  $result = java_build_queryannotationnode( $1,
                                            jenv,
                                            qanClazz,
                                            qanConst );
}

%typemap(javaout) indri::api::QueryAnnotationNode* {
  return $jnicall;
}
#endif

#ifdef SWIGCSHARP
%typemap(ctype) indri::api::QueryAnnotationNode * "void *"
%typemap(imtype, out="IntPtr")  indri::api::QueryAnnotationNode * "HandleRef"
%typemap(cstype) indri::api::QueryAnnotationNode * "QueryAnnotationNode"
SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(QueryAnnotationNode, indri::api::QueryAnnotationNode *)
  %template (QueryAnnotationNodeVector) std::vector<indri::api::QueryAnnotationNode *>;

#endif
namespace indri {
  namespace api {
#ifdef SWIGCSHARP
    
    class QueryAnnotationNode {
    public:
        std::string name;
        std::string type;
        std::string queryText;
        std::vector<QueryAnnotationNode*> children;
    };
#endif

    class QueryAnnotation {
    public:
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
@return QueryAnnotationNodes for the query tree.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      const indri::api::QueryAnnotationNode* getQueryTree() const throw (lemur::api::Exception) ;
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
@return Annotations for the query tree.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";

      const indri::infnet::EvaluatorNode::MResults& getAnnotations() const throw (lemur::api::Exception) ;
#endif
#ifdef SWIGCSHARP
      const std::map< std::string, std::vector<indri::api::ScoredExtentResult> > & getAnnotations() const throw (lemur::api::Exception) ;
#endif
#ifdef SWIGJAVA
      %javamethodmodifiers  "
/**
@return ScoredExtentResults for the query tree.
@throws Exception if a lemur::api::Exception was thrown by the JNI library.
*/
public";
#endif

      const std::vector<indri::api::ScoredExtentResult>& getResults() const throw (lemur::api::Exception) ;
    };
  }
}

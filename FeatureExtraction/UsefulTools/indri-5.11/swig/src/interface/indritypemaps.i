#ifdef SWIGPHP5
// typemaps for indri types.
typedef long long INT64;
typedef long long UINT64;


%typemap(out) INT64, UINT64 %{

  ZVAL_LONG(return_value,$1);
  %}

%typemap(in) INT64, UINT64 %{
  convert_to_long_ex($input);
  $1 = ($1_ltype) Z_LVAL_PP($input);
  %}

%include "std_vector.i"

%typemap(in) 
  std::vector< std::string > * ,
  std::vector< std::string > &
{
  int iStatus;
  ulong iIndex;
  char *sIndex=NULL;
  zval **Data;
  $1=new std::vector< std::string >;
  convert_to_array(*$input);
  zend_hash_internal_pointer_reset((*$input)->value.ht);
  while((iStatus=zend_hash_get_current_key((*$input)->value.ht,&sIndex,&iIndex,1))!=HASH_KEY_NON_EXISTANT) {
    zend_hash_get_current_data((*$input)->value.ht,(void **) &Data);
    convert_to_string(*Data);
    $1->push_back((*Data)->value.str.val);
    zend_hash_move_forward((*$input)->value.ht);  
    if (sIndex) {
      efree(sIndex);
      sIndex=NULL;
    }
  }
  zend_hash_internal_pointer_reset((*$input)->value.ht);
}


%typemap(in) 
  std::vector< indri::api::ScoredExtentResult > *,
  std::vector< indri::api::ScoredExtentResult > &,
  const std::vector< indri::api::ScoredExtentResult > &
{
  int iStatus;
  ulong iIndex;
  char *sIndex=NULL;
  zval **Data;
  $1=new std::vector< indri::api::ScoredExtentResult >;
  convert_to_array(*$input);
  zend_hash_internal_pointer_reset((*$input)->value.ht);
  while((iStatus=zend_hash_get_current_key((*$input)->value.ht,&sIndex,&iIndex,1))!=HASH_KEY_NON_EXISTANT) {
    zend_hash_get_current_data((*$input)->value.ht,(void **) &Data);
    indri::api::ScoredExtentResult * arg1 = 0;
    SWIG_ConvertPtr(*(Data), (void **) &arg1, SWIGTYPE_p_indri__api__ScoredExtentResult, 0);
    $1->push_back(*arg1);
    zend_hash_move_forward((*$input)->value.ht);  
    if (sIndex) {
      efree(sIndex);
      sIndex=NULL;
    }
  }
  zend_hash_internal_pointer_reset((*$input)->value.ht);
}

// need std::vector<int> in and out
%typemap(in) 
  std::vector< lemur::api::DOCID_T > &,
  std::vector< lemur::api::DOCID_T > *
{
  int iStatus;
  ulong iIndex;
  char *sIndex=NULL;
  zval **Data;
  $1=new std::vector<lemur::api::DOCID_T>;
  convert_to_array(*$input);
  zend_hash_internal_pointer_reset((*$input)->value.ht);
  while((iStatus=zend_hash_get_current_key((*$input)->value.ht,&sIndex,&iIndex,1))!=HASH_KEY_NON_EXISTANT) {
    zend_hash_get_current_data((*$input)->value.ht,(void **) &Data);
    convert_to_long(*Data);
    $1->push_back((*Data)->value.lval);
    zend_hash_move_forward((*$input)->value.ht);  
    if (sIndex) {
      efree(sIndex);
      sIndex=NULL;
    }
  }
  zend_hash_internal_pointer_reset((*$input)->value.ht);
}

%typemap(in)
  USER *	SWIG_DEFAULT_TYPE
{
  $1=($type) getClass($input,$index);
}


// --- Return value processing -------------------------------------------

%typemap(out) 
  std::vector< lemur::api::DOCID_T > 
{
  std::vector< lemur::api::DOCID_T >::size_type iIndex;
  array_init(return_value);
  std::vector< lemur::api::DOCID_T > *resultobj = &result; 
  for (iIndex=0;iIndex<resultobj->size();iIndex++) 
    add_next_index_long(return_value,(*resultobj)[iIndex]);
}

%typemap(out) 
  std::vector< std::string > 
{
  std::vector< std::string >::size_type iIndex;
  array_init(return_value);
  std::vector< std::string > *resultobj = &result; 
  for (iIndex=0;iIndex<resultobj->size();iIndex++) 
    add_next_index_string(return_value,(char *) (*resultobj)[iIndex].c_str(),1);
}

%typemap(out) 
  std::vector< indri::api::ScoredExtentResult >
{
  std::vector< indri::api::ScoredExtentResult >::size_type iIndex;
  array_init(return_value);
  std::vector< indri::api::ScoredExtentResult > *resultobj = &result; 
  for (iIndex=0;iIndex<resultobj->size();iIndex++)  {
    zval *obj, *_cPtr;
    MAKE_STD_ZVAL(obj);
    zend_class_entry *ce = zend_fetch_class("ScoredExtentResult",
  sizeof("ScoredExtentResult") - 1, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);
    MAKE_STD_ZVAL(_cPtr);
    indri::api::ScoredExtentResult *r = new indri::api::ScoredExtentResult((*resultobj)[iIndex]);
    SWIG_SetPointerZval(obj, (void *)r, SWIGTYPE_p_indri__api__ScoredExtentResult, 1);
    *_cPtr = *obj;
    INIT_ZVAL(*obj);
    object_init_ex(obj,ce);
    add_property_zval(obj,"_cPtr",_cPtr);
    add_property_double(obj,"score",r->score);
    add_property_long(obj,"document",r->document);
    add_property_long(obj,"begin",r->begin);
    add_property_long(obj,"end",r->end);
    add_property_long(obj,"number", r->number);
    add_property_long(obj,"ordinal", r->ordinal);
    add_property_long(obj,"parentOrdinal", r->parentOrdinal);
    add_next_index_zval(return_value, obj);
  }
}

%typemap(out) 
   std::vector< indri::api::ParsedDocument * >
{
  std::vector< indri::api::ParsedDocument * >::size_type iIndex;
  array_init(return_value);
  std::vector< indri::api::ParsedDocument * > *resultobj = &result; 
    zend_class_entry *ce = zend_fetch_class("ParsedDocument",
  sizeof("ParsedDocument") - 1, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);
    zend_class_entry *tce = zend_fetch_class("TermExtent",
  sizeof("TermExtent") - 1, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);

  for (iIndex=0;iIndex<resultobj->size();iIndex++)  {
    zval *obj, *_cPtr;
    MAKE_STD_ZVAL(obj);
    MAKE_STD_ZVAL(_cPtr);
    indri::api::ParsedDocument *r = (*resultobj)[iIndex];
    SWIG_SetPointerZval(obj, (void *)r, SWIGTYPE_p_indri__api__ParsedDocument, 1);
    *_cPtr = *obj;
    INIT_ZVAL(*obj);
    object_init_ex(obj,ce);
    add_property_zval(obj,"_cPtr",_cPtr);
    // ignore other elements.
    add_property_string(obj,"text",(char *)(r->text), 1);
    add_property_long(obj,"textLength",r->textLength);
    // content
    add_property_string(obj,"content",(char *)(r->content), 1);
    add_property_long(obj,"contentLength",r->contentLength);

    // positions
    // must wrap TermExtent
    zval *positions, *pos, *_ptr;
    MAKE_STD_ZVAL(positions);
    array_init(positions);    
    for (indri::utility::greedy_vector<indri::parse::TermExtent>::iterator iter = r->positions.begin();
	 iter != r->positions.end(); iter++) {
      MAKE_STD_ZVAL(pos);
      MAKE_STD_ZVAL(_ptr);
      indri::parse::TermExtent *t = new indri::parse::TermExtent;
      t->begin = iter->begin;
      t->end = iter->end;
      SWIG_SetPointerZval(pos, (void *)t, SWIGTYPE_p_indri__parse__TermExtent, 1);
      *_ptr = *pos;
      INIT_ZVAL(*pos);
      object_init_ex(pos,tce);
      add_property_zval(pos,"_cPtr",_ptr);
      add_property_long(pos,"begin", iter->begin);
      add_property_long(pos,"end",iter->end);
      add_next_index_zval(positions, pos);
    }
    add_property_zval(obj,"positions",positions);	
    // metadata
    // must wrap MetadataPair 
    zval *pairs;
    MAKE_STD_ZVAL(pairs);
    array_init(pairs);    
      // copy metadata information
    for (indri::utility::greedy_vector<indri::parse::MetadataPair>::iterator iter = r->metadata.begin();
	 iter != r->metadata.end(); iter++) {
      add_assoc_string(pairs, (char *)iter->key, (char *)iter->value, 1);
    }

    add_property_zval(obj,"metadata",pairs);
    add_next_index_zval(return_value, obj);
  }
}

%typemap(out) 
     std::vector< indri::api::ScoredExtentResult > &,
     std::vector< indri::api::ScoredExtentResult > *
{
  std::vector< indri::api::ScoredExtentResult >::size_type iIndex;
  array_init(return_value);
  std::vector< indri::api::ScoredExtentResult > *resultobj = result; 
    zend_class_entry *ce = zend_fetch_class("ScoredExtentResult",
  sizeof("ScoredExtentResult") - 1, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);

  for (iIndex=0;iIndex<resultobj->size();iIndex++)  {
    zval *obj, *_cPtr, *retval;
    MAKE_STD_ZVAL(obj);
    MAKE_STD_ZVAL(_cPtr);
    indri::api::ScoredExtentResult *r = new indri::api::ScoredExtentResult((*resultobj)[iIndex]);
    SWIG_SetPointerZval(obj, (void *)r, SWIGTYPE_p_indri__api__ScoredExtentResult, 1);
    *_cPtr = *obj;
    INIT_ZVAL(*obj);
    object_init_ex(obj,ce);
    add_property_zval(obj,"_cPtr",_cPtr);
    add_property_double(obj,"score",r->score);
    add_property_long(obj,"document",r->document);
    add_property_long(obj,"begin",r->begin);
    add_property_long(obj,"end",r->end);
    add_property_long(obj,"number", r->number);
    add_property_long(obj,"ordinal", r->ordinal);
    add_property_long(obj,"parentOrdinal", r->parentOrdinal);
    add_next_index_zval(return_value, obj);
  }
}

%typemap(out) 
  std::vector< std::string > *,
  std::vector< std::string > &
{
  std::vector< std::string >::size_type iIndex;
  array_init($return_value);
  std::vector< std::string > * resultobj = &result; 
  for (iIndex=0;iIndex<resultobj->size();iIndex++) 
    add_next_index_string($return_value,(char *) (*resultobj)[iIndex].c_str(),1);
}

%typemap(out) 
  indri::infnet::EvaluatorNode::MResults &
{
  array_init(return_value);
  const indri::infnet::EvaluatorNode::MResults & matches = *result; 
  indri::infnet::EvaluatorNode::MResults::iterator iter;
  std::vector< indri::api::ScoredExtentResult >::size_type iIndex;
    zend_class_entry *ce = zend_fetch_class("ScoredExtentResult",
  sizeof("ScoredExtentResult") - 1, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);

  for( iter = $1->begin(); iter != $1->end(); iter++ ) {
    zval *seRes;
    MAKE_STD_ZVAL(seRes);
    array_init(seRes);
    std::vector<indri::api::ScoredExtentResult>& vec = iter->second;
    char *key = (char *)iter->first.c_str();
    for (iIndex=0;iIndex<vec.size();iIndex++)  {
      zval *obj, *_cPtr;
      MAKE_STD_ZVAL(obj);
      MAKE_STD_ZVAL(_cPtr);
      indri::api::ScoredExtentResult *r = new indri::api::ScoredExtentResult(vec[iIndex]);
      SWIG_SetPointerZval(obj, (void *)r, SWIGTYPE_p_indri__api__ScoredExtentResult, 1);
      *_cPtr = *obj;
      INIT_ZVAL(*obj);
      object_init_ex(obj,ce);
      add_property_zval(obj,"_cPtr",_cPtr);
      add_property_double(obj,"score",r->score);
      add_property_long(obj,"document",r->document);
      add_property_long(obj,"begin",r->begin);
      add_property_long(obj,"end",r->end);
      add_property_long(obj,"number", r->number);
      add_property_long(obj,"ordinal", r->ordinal);
      add_property_long(obj,"parentOrdinal", r->parentOrdinal);
      add_next_index_zval(seRes, obj);
    }
    add_assoc_zval(return_value, key, seRes);
  }
}


%typemap(out) indri::api::QueryAnnotation * {
    zval *retval = 0, *_cPtr;
    zend_class_entry *ce = zend_fetch_class("QueryAnnotation",
  sizeof("QueryAnnotation") - 1, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);

    MAKE_STD_ZVAL(_cPtr);
    MAKE_STD_ZVAL(retval);
    SWIG_SetPointerZval(retval, $1,SWIGTYPE_p_indri__api__QueryAnnotation, 1);
    *_cPtr = *retval;
    INIT_ZVAL(*retval);
    object_init_ex(retval,ce);
    add_property_zval(retval,"_cPtr",_cPtr);
    *($result) = *retval;
}

%typemap(out) indri::api::QueryAnnotationNode * {
  zval *tmp = php_makeQueryAnnotationNode($1);
  *($result) =  *tmp;
  return;
}

// need a method to make zvals for each child.
%wrapper %{

  zval *php_makeQueryAnnotationNode(indri::api::QueryAnnotationNode *inNode) {
    zval *retval = 0, *_cPtr;
    zend_class_entry *ce = zend_fetch_class("QueryAnnotationNode",
  sizeof("QueryAnnotationNode") - 1, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);

    MAKE_STD_ZVAL(_cPtr);
    MAKE_STD_ZVAL(retval);
    SWIG_SetPointerZval(retval, (void *)inNode,SWIGTYPE_p_indri__api__QueryAnnotationNode, 0);
    *_cPtr = *retval;
    INIT_ZVAL(*retval);
    object_init_ex(retval,ce);
    add_property_zval(retval,"_cPtr",_cPtr);
    // don't deref NULL
    if (inNode) {
      // name
      // type
      // query text
      // children
      add_property_string(retval, "name", (char *)inNode->name.c_str(), 1);
      add_property_string(retval, "type",  (char *)inNode->type.c_str(), 1);
      add_property_string(retval, "queryText",  (char *)inNode->queryText.c_str(), 1);
      zval *children;
      MAKE_STD_ZVAL(children);
      array_init(children);
      add_property_zval(retval, "children", children);
      for( unsigned int i=0; i<inNode->children.size(); i++ ) {
        zval *child;
        child = php_makeQueryAnnotationNode(inNode->children[i]);
        add_next_index_zval(children, child);
      }
    }
    // need the _cPtr, etc.
    return retval;
  }
  %}

// --- Argument clean up -------------------------------------------------

%typemap(freearg) 
  std::vector< std::string > *, 
 std::vector< std::string > &, 
 std::vector< std::string >, 
 std::vector< indri::api::ScoredExtentResult > *, 
 std::vector< indri::api::ParsedDocument * > *, 
 std::vector< lemur::api::DOCID_T > *, 
 std::vector< indri::api::ScoredExtentResult > &, 
 std::vector< lemur::api::DOCID_T > &, 
 std::vector< indri::api::ScoredExtentResult >, 
 std::vector< indri::api::ParsedDocument * >, 
 std::vector< lemur::api::DOCID_T >
{
// freearg typemap
  delete $1;
}
#endif

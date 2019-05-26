<?php

/* PHP Proxy Classes */
class TermExtent {
	public $_cPtr=null;

	function __set($var,$value) {
		if ($var == 'end') return TermExtent_end_set($this->_cPtr,$value);
		if ($var == 'begin') return TermExtent_begin_set($this->_cPtr,$value);
	}

	function __isset($var) {
		return function_exists('TermExtent_'.$var.'_set');
	}

	function __get($var) {
		if ($var == 'end') return TermExtent_end_get($this->_cPtr);
		if ($var == 'begin') return TermExtent_begin_get($this->_cPtr);
		return null;
	}

	function __construct() {
		$this->_cPtr=new_TermExtent();
	}
}

class ScoredExtentResult {
	public $_cPtr=null;

	function __construct() {
		$this->_cPtr=new_ScoredExtentResult();
	}
}

class ParsedDocument {
	public $_cPtr=null;

	function getContent() {
		return ParsedDocument_getContent($this->_cPtr);
	}

	function __construct() {
		$this->_cPtr=new_ParsedDocument();
	}
}

class QueryAnnotationNode {
	public $_cPtr=null;

	function __construct() {
		$this->_cPtr=new_QueryAnnotationNode();
	}
}

class QueryAnnotation {
	public $_cPtr=null;
	function __construct($h) {
		$this->_cPtr=$h;
	}

	function getQueryTree() {
		$r=QueryAnnotation_getQueryTree($this->_cPtr);
		return is_resource($r) ? new QueryAnnotationNode($r) : $r;
	}

	function getAnnotations() {
		return QueryAnnotation_getAnnotations($this->_cPtr);
	}

	function getResults() {
		return QueryAnnotation_getResults($this->_cPtr);
	}
}

class QueryEnvironment {
	public $_cPtr=null;

	function addServer($hostname) {
		QueryEnvironment_addServer($this->_cPtr,$hostname);
	}

	function addIndex($pathname) {
		QueryEnvironment_addIndex($this->_cPtr,$pathname);
	}

	function close() {
		QueryEnvironment_close($this->_cPtr);
	}

	function setMemory($memory) {
		QueryEnvironment_setMemory($this->_cPtr,$memory);
	}

	function setScoringRules($rules) {
		QueryEnvironment_setScoringRules($this->_cPtr,$rules);
	}

	function setStopwords($stopwords) {
		QueryEnvironment_setStopwords($this->_cPtr,$stopwords);
	}

	function runQuery($query,$resultsRequested) {
		return QueryEnvironment_runQuery($this->_cPtr,$query,$resultsRequested);
	}

	function runQuerydocset($query,$documentSet,$resultsRequested) {
		return QueryEnvironment_runQuerydocset($this->_cPtr,$query,$documentSet,$resultsRequested);
	}

	function runAnnotatedQuery($query,$resultsRequested) {
		$r=QueryEnvironment_runAnnotatedQuery($this->_cPtr,$query,$resultsRequested);
		return is_resource($r) ? new QueryAnnotation($r) : $r;
	}

	function runAnnotatedQuerydocset($query,$documentSet,$resultsRequested) {
		$r=QueryEnvironment_runAnnotatedQuerydocset($this->_cPtr,$query,$documentSet,$resultsRequested);
		return is_resource($r) ? new QueryAnnotation($r) : $r;
	}

	function documentsdocids($documentIDs) {
		return QueryEnvironment_documentsdocids($this->_cPtr,$documentIDs);
	}

	function documents($results) {
		return QueryEnvironment_documents($this->_cPtr,$results);
	}

	function documentMetadatadocids($documentIDs,$attributeName) {
		return QueryEnvironment_documentMetadatadocids($this->_cPtr,$documentIDs,$attributeName);
	}

	function documentMetadata($documentIDs,$attributeName) {
		return QueryEnvironment_documentMetadata($this->_cPtr,$documentIDs,$attributeName);
	}

	function documentIDsFromMetadata($attributeName,$attributeValue) {
		return QueryEnvironment_documentIDsFromMetadata($this->_cPtr,$attributeName,$attributeValue);
	}

	function termCount() {
		return QueryEnvironment_termCount($this->_cPtr);
	}

	function onetermCount($term) {
		return QueryEnvironment_onetermCount($this->_cPtr,$term);
	}

	function stemCount($term) {
		return QueryEnvironment_stemCount($this->_cPtr,$term);
	}

	function termFieldCount($term,$field) {
		return QueryEnvironment_termFieldCount($this->_cPtr,$term,$field);
	}

	function stemFieldCount($term,$field) {
		return QueryEnvironment_stemFieldCount($this->_cPtr,$term,$field);
	}

	function fieldList() {
		return QueryEnvironment_fieldList($this->_cPtr);
	}

	function documentCount() {
		return QueryEnvironment_documentCount($this->_cPtr);
	}

	function onedocumentCount($term) {
		return QueryEnvironment_onedocumentCount($this->_cPtr,$term);
	}

	function expressionCount($expression,$queryType=null) {
		switch (func_num_args()) {
		case 1: $r=QueryEnvironment_expressionCount($this->_cPtr,$expression); break;
		default: $r=QueryEnvironment_expressionCount($this->_cPtr,$expression,$queryType);
		}
		return $r;
	}

	function expressionList($expression,$queryType=null) {
		switch (func_num_args()) {
		case 1: $r=QueryEnvironment_expressionList($this->_cPtr,$expression); break;
		default: $r=QueryEnvironment_expressionList($this->_cPtr,$expression,$queryType);
		}
		return $r;
	}

	function documentLength($documentID) {
		return QueryEnvironment_documentLength($this->_cPtr,$documentID);
	}

	function __construct() {
		$this->_cPtr=new_QueryEnvironment();
	}
}

class Parameters {
	public $_cPtr=null;

	function __construct() {
		$this->_cPtr=new_Parameters();
	}

	function set($value) {
		Parameters_set($this->_cPtr,$value);
	}

	function get_bool($name,$def) {
		return Parameters_get_bool($this->_cPtr,$name,$def);
	}

	function get_int($name,$def) {
		return Parameters_get_int($this->_cPtr,$name,$def);
	}

	function get_double($name,$def) {
		return Parameters_get_double($this->_cPtr,$name,$def);
	}

	function get_INT64($name,$def) {
		return Parameters_get_INT64($this->_cPtr,$name,$def);
	}

	function get_string($name,$def) {
		return Parameters_get_string($this->_cPtr,$name,$def);
	}

	function remove($path) {
		Parameters_remove($this->_cPtr,$path);
	}

	function set_bool($name,$value) {
		Parameters_set_bool($this->_cPtr,$name,$value);
	}

	function set_string($name,$value) {
		Parameters_set_string($this->_cPtr,$name,$value);
	}

	function set_int($name,$value) {
		Parameters_set_int($this->_cPtr,$name,$value);
	}

	function set_UINT64($name,$value) {
		Parameters_set_UINT64($this->_cPtr,$name,$value);
	}

	function set_double($name,$value) {
		Parameters_set_double($this->_cPtr,$name,$value);
	}

	function clear() {
		Parameters_clear($this->_cPtr);
	}

	function size() {
		return Parameters_size($this->_cPtr);
	}

	function exists($name) {
		return Parameters_exists($this->_cPtr,$name);
	}

	function load($text) {
		Parameters_load($this->_cPtr,$text);
	}
}

abstract class QueryExpander {
	public $_cPtr=null;
	function __construct($h) {
		$this->_cPtr=$h;
	}

	function runExpandedQuery($originalQuery,$resultsRequested,$verbose=false) {
		return QueryExpander_runExpandedQuery($this->_cPtr,$originalQuery,$resultsRequested,$verbose);
	}

	function expand($originalQuery,$results) {
		return QueryExpander_expand($this->_cPtr,$originalQuery,$results);
	}
}

class RMExpander extends QueryExpander {
	public $_cPtr=null;

	function __construct($env,$param) {
		$this->_cPtr=new_RMExpander($env,$param);
	}

	function expand($originalQuery,$results) {
		return RMExpander_expand($this->_cPtr,$originalQuery,$results);
	}
}

class PonteExpander extends QueryExpander {
	public $_cPtr=null;

	function __construct($env,$param) {
		$this->_cPtr=new_PonteExpander($env,$param);
	}

	function expand($originalQuery,$results) {
		return PonteExpander_expand($this->_cPtr,$originalQuery,$results);
	}
}


?>

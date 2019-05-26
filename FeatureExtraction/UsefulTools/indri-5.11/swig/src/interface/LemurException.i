#ifdef SWIGJAVA
//default exception handler.
#if SWIG_VERSION >= 0x010325
%exception {
  try {
    $action
      } catch( lemur::api::Exception& e ) {
        SWIG_exception(SWIG_RuntimeError, e.what().c_str() );
        // control does not leave method when thrown. (fixed in 1.3.25
        // return $null;
      }
}
#else
%exception {
  try {
    $action
      } catch( lemur::api::Exception& e ) {
        SWIG_exception(SWIG_RuntimeError, e.what().c_str() );
        // control does not leave method when thrown. (fixed in 1.3.25
        return $null;
      }
}
#endif

%define javaSetEx(method) 
  %javaexception("java.lang.Exception") method{
  try {
    $action
      } catch( lemur::api::Exception& e ) {
        // Map all Lemur exceptions to Exception.
        jclass excep = jenv->FindClass("java/lang/Exception");
        if (excep)
          jenv->ThrowNew(excep, $1.what().c_str());
        return $null;
      }
}
%enddef
%typemap(throws, throws="java.lang.Exception") lemur::api::Exception {
  jclass excep = jenv->FindClass("java/lang/Exception");
  if (excep)
    jenv->ThrowNew(excep, $1.what().c_str());
  return $null;
}

#endif
#ifdef SWIGPHP5
// need separate map for individual methods.
%define setEx(method) 
  %exception method {
  try {
    $action
      } catch( lemur::api::Exception& e ) {
        //    SWIG_exception( SWIG_RuntimeError, e.what().c_str() );
        // get a warning message rather than abort the script.
        //      zend_error(E_WARNING, e.what().c_str());
        RETURN_NULL() ;
      }
}
%enddef
#endif
#ifdef SWIGCSHARP
// csharp exception type maps
%typemap(throws, canthrow=1) lemur::api::Exception
%{ 
  SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, ("C++ #$1_type exception: " + $1.what()).c_str());
  return $null; %}

#endif

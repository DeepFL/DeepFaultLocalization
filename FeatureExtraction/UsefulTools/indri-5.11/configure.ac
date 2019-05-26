dnl Process this file with autoconf to produce a configure script.
AC_INIT([Indri],[5.11],[project@lemurproject.org])
CFLAGS=${CFLAGS:-"-g -O3"}
CXXFLAGS=${CXXFLAGS:-"-g -O3"}
AC_CONFIG_SRCDIR(indri-VS2012.sln)
AC_CONFIG_FILES(MakeDefns Makefile.app)
AC_CONFIG_FILES([site-search/build.param site-search/crawl-index site-search/seeds.txt site-search/excluded_hosts], [test -f site-search/crawl-index && chmod +x site-search/crawl-index])
dnl To use a header file instead of command line -D defs, use
dnl AC_CONFIG_HEADERS(config.h)
dnl canonical host
AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LEX
AC_PROG_INSTALL
AC_PATH_PROGS(AR, ar gar)
if test -z "$AR" ; then
   AC_MSG_ERROR([cannot find the program ar on your path. Add AR=/path/to/ar to your configure command line])
fi

dnl We don't want gcc < 3.2
dnl 3.3.1 has optimization issue (possibly .2 and .3...
AC_MSG_CHECKING([for a supported version of gcc])

gcc_version=`${CC} -dumpversion`
GCC_33=0
case "${gcc_version}" in
     2.*)
	AC_MSG_RESULT([no (${gcc_version})])
	AC_MSG_ERROR([please use a recent (>= gcc-3.2) version of gcc])
	;;
     3.0*)
	AC_MSG_RESULT([no (${gcc_version})])
	AC_MSG_ERROR([please use a recent (>= gcc-3.2) version of gcc])
	;;
     3.1*)
	AC_MSG_RESULT([no (${gcc_version})])
	AC_MSG_ERROR([please use a recent (>= gcc-3.2) version of gcc])
	;;
     3.3*)
	AC_MSG_RESULT([buggy (${gcc_version})])
        GCC_33=1
	;;
     4.2.1*)
        AC_MSG_RESULT([clang (${gcc_version})])
        ;;
     *)
	AC_MSG_RESULT([ok (${gcc_version})])
	;;
esac
AC_SUBST(GCC_33)
AC_SUBST(SHARED)
INDRI_STANDALONE=1
AC_DEFINE(INDRI_STANDALONE, 1, [standalone indri])
AC_SUBST(INDRI_STANDALONE)
dnl Checks for libraries.
AC_CHECK_LIB(m, log)
AC_CHECK_LIB(pthread, pthread_create)

case $host in
   *-*-solaris*) 
     AC_CHECK_LIB(nsl, gethostbyname)
     AC_CHECK_LIB(socket, socket)
     SHARED="-shared -fPIC"
     PHP_SHARED="-G"
     PHP_SO=".so"
     JAVA_SHARED="-G"
     JAVA_SO=".so"
     CSHARP_SHARED="-G"
     CSHARP_SO=".so"
     ;;
   *linux*) 
     SHARED="-shared -fPIC"
     PHP_SHARED="-shared"
     PHP_SO=".so"
     JAVA_SHARED="-shared"
     JAVA_SO=".so"
     CSHARP_SHARED="-shared"
     CSHARP_SO=".so"
     ;;
   *-*-darwin1@<:@3-9@:>@*)
     SHARED=

# have to fix this for php/java bits.
     PHP_SHARED=" -stdlib=libstdc++ -bundle  -undefined suppress -flat_namespace"
     PHP_SO=".so"
     JAVA_SHARED=" -stdlib=libstdc++ -dynamiclib -framework JavaVM"
     JAVA_SO=".jnilib"
     CSHARP_SHARED=" -stdlib=libstdc++ -bundle  -undefined suppress -flat_namespace"
     CSHARP_SO=".so"
     CPPFLAGS+=" -stdlib=libstdc++"
     LDFLAGS+=" -stdlib=libstdc++"
     ;;
   *-*-darwin*)
     SHARED=
#SWIG_SHARED = -bundle -bundle_loader /var/tmp/indri-2.2/tmp/php-4.4.2/sapi/cli/php
# have to fix this for php/java bits.
     PHP_SHARED="-bundle  -undefined suppress -flat_namespace"
     PHP_SO=".so"
     JAVA_SHARED="-dynamiclib -framework JavaVM"
     JAVA_SO=".jnilib"
     CSHARP_SHARED="-bundle  -undefined suppress -flat_namespace"
     CSHARP_SO=".so"
     ;;
   *);;
esac

AC_CHECK_LIB(z, deflate)
AC_CHECK_LIB(iberty, mkstemps)

dnl Checks for header files.
AC_DEFUN([AC_CXX_NAMESPACES],
	 [AC_MSG_CHECKING([whether C++ namespaces are supported])
	  AC_LANG_PUSH([C++])
	  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[namespace Outer { namespace Inner { int i = 0; }}]], [[using namespace Outer::Inner; return i;]])],[ac_cv_cxx_namespaces=yes],[ac_cv_cxx_namespaces=no])
          AC_LANG_POP
	  AC_MSG_RESULT($ac_cv_cxx_namespaces)
	  if test "$ac_cv_cxx_namespaces" = yes; then
		AC_DEFINE(HAVE_NAMESPACES,,[define if the compiler implements namespaces])
          fi])
			      
AC_DEFUN([AC_CXX_ISNAN_IN_NAMESPACE_STD],
	[AC_REQUIRE([AC_CXX_NAMESPACES])
	AC_MSG_CHECKING([whether isnan is in namespace std])
	 AC_LANG_PUSH([C++])
	 AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <cmath>
	   namespace lemur { int isnan(float x){ return std::isnan(x); } };]], [[
	   using namespace lemur; float x = 1.0; lemur::isnan(x); return 0;]])],[ac_cv_cxx_isnan_std=yes],[ac_cv_cxx_isnan_std=no])
         AC_LANG_POP
 	  AC_MSG_RESULT($ac_cv_cxx_isnan_std)
         if test "$ac_cv_cxx_isnan_std" = yes; then
	    AC_DEFINE(ISNAN_IN_NAMESPACE_STD,,[define if the compiler has isnan function in namespace std])
	 fi])

AC_DEFUN([AC_CXX_ISNAN_IN_NAMESPACE_GNU_CXX],
	[AC_REQUIRE([AC_CXX_NAMESPACES])
	AC_MSG_CHECKING([whether isnan is in namespace __gnu_cxx])
	 AC_LANG_PUSH([C++])
	 AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <cmath>
	   namespace lemur { int isnan(float x){ return __gnu_cxx::isnan(x); } };]], [[
	   using namespace lemur; float x = 1.0; lemur::isnan(x); return 0;]])],[ac_cv_cxx_isnan_gnu_cxx=yes],[ac_cv_cxx_isnan_gnu_cxx=no])
         AC_LANG_POP
 	  AC_MSG_RESULT($ac_cv_cxx_isnan_gnu_cxx)
         if test "$ac_cv_cxx_isnan_gnu_cxx" = yes; then
	    AC_DEFINE(ISNAN_IN_NAMESPACE_GNU_CXX,,[define if the compiler has isnan function in namespace __gnu_cxx])
	 fi])

AC_CXX_ISNAN_IN_NAMESPACE_STD	       
AC_CXX_ISNAN_IN_NAMESPACE_GNU_CXX
dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_CHECK_SIZEOF(int)
dnl AC_CHECK_SIZEOF(void *)
dnl AC_CHECK_SIZEOF(unsigned long)
dnl AC_CHECK_SIZEOF(unsigned long long)
AC_C_BIGENDIAN
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

dnl ########################################################################
dnl see if using GNU glibc++, because then we have atomic integer operations

    P_HAS_ATOMIC_INT=0
    P_NEEDS_GNU_CXX_NAMESPACE=0
    AC_LANG_PUSH([C++])
    AC_CHECK_HEADERS([bits/atomicity.h], P_HAS_ATOMIC_INT=1)
    AC_CHECK_HEADERS([ext/atomicity.h], P_NEEDS_GNU_CXX_NAMESPACE=1)
    AC_LANG_POP
# should abort if neither is found.
    AC_LANG_PUSH([C++])
   if test ${P_HAS_ATOMIC_INT} = 1 ; then
     AC_MSG_CHECKING(if __atomic_add is in __gnu_cxx namespace)
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <bits/atomicity.h>]], [[_Atomic_word val; __gnu_cxx::__atomic_add(&val, 1)]])],[P_NEEDS_GNU_CXX_NAMESPACE=1],[])
     if test ${P_NEEDS_GNU_CXX_NAMESPACE} = 1 ; then
	AC_MSG_RESULT(yes)
        AC_DEFINE(P_NEEDS_GNU_CXX_NAMESPACE, 1, [define if __atomic_add is in __gnu_cxx namespace])
     else
	AC_MSG_RESULT(no)
        AC_DEFINE(P_NEEDS_GNU_CXX_NAMESPACE, 0, [define if __atomic_add is in __gnu_cxx namespace])
     fi
   else
    if test ${P_NEEDS_GNU_CXX_NAMESPACE} = 1 ; then
        AC_DEFINE(P_NEEDS_GNU_CXX_NAMESPACE, 1, [define if __atomic_add is in __gnu_cxx namespace])
    else
     AC_DEFINE(P_NEEDS_GNU_CXX_NAMESPACE, 0, [define if __atomic_add is in __gnu_cxx namespace])
    fi
   fi
   AC_LANG_POP

dnl Checks for library functions.
AC_CHECK_FUNCS(mkstemp)
AC_CHECK_FUNCS(mkstemps)

dnl check for optional components.
WITH_SWIG=0
AC_ARG_ENABLE(php,
	    AS_HELP_STRING(--enable-php,Enable php shared library generation))
if test "$enable_php" = "yes" ; then
  WITH_SWIG=1
  WITH_PHP=1
else
  WITH_PHP=0
fi
AC_ARG_ENABLE(csharp,
	    AS_HELP_STRING(--enable-csharp,Enable csharp shared library generation))
if test "$enable_csharp" = "yes" ; then
   WITH_SWIG=1
   WITH_CSHARP=1
else
   WITH_CSHARP=0
fi
AC_ARG_ENABLE(java,
	    AS_HELP_STRING(--enable-java,Enable java shared library generation))
if test "$enable_java" = "yes" ; then
  WITH_SWIG=1
  WITH_JAVA=1
else
  WITH_JAVA=0
fi

AC_ARG_ENABLE(assert,
	    AS_HELP_STRING(--enable-assert,Enable assert statements in code),
			   [if test "${enable_assert}" = "no" ; then
			    AC_DEFINE(NDEBUG, 1, [Turn off assert statements])
			   fi],
			   AC_DEFINE(NDEBUG, 1, [Turn off assert statements]))

dnl Look for java
if test "$enable_java" = "yes" ; then
   AC_ARG_WITH(javahome, [  --with-javahome=path       Set location of JAVAHOME],[JAVAHOME="$withval"; JAVACBIN=$JAVAHOME/bin/javac; JAVAINCDIR=$JAVAHOME/include; JAR=$JAVAHOME/bin/jar; JAVADOC=$JAVAHOME/bin/javadoc], [JAVAHOME=])
   
   if test -z "$JAVACBIN" ; then
     AC_PATH_PROGS(JAVAC, javac)
     AC_PATH_PROGS(JAR, jar)
     AC_PATH_PROGS(JAVADOC, javadoc)
   else
     JAVAC="$JAVACBIN"
   fi
   AC_MSG_CHECKING(for java include file jni.h)
   if test -z "$JAVAINCDIR"; then
     JAVAINCDIR="/usr/j2sdk*/include /usr/local/j2sdk*/include /usr/jdk*/include /usr/local/jdk*/include /opt/j2sdk*/include /opt/jdk*/include /usr/java/include /usr/java/j2sdk*/include /usr/java/jdk*/include /usr/local/java/include /opt/java/include /usr/include/java /usr/local/include/java /usr/lib/java/include /System/Library/Frameworks/JavaVM.framework/Headers/"
   fi
   JAVAINC=""
   for d in $JAVAINCDIR ; do
     if test -r $d/jni.h ; then
       AC_MSG_RESULT($d)
       JAVAINCDIR=$d
       JAVAINC="-I$d"
       break
     fi
   done
dnl only disable if no java includes or no javac
   if test "$JAVAINC" = "" -o -z "$JAVAC" ; then
      AC_MSG_RESULT(not found disabling swig java wrappers)
      WITH_JAVA=0
      enable_java=no
   else
dnl now look for <arch>/jni_md.h
      AC_MSG_CHECKING(for java include file jni_md.h)
      JAVAMDDIR=`find $JAVAINCDIR -follow -name jni_md.h -print | head -1`
      if test "$JAVAMDDIR" = "" ; then
	 AC_MSG_RESULT(not found)
	 WITH_JAVA=0
         enable_java=no
      else
	 JAVAMDDIR=`dirname $JAVAMDDIR`
	 JAVAINC="${JAVAINC} -I$JAVAMDDIR"
	 AC_MSG_RESULT($JAVAMDDIR)
	 WITH_JAVA=1
      fi
   fi
else
   JAVAC=
   JAR=
   JAVAINC=
   JAVAHOME=
   WITH_JAVA=0
fi
AC_SUBST(JAVAC)
AC_SUBST(JAR)
AC_SUBST(JAVAINC)
AC_SUBST(JAVADOC)
AC_SUBST(JAVAHOME)
AC_SUBST(WITH_JAVA)

dnl Look for php
if test "$enable_php" = "yes" ; then
   AC_ARG_WITH(php-config, [  --with-php-config=path       Set location of php-config],[PHPCONFIG="$withval"], [PHPCONFIG=])
   if test -z "$PHPCONFIG" ; then
      AC_PATH_PROG(PHPCONFIG, php-config)
   fi
   if test "$PHPCONFIG" = ""; then
      AC_MSG_WARN($PHPCONFIG not found disabling swig php wrappers)
      WITH_PHP=0
      enable_php=no
      PHPINCLUDE=
   else
      AC_MSG_CHECKING(for php includes)
      PHPINCLUDE=`$PHPCONFIG --includes`
      AC_MSG_RESULT(found)
      WITH_PHP=1
   fi
else
   PHPINCLUDE=
   WITH_PHP=0
fi
AC_SUBST(PHPINCLUDE)
AC_SUBST(WITH_PHP)

dnl Look for mono csharp compiler
if test "$enable_csharp" = yes ; then
  if test -z "$MCS" ; then
     AC_PATH_PROG(MCS, mcs)
  fi
  if test -z "$MCS" ; then
     AC_MSG_WARN([cannot find the program mcs on your path. Add MCS=/path/to/mcs to your configure command line])
     MCS="echo install mcs"
  fi
fi

dnl Look for swig
SWIG=
if test $WITH_SWIG = 1 ; then
  AC_ARG_WITH(swig, [  --with-swig=path       Set location of swig executable],[SWIG="$withval"])
  if test -z "$SWIG" ; then
     AC_PATH_PROGS(SWIG, swig)
  fi
fi

dnl check for antlr
NEED_ANTLR=1
AC_ARG_WITH(antlr, [  --with-antlr=path       Set location of antlr install],
                   [CPPFLAGS="-I$withval/include $CPPFLAGS"; LDFLAGS="-L$withval/lib $LDFLAGS"; LIBS="-lantlr $LIBS"; NEED_ANTLR=0])
dnl only accept via with-antlr
dnl AC_LANG_PUSH([C++])
dnl AC_CHECK_HEADER([antlr/ANTLRException.hpp], [LIBS="-lantlr $LIBS"; NEED_ANTLR=0])
dnl AC_LANG_POP()
AC_SUBST(NEED_ANTLR)
AC_SUBST(SWIG)
AC_SUBST(WITH_CSHARP)
AC_SUBST(MCS)
AC_SUBST(WITH_SWIG)
AC_SUBST(PHP_SHARED)
AC_SUBST(JAVA_SHARED)
AC_SUBST(CSHARP_SHARED)
AC_SUBST(PHP_SO)
AC_SUBST(JAVA_SO)
AC_SUBST(CSHARP_SO)
AC_SUBST(CSHARP_SO)

dnl site search parameters
SITESEED="unknownhost"
AC_ARG_WITH(site-seed, [  --with-site-seed=hostname       Set the seed host for site-search],[SITESEED="$withval"])
AC_SUBST(SITESEED)

AC_OUTPUT

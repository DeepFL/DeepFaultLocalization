include ../../MakeDefns
## GCC 4.0 misoptimizes the wrappers with strict-aliasing
PACKAGE = indri_jni
SWIG_DEFS=
ifeq ($(INDRI_STANDALONE), 1)
  SWIG_DEFS=-DINDRI_STANDALONE
endif
CXXFLAGS += -fno-strict-aliasing
INCPATH=-I../../include $(patsubst %, -I../../contrib/%/include, $(DEPENDENCIES))
LIBPATH=-L../../obj $(patsubst %,-L../../contrib/%/obj, $(DEPENDENCIES))
LIBS=-lindri $(patsubst %,-l%, $(DEPENDENCIES))
#LIBDEPS=$(wildcard ../../obj/*.o) $(wildcard $(patsubst %, ../../contrib/%/obj/*.o, $(DEPENDENCIES)))

LIBDEPS=../../obj/libindri.a
LDFLAGS = $(LIBPATH) $(LIBS) $(CPPLDFLAGS)
INDRI_SRC=interface/indri.i
PHPSRC=$(SRC)

OBJ=../obj
OUTDIR=$(OBJ)/java/lemurproject/lemur
OUTDIRUI=$(OUTDIR)/ui
JSRC=java/lemurproject/lemur
JSRCUI=$(JSRC)/ui

INDRI_JSRC=java/lemurproject/indri
INDRI_JSRCUI=$(INDRI_JSRC)/ui
INDRI_OUTDIR=$(OBJ)/java/lemurproject/indri
INDRI_OUTDIRUI=$(INDRI_OUTDIR)/ui

JAVASRC = $(notdir $(wildcard $(JSRC)/*.java))
JAVASRCUI = $(notdir $(wildcard $(JSRCUI)/*.java))

INDRI_JAVASRC = $(notdir $(wildcard $(INDRI_JSRC)/*.java))
INDRI_JAVASRCUI = $(notdir $(wildcard $(INDRI_JSRCUI)/*.java))

ALLSRC= $(wildcard $(JSRC)/*.java) $(wildcard $(JSRCUI)/*.java) $(wildcard $(INDRI_JSRC)/*.java) $(wildcard $(INDRI_JSRCUI)/*.java)

OUTDIRPROP=$(OUTDIR)/ui/properties
PROPS = $(patsubst %,$(OUTDIRPROP)/%, $(notdir $(wildcard $(JSRCUI)/properties/*)))

INDRI_OUTDIRPROP=$(INDRI_OUTDIR)/ui/properties
INDRI_PROPS = $(patsubst %,$(INDRI_OUTDIRPROP)/%, $(notdir $(wildcard $(INDRI_JSRCUI)/properties/*)))

LEMUR_MFST=java/manifest.lemur
LEMUR_MFSTRET=java/manifest.LemurRet
LEMUR_MFSTIND=java/manifest.LemurIndex
INDRI_MFST=java/manifest.indri
MFSTRET=java/manifest.RetUI
MFSTIND=java/manifest.IndexUI

JARFILES=$(OBJ)/java/indri.jar $(OBJ)/java/RetUI.jar $(OBJ)/java/IndexUI.jar

INTERFACES = $(wildcard interface/*.i)

CSHARPSRC=interface/lemur.i
CSHARP_CPP=csharp/lemur_csharp.cpp

INDRI_CSHARPSRC=interface/indri.i
INDRI_CSHARP_CPP=csharp/indri_csharp.cpp

PHPSRC=interface/indri.i
PHPINTERFACES = $(PHPSRC) interface/indritypemaps.i
## get from configure

OUTDIRPHP=$(OBJ)/php
PHPLIB=$(OUTDIRPHP)/libindri_php$(PHP_SO)
PHPLIBPHP=php/libindri_php.php

JAVADIRS = $(INDRI_OUTDIR) $(INDRI_OUTDIRUI) $(INDRI_OUTDIRPROP) $(OBJ)/java/docs

DIRS=

JAVAOBJ =  $(patsubst %,$(INDRI_OUTDIR)/%, $(INDRI_JAVASRC:.java=.class)) $(patsubst %,$(INDRI_OUTDIRUI)/%, $(INDRI_JAVASRCUI:.java=.class))

SOLIB=$(OBJ)/java/lib$(PACKAGE)$(JAVA_SO)

OUTDIRCSHARP=$(OBJ)/csharp
CSHARP_SO=.so
#naming issues on windows
CSHARPLIB=$(OUTDIRCSHARP)/libindri_csharp$(CSHARP_SO)
CSHARP_DLL=$(OUTDIRCSHARP)/IndriCsharp.dll
PCK=lemurproject.lemur
INDRI_PCK=lemurproject.indri

DOC_PCK=$(INDRI_PCK) $(INDRI_PCK).ui

DOCARGS=-private -author -version
DOCS=$(patsubst %,$(INDRI_OUTDIR)/%,$(patsubst %.java,%.html,$(INDRI_JAVASRC)))
DOCSUI=$(patsubst %,$(INDRI_OUTDIRUI)/%,$(patsubst %.java,%.html,$(INDRI_JAVASRCUI)))
DOC=$(JAVADOC) -classpath java  $(DOCARGS)
DOCDEP=$(OBJ)/java/docs/index.html
TARGETS=
ifeq ($(WITH_SWIG), 1)
  ifeq ($(WITH_JAVA), 1)
    TARGETS += java-lib doc
    DIRS += $(JAVADIRS)
  endif
  ifeq ($(WITH_PHP), 1)
    TARGETS += php-lib
    DIRS += $(OUTDIRPHP)
  endif
  ifeq ($(WITH_CSHARP), 1)
    TARGETS += csharp-lib
    DIRS += $(OUTDIRCSHARP)
  endif
endif
.PHONY: java-lib php-lib csharp-lib doc

#all: java-lib php/indri_phpwrap.cpp
#remove doc from target
all: $(TARGETS)


java-lib: $(JAVADIRS) $(INDRI_PROPS) $(SOLIB) $(JARFILES)

php-lib: $(OUTDIRPHP) $(PHPLIB)

csharp-lib: $(OUTDIRCSHARP) $(CSHARPLIB) $(CSHARP_DLL)

$(SOLIB): $(LIBDEPS)

$(SOLIB): $(OBJ)/java/indri_jni.o
	$(CXX) $(JAVA_SHARED)  $< -o $@ $(LDFLAGS)

$(OBJ)/java/indri_jni.o: java/indri_jni.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(JAVAINC)

java/indri_jni.cpp: $(INTERFACES)
ifneq ("$(SWIG)", "")
	$(SWIG) $(SWIG_DEFS) -c++ -java -package $(INDRI_PCK) -o $@ -outdir $(INDRI_JSRC) $(INDRI_SRC) 
endif

$(INDRI_CSHARP_CPP): $(INTERFACES)
ifneq ("$(SWIG)", "")
	$(SWIG) -c++ -csharp -namespace Indri -o $@ -outdir csharp $(INDRI_CSHARPSRC) 
endif

$(OUTDIRCSHARP)/indri_csharp.o: $(INDRI_CSHARP_CPP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#needs csharp_shared
$(CSHARPLIB): $(LIBDEPS)

$(CSHARPLIB): $(OUTDIRCSHARP)/indri_csharp.o
	$(CXX) -g $(CSHARP_SHARED) $< -o $@ $(LDFLAGS)

$(CSHARP_DLL): $(INDRI_CSHARP_CPP)
	$(MCS) -target:library -out:$@ csharp/*.cs

$(DIRS):
	$(INSTALL_DIR) $@

$(OUTDIR)/%.class:$(JSRC)/%.java
	$(JAVAC) -classpath java -d $(OBJ)/java $<

$(INDRI_OUTDIR)/%.class:$(INDRI_JSRC)/%.java
	$(JAVAC) -classpath java -d $(OBJ)/java $<

$(OUTDIRPROP)/%:$(JSRCUI)/properties/%
	if test -f $< ; then $(INSTALL_DATA) $< $@ ; fi

$(INDRI_OUTDIRPROP)/%:$(INDRI_JSRCUI)/properties/%
	if test -f $< ; then $(INSTALL_DATA) $< $@ ; fi


$(DOCDEP): $(ALLSRC)
	$(DOC) -d $(OBJ)/java/docs -doctitle "Indri JNI" -windowtitle "Indri JNI" $(DOC_PCK)

$(OBJ)/java/indri.jar: $(JAVAOBJ) $(INDRI_MFST) $(DOCDEP)
	rm -f $@
	cd $(OBJ)/java; $(JAR) cmf ../../src/$(INDRI_MFST) indri.jar lemurproject docs

$(OBJ)/java/RetUI.jar: $(JAVAOBJ) $(MFSTRET) $(DOCDEP)
	rm -f $@
	cd $(OBJ)/java; $(JAR) cmf ../../src/$(MFSTRET) RetUI.jar lemurproject docs

$(OBJ)/java/IndexUI.jar: $(JAVAOBJ) $(MFSTIND) $(DOCDEP)
	rm -f $@
	cd $(OBJ)/java; $(JAR) cmf ../../src/$(MFSTIND) IndexUI.jar lemurproject docs

## PHP
php/indri_phpwrap.cpp: $(PHPINTERFACES)
ifneq ("$(SWIG)", "")
	$(SWIG) -php5 -c++ -o php/indri_phpwrap.cpp -outdir php $(INCLUDES) $(PHPSRC) 
endif

$(OUTDIRPHP)/indri_phpwrap.o: php/indri_phpwrap.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(PHPINCLUDE)

$(PHPLIB): $(LIBDEPS)

$(PHPLIB): $(OUTDIRPHP)/indri_phpwrap.o 
	$(CXX) -g $(PHP_SHARED) $< -o $@ $(LDFLAGS)


clean:
	rm -rf $(OUTDIRPHP) $(OUTDIRCSHARP) $(OBJ)/java 


install_dirs: $(pkgdatadir) $(pkgdatadir)/JNIdoc $(libdir)
$(pkgdatadir) $(pkgdatadir)/JNIdoc $(libdir):
	$(INSTALL_DIR) $@

install: install_dirs install_data install_docs

install_docs:
ifeq ($(WITH_SWIG), 1)
  ifeq ($(WITH_JAVA), 1)
	cp -pr $(OBJ)/java/docs/* $(pkgdatadir)/JNIdoc
  endif
endif

install_data:
ifeq ($(WITH_SWIG), 1)
  ifeq ($(WITH_JAVA), 1)
	$(INSTALL_DATA)  $(JARFILES) $(pkgdatadir)
	$(INSTALL_PROGRAM) $(SOLIB) $(libdir)
  endif
  ifeq ($(WITH_PHP), 1)
	$(INSTALL_PROGRAM) $(PHPLIB) $(libdir)
	$(INSTALL_DATA) $(PHPLIBPHP) $(pkgdatadir)
  endif
  ifeq ($(WITH_CSHARP), 1)
	$(INSTALL_PROGRAM) $(CSHARPLIB) $(libdir)
	$(INSTALL_PROGRAM) $(CSHARP_DLL) $(libdir)
  endif
endif

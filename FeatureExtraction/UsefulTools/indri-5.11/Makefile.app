## your application name here
APP=
SRC=$(APP).cpp
## extra object files for your app here
OBJ=

prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
INCPATH=-I$(includedir)
LIBPATH=-L$(libdir)
CXXFLAGS=-DPACKAGE_NAME=\"Indri\" -DPACKAGE_TARNAME=\"indri\" -DPACKAGE_VERSION=\"5.11\" -DPACKAGE_STRING=\"Indri\ 5.11\" -DPACKAGE_BUGREPORT=\"project@lemurproject.org\" -DYYTEXT_POINTER=1 -DINDRI_STANDALONE=1 -DHAVE_LIBM=1 -DHAVE_LIBPTHREAD=1 -DHAVE_LIBZ=1 -DHAVE_NAMESPACES=/\*\*/ -DISNAN_IN_NAMESPACE_STD=/\*\*/ -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_FSEEKO=1 -DHAVE_EXT_ATOMICITY_H=1 -DP_NEEDS_GNU_CXX_NAMESPACE=1 -DHAVE_MKSTEMP=1 -DHAVE_MKSTEMPS=1 -DNDEBUG=1  -g -O3 $(INCPATH)
CPPLDFLAGS  =  -lindri -lz -lpthread -lm 

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(APP) $(OBJ) $(LIBPATH) $(CPPLDFLAGS)

clean:
	rm -f $(APP)



#define this module
MODULE	= indri

-include ../MakeDefns
ALLHEADER += ../include/indri/greedy_vector ../include/indri/count_iterator
OBJLIB = libindri.a

all: $(OBJLIB)

QUERYSPEC = ../include/indri/QuerySpec.hpp
SPECOBJS = Copier.o Walker.o UnpackerInternal.o
SPECSRCS = ../src/Copier.cpp ../src/Walker.cpp ../src/UnpackerInternal.cpp
SPECHEADERS = ../include/indri/Copier.hpp ../include/indri/Walker.hpp 
GENERATORS = ../src/generators


# how to make a library from object files
$(OBJLIB): $(SPECHEADERS) $(OBJ) $(SPECOBJS)
	rm -f $@; $(AR) -rs $@ $(OBJ)

#check this.
$(SPECHEADERS): $(QUERYSPEC)
	cat $(QUERYSPEC) | awk -f $(GENERATORS)/include/$@.awk > $@

$(SPECSRCS): $(QUERYSPEC)
	cat $(QUERYSPEC) | awk -f $(GENERATORS)/src/$@.awk > $@


# compiling C++ code
%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# compiling C code
%.o: ../src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a

install:
	$(INSTALL_DATA) $(ALLHEADER) $(pkgincludedir)
	$(AR) -rs $(libdir)/$(INDRILIB) $(OBJ)

stamp:
	awk -f ../src/version-stamper "stamp=`date`" ../include/indri/indri-platform.h > ../include/indri/indri-platform.h2
	mv ../include/indri/indri-platform.h2 ../include/indri/indri-platform.h

# Each source file has a corresponding ".d" file
# that specifies its dependents. 
ifneq (clean, $(MAKECMDGOALS))
ifneq (install, $(MAKECMDGOALS))
ifneq (stamp, $(MAKECMDGOALS))
ifneq ($(strip $(CPPSRC)),)
-include $(patsubst %, ../depend/%, $(CPPSRC:.cpp=.d))
endif

ifneq ($(strip $(CSRC)),)
-include $(patsubst %, ../depend/%, $(CSRC:.c=.d))
endif
endif
endif
endif

# how to generate .d  file from a .C file
../depend/%.d: ../src/%.cpp
	@echo "Generating $@ for $<"
	@set -e; $(CXX) -MM $(CXXFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o \.\.\/depend\/\1.d : /g' > $@; [ -s $@ ] || rm -f $@

# how to generate .d  file from a .C file
../depend/%.d: ../src/%.cc
	@echo "Generating $@ for $<"
	@set -e; $(CXX) -MM $(CXXFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o \.\.\/depend\/\1.d : /g' > $@; [ -s $@ ] || rm -f $@

# how to generate .d file from a .c file
../depend/%.d: ../src/%.c
	@echo "Generating $@ for $<"
	@set -e; $(CC) -MM $(CFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o \.\.\/depend\/\1.d : /g' > $@; [ -s $@ ] || rm -f $@


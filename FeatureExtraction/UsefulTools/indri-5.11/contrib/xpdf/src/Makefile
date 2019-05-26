#define this module
MODULE	= xpdf

include ../../../MakeDefns

OBJLIB = lib$(MODULE).a

all: $(OBJLIB)

# compiling C++ code
%.o: ../src/%.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# compiling C++ code
%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# compiling C code
%.o: ../src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

# how to make a library from object files
$(OBJLIB): $(OBJ)
	rm -f $@; $(AR) -rs $@ $(OBJ)

clean:
	rm -f *.o *.a

INCDIR = $(includedir)/$(MODULE)
ALLHEADER = $(wildcard ../include/*.h*)
install:
	$(INSTALL_DIR) $(INCDIR)
	$(INSTALL_DATA) $(ALLHEADER) $(INCDIR)
	$(AR) -qs $(libdir)/$(INDRILIB) $(OBJ)

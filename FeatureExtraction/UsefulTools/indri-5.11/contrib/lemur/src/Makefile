#define this module
MODULE	= lemur

include ../../../MakeDefns

OBJLIB = liblemur.a

all: $(OBJLIB)

# compiling C++ code
%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -I../../../include -I../include/$(MODULE) -o $@ -c $<

# compiling C code
%.o: ../src/%.c
	$(CC) $(CFLAGS) -I../../../include -I../include/$(MODULE) -o $@ -c $<

# how to make a library from object files
$(OBJLIB): $(OBJ)
	rm -f $@; $(AR) -rs $@ $(OBJ)

clean:
	rm -f *.o *.a


INCDIR = $(includedir)/$(MODULE)
ALLHEADER = $(wildcard ../include/$(MODULE)/*.h*)
install:
	$(INSTALL_DATA) $(ALLHEADER) $(INCDIR)
	$(AR) -qs $(libdir)/$(INDRILIB) $(OBJ)

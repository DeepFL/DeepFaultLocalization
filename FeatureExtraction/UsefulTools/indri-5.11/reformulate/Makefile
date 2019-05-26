
include ../MakeDefns
SHARED=
INCPATH=-I../include $(patsubst %, -I../contrib/%/include, $(DEPENDENCIES))
LIBPATH=-L../obj  $(patsubst %, -L../contrib/%/obj, $(DEPENDENCIES))
LIBS=-lindri $(patsubst %, -l%, $(DEPENDENCIES))
APP=ReformulateQuery
OBJ=
all: $(OBJ)
	$(CXX) $(CXXFLAGS) $(APP).cpp -o $(APP) $(OBJ) $(LIBPATH) $(LIBS) $(CPPLDFLAGS)

install:
	$(INSTALL_PROGRAM) $(APP) $(bindir)

clean:
	rm -f $(APP)


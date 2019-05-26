LEMURDIR = ../..
include $(LEMURDIR)/MakeDefns
LIBNAMES=indri
CONTMODULES=contrib/lemur contrib/antlr contrib/xpdf
INCPATH = -I$(LEMURDIR)/include -I$(LEMURDIR)/contrib/lemur/include/lemur $(patsubst %, -I$(LEMURDIR)/%/include, $(CONTMODULES))

CFLAGS += $(INCPATH)
CXXFLAGS += $(INCPATH)

LIBPATH=-L$(LEMURDIR)/obj $(patsubst %,-L$(LEMURDIR)/%/obj, $(CONTMODULES))
LIBS=$(patsubst %,-l%, $(LIBNAMES) lemur antlr xpdf)

LIBDIRS=$(LEMURDIR)/obj/
LIBNAMESFULL=$(patsubst %,lib%.a, $(LIBNAMES))
LIBDEPS=$(join $(LIBDIRS), $(LIBNAMESFULL))

LDFLAGS = $(LIBPATH) $(LIBS) $(CPPLDFLAGS)

INSTDIR=$(pkgdatadir)/cgi
INSTALLDIRS=$(INSTDIR) $(INSTDIR)/templates
FILES=README README_Templates.txt bin/help-db.html bin/help-qry.html bin/lemur.config bin/lemur100.gif bin/lemur150.gif
TEMPLATE_FILES=bin/templates/ErrorPage.html bin/templates/GenericPage.html bin/templates/ResultsPage.html bin/templates/SearchPage.html bin/templates/SingleResult.html
 
OBJS = LemurCGI.o CGIConfiguration.o CGIOutput.o DBInterface.o IndriSearchInterface.o DictionaryHash.o SingleResultItem.o
PROG = bin/lemur.cgi

all: $(PROG)

$(PROG): $(OBJS) $(LIBDEPS)
	$(CXX)  -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

## target specific dependencies
LemurCGI.o: LemurCGI.cpp LemurSearchCGIConstants.h CGIConfiguration.h CGIOutput.h DictionaryHash.h DBInterface.h IndriSearchInterface.h
CGIConfiguration.o: CGIConfiguration.cpp CGIConfiguration.h LemurSearchCGIConstants.h
CGIOutput.o: CGIOutput.cpp CGIOutput.h LemurSearchCGIConstants.h CGIConfiguration.h DictionaryHash.h SingleResultItem.h
IndriSearchInterface.o: IndriSearchInterface.h CGIOutput.h
DBInterface.o: DBInterface.cpp DBInterface.h IndriSearchInterface.h CGIOutput.h
DictionaryHash.o: DictionaryHash.cpp DictionaryHash.h
SingleResultItem.o: SingleResultItem.cpp SingleResultItem.h LemurSearchCGIConstants.h DictionaryHash.h

$(INSTALLDIRS):
	$(INSTALL_DIR) $@

install: $(INSTALLDIRS)
	$(INSTALL_PROGRAM) $(PROG)  $(INSTDIR)
	$(INSTALL_DATA) $(FILES) $(INSTDIR)
	$(INSTALL_DATA) $(TEMPLATE_FILES) $(INSTDIR)/templates
	echo "read $(INSTDIR)/README for web server install instructions."

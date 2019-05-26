-include MakeDefns

INSTALLDIRS = $(bindir) $(includedir) $(pkgincludedir) $(includedir)/lemur $(libdir) $(pkgdatadir) $(pkgdatadir)/doc

.PHONY: all dist clean install $(INSTALLDIRS) site-search

all: 
	$(MAKE) -C contrib
	$(MAKE) -C obj -f ../src/Makefile
ifeq ($(WITH_SWIG), 1)
	$(MAKE) -C swig/src
endif
	$(MAKE) -C buildindex
	$(MAKE) -C runquery
	$(MAKE) -C indrid
	$(MAKE) -C dumpindex
	$(MAKE) -C harvestlinks
	$(MAKE) -C pagerank
	$(MAKE) -C rmodel
	$(MAKE) -C makeprior
	$(MAKE) -C site-search
	$(MAKE) -C modifyfields
	$(MAKE) -C clarity
	$(MAKE) -C reformulate

$(INSTALLDIRS):
	$(INSTALL_DIR) $@

clean:
	$(MAKE) clean -C contrib
	$(MAKE) clean -C obj -f ../src/Makefile
ifeq ($(WITH_SWIG), 1)
	$(MAKE) clean -C swig/src
endif
	$(MAKE) clean -C buildindex
	$(MAKE) clean -C runquery
	$(MAKE) clean -C indrid
	$(MAKE) clean -C dumpindex
	$(MAKE) clean -C harvestlinks
	$(MAKE) clean -C pagerank
	$(MAKE) clean -C rmodel
	$(MAKE) clean -C makeprior
	$(MAKE) -C site-search clean
	$(MAKE) -C modifyfields clean
	$(MAKE) -C clarity clean
	$(MAKE) -C reformulate clean
	rm -f depend/*

distclean: clean
	rm -rf autom4te.cache MakeDefns Makefile.app config.log config.status indri.ncb indri.suo
dist: distclean
	$(MAKE) stamp -C obj -f ../src/Makefile
	cd ..; tar zcvf indri-`date +%Y%m%d-%H%M`.tar.gz indri

install: $(INSTALLDIRS)
	rm -f $(libdir)/$(INDRILIB)
	$(MAKE) install -C contrib
	$(MAKE) install -C obj -f ../src/Makefile
ifeq ($(WITH_SWIG), 1)
	$(MAKE) install -C swig/src
endif
	$(MAKE) install -C buildindex
	$(MAKE) install -C runquery
	$(MAKE) install -C indrid
	$(MAKE) install -C dumpindex
	$(MAKE) install -C harvestlinks
	$(MAKE) install -C pagerank
	$(MAKE) install -C rmodel
	$(MAKE) install -C makeprior
	$(MAKE) install -C modifyfields
	$(MAKE) install -C clarity
	$(MAKE) install -C reformulate
	$(MAKE) install -C doc
	$(MAKE) -C site-search install
	$(INSTALL_DATA) Makefile.app $(pkgdatadir)

test:

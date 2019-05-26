LEMURDIR = ../..
include $(LEMURDIR)/MakeDefns

DIRS=images include style
INSTDIR=$(pkgdatadir)/php
INSTALLDIRS=$(INSTDIR) $(patsubst %,$(INSTDIR)/%, $(DIRS))
IMG_FILES=images/indri.png images/lemur_logo_32.gif
INC_FILES=include/config.php include/libindri.php include/libindri_php.php include/header.php include/footer.php include/error.php include/title.php include/results.php include/grouping.php
FILES=index.php query.php showdoc.php
STYLE=style/style.css

all: 

clean:

$(INSTALLDIRS):
	$(INSTALL_DIR) $@

install: $(INSTALLDIRS)
	$(INSTALL_DATA) $(FILES) $(INSTDIR)
	$(INSTALL_DATA) $(IMG_FILES) $(INSTDIR)/images
	$(INSTALL_DATA) $(INC_FILES) $(INSTDIR)/include
	$(INSTALL_DATA) $(STYLE) $(INSTDIR)/style


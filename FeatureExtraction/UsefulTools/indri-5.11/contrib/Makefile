include ../MakeDefns
all: 
ifeq ($(NEED_ANTLR), 1)
	$(MAKE) -C antlr/obj -f ../src/Makefile
endif
	$(MAKE) -C xpdf/obj -f ../src/Makefile
	$(MAKE) -C lemur/obj -f ../src/Makefile

clean:
ifeq ($(NEED_ANTLR), 1)
	$(MAKE) clean -C antlr/obj -f ../src/Makefile
endif
	$(MAKE) clean -C xpdf/obj -f ../src/Makefile
	$(MAKE) clean -C lemur/obj -f ../src/Makefile

install: 
ifeq ($(NEED_ANTLR), 1)
	$(MAKE) install -C antlr/obj -f ../src/Makefile
endif
	$(MAKE) install -C xpdf/obj -f ../src/Makefile
	$(MAKE) install -C lemur/obj -f ../src/Makefile

stamp:



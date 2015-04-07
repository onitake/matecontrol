SUBDIRS := src doc test
SOURCEDIR := src
MAKEFILE := Makefile

.PHONY: all clean doc test program erase reset debug fuse config noconfig menuconfig

all:
	list='$(SUBDIRS)'; for subdir in $$list; do test "$$subdir" = . -o ! -r "$$subdir/$(MAKEFILE)" || (cd $$subdir && $(MAKE) $(MAKEFLAGS)); done

help:
	@echo "Make targets:"
	@echo
	@echo "all:        Default; build all MC targets and documentation"
	@echo "clean:      Delete build targets and intermediate files"
	@echo "doc:        Compile documentation"
	@echo "test:       Compile test cases"
	@echo "program:    Flash main firmware to controller"
	@echo "erase:      Erase controller flash"
	@echo "reset:      Reboot the controller"
	@echo "debug:      Build all MC targets in debug mode (?)"
	@echo "fuse:       Set fuse bits on controller"
	@echo "config:     Run simple configuration"
	@echo "noconfig:   Run automatic configuration, updating entries"
	@echo "menuconfig: Run ncurses text mode UI configuration"

clean:
	list='$(SUBDIRS)'; for subdir in $$list; do test "$$subdir" = . -o ! -r "$$subdir/$(MAKEFILE)" || (cd $$subdir && $(MAKE) $(MAKEFLAGS) clean); done

doc:
	list='$(SUBDIRS)'; for subdir in $$list; do test "$$subdir" = . -o ! -r "$$subdir/$(MAKEFILE)" || (cd $$subdir && $(MAKE) $(MAKEFLAGS) doc); done

test:
	list='$(SUBDIRS)'; for subdir in $$list; do test "$$subdir" = . -o ! -r "$$subdir/$(MAKEFILE)" || (cd $$subdir && $(MAKE) $(MAKEFLAGS) test); done

program:
	subdir=$(SOURCEDIR); (cd $$subdir && $(MAKE) $(MAKEFLAGS) program); done

erase:
	subdir=$(SOURCEDIR); (cd $$subdir && $(MAKE) $(MAKEFLAGS) erase); done

reset:
	subdir=$(SOURCEDIR); (cd $$subdir && $(MAKE) $(MAKEFLAGS) reset); done

debug:
	subdir=$(SOURCEDIR); (cd $$subdir && $(MAKE) $(MAKEFLAGS) debug); done

fuse:
	subdir=$(SOURCEDIR); (cd $$subdir && $(MAKE) $(MAKEFLAGS) fuse); done

config:
	subdir=$(SOURCEDIR); (cd $$subdir && $(MAKE) $(MAKEFLAGS) config); done

noconfig:
	subdir=$(SOURCEDIR); (cd $$subdir && $(MAKE) $(MAKEFLAGS) noconfig); done

menuconfig:
	subdir=$(SOURCEDIR); (cd $$subdir && $(MAKE) $(MAKEFLAGS) menuconfig); done

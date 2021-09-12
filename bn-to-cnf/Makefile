DIR := $(shell cd "$( dirname "$0" )" && pwd)
WMC := $(shell cd $(DIR)/../.. && pwd)

ifeq ($(PREFIX),)
	PREFIX=$(WMC)
endif

export GCC_COLORS='error=01;31:warning=01;38;5;172:caret=01;38;5;172:locus=01:quote=:note=01;32:range1=:range2=:fixit-insert=:fixit-delete=:diff-filename=:diff-hunk=:diff-delete=:diff-insert=:type-diff='
export CMAKE_CXX_FLAGS += -fdiagnostics-color=auto
export CMAKE_C_FLAGS += -fdiagnostics-color=auto

DEFAULTBUILDDIR := build
DEBUGBUILDDIR	:= debug
RELEASEBUILDDIR	:= release
SOURCEDIR 		:= $(DIR)

MAKEFLAGS += --no-print-directory
CMAKEFLAGS += -DCMAKE_INSTALL_PREFIX:PATH=$(PREFIX)

.PHONY: build debug release clean help

build: $(DEFAULTBUILDDIR)/CMakeCache.txt
	@$(MAKE) -C $(DEFAULTBUILDDIR) install

release: CMAKEFLAGS += -DCMAKE_BUILD_TYPE=Release
release: $(RELEASEBUILDDIR)/CMakeCache.txt
	@$(MAKE) -C $(RELEASEBUILDDIR) install

debug: CMAKEFLAGS += -DCMAKE_BUILD_TYPE=Debug
debug: $(DEBUGBUILDDIR)/CMakeCache.txt
	@$(MAKE) -C $(DEBUGBUILDDIR) install

%/CMakeCache.txt:
	@mkdir -p $*
	@cd $* && cmake $(CMAKEFLAGS) $(SOURCEDIR)

clean:
	@rm -rf $(DEBUGBUILDDIR) $(DEFAULTBUILDDIR) $(RELEASEBUILDDIR)

help:
	@echo "Usage:"
	@echo "    make [option]"
	@echo ""
	@echo "Options:"
	@echo "    build*    : create build directory (cmake) and compile"
	@echo "    release   : same as 'build', but with NDEBUG"
	@echo "    debug     : same as 'build', but with debug symbols"
	@echo "    clean     : remove build directories"
	@echo ""
	@echo "    * = default"
	@echo ""


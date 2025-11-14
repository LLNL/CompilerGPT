-include env.mk
-include version.mk

BUILDTYPE ?= debug
PROJECT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

ifeq ($(BUILDTYPE),debug)
  OPTFLAG    ?= -O0
  DBGFLAG    ?= -ggdb
endif

ifeq ($(BUILDTYPE),release)
  OPTFLAG    ?= -O3
endif


BINARIES := compgpt.bin logfilter.bin prettyjson.bin code-assist.bin test-llmtools.bin
HEADERS  := ./include/tool_version.hpp ./include/llmtools.hpp
SOURCES  := $(BINARY:.bin=.cc)
OBJECTS  := $(SOURCES:.cc=.o)

INCLUDES   ?= -I$(BOOST_HOME)/include -I./include
BOOSTLIBS  ?= -Wl,-rpath=$(BOOST_HOME)/lib -L$(BOOST_HOME)/lib \
              -lboost_program_options -lboost_filesystem -lboost_atomic -lboost_json
CXXVERSION ?= -std=c++20
WARNFLAG   ?= -Wall -Wextra -pedantic
OPTFLAG    ?= -O3
CPUARCH    ?= -march=native
DBGFLAG    ?= -DNDEBUG=1

CXXFLAGS   := $(CXXVERSION) $(WARNFLAG) $(OPTFLAG) $(CPUARCH) $(DBGFLAG)


# Configure shared library flags
SONAME     := libllmtools.so.1
LIBNAME    := libllmtools.so
LIBDIR     := $(PROJECT_DIR)/lib
LIBPATH    := $(LIBDIR)/$(SONAME)
LDFLAGS    += -Wl,-rpath=$(LIBDIR) -L$(LIBDIR) -lllmtools
DLLFLAG    := -fPIC

$(info $(OBJECTS))

.phony: default
default: $(LIBDIR) $(LIBDIR)/$(LIBNAME) $(BINARIES)

$(LIBDIR):
	mkdir -p $(LIBDIR)

%.o: src/%.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DLLFLAG) -o $@ -c $<

$(LIBDIR)/$(SONAME): llmtools.o
	$(CXX) -shared -Wl,-soname,$(SONAME) -o $@ $^ $(BOOSTLIBS) -ldl

$(LIBDIR)/$(LIBNAME): $(LIBDIR)/$(SONAME)
	ln -sf $(SONAME) $@

compgpt.bin: compgpt.o $(LIBDIR)/$(LIBNAME)
	$(CXX) $(CXXFLAGS) -o $@ $< $(BOOSTLIBS) $(LDFLAGS) -pthread

prettyjson.bin: prettyjson.o $(LIBDIR)/$(LIBNAME)
	$(CXX) $(CXXFLAGS) -o $@ $< $(BOOSTLIBS) $(LDFLAGS) -pthread

test-llmtools.bin: test-llmtools.o $(LIBDIR)/$(LIBNAME)
	$(CXX) $(CXXFLAGS) -o $@ $< $(BOOSTLIBS) $(LDFLAGS) -pthread

code-assist.bin: code-assist.o $(LIBDIR)/$(LIBNAME)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(BOOSTLIBS) -pthread

logfilter.bin: logfilter.o $(LIBDIR)/$(LIBNAME)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(BOOSTLIBS) -pthread

#~ %.bin: %.o
#~ 	$(CXX) $(CXXFLAGS) -o $@ $^ $(BOOSTLIBS) -pthread

.phony: clean
clean:
	rm -rf *.o q.json query.json response.*

.phony: pure
pure: clean
	rm -rf *.bin lib/*.so* lib/*.so.*

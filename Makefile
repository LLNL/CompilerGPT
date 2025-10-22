-include env.mk
-include version.mk

BUILDTYPE ?= debug
PROJECT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

ifeq ($(BUILDTYPE),debug)
  OPTFLAG    ?= -O0
  DBGFLAG    ?= -ggdb
endif


BINARIES := compgpt.bin logfilter.bin prettyjson.bin code-assist.bin
HEADERS  := ./include/tool_version.hpp
SOURCES  := $(BINARY:.bin=.cc)
OBJECTS  := $(SOURCES:.cc=.o)

INCLUDES   ?= -I$(BOOST_HOME)/include -I./include
BOOSTLIBS  ?= -Wl,-rpath=$(BOOST_HOME)/lib -L$(BOOST_HOME)/lib -lboost_program_options -lboost_filesystem
CXXVERSION ?= -std=c++20
WARNFLAG   ?= -Wall -Wextra -pedantic
OPTFLAG    ?= -O3
CPUARCH    ?= -march=native
DBGFLAG    ?= -DNDEBUG=1

CXXFLAGS   := $(CXXVERSION) $(WARNFLAG) $(OPTFLAG) $(CPUARCH) $(DBGFLAG)

$(info $(OBJECTS))

.phony: default
default: $(BINARIES)

%.o: src/%.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DLLFLAG) -o $@ -c $<

libllmtools.a: llmtools.o
	ar -r $@ $<

compgpt.bin: compgpt.o libllmtools.a
	$(CXX) $(BOOSTLIBS) -pthread -o $@ $^

prettyjson.bin: prettyjson.o libllmtools.a
	$(CXX) $(BOOSTLIBS) -pthread -o $@ $^

test-llmtools.bin: test-llmtools.o libllmtools.a
	$(CXX) $(BOOSTLIBS) -pthread -o $@ $^

code-assist.bin: code-assist.o libllmtools.a
	$(CXX) -pthread -o $@ $^

%.bin: %.o
	$(CXX) $(BOOSTLIBS) -pthread -o $@ $^



.phony: clean
clean:
	rm -rf *.o

.phony: pure
pure: clean
	rm -rf *.bin *.a query.json response.txt

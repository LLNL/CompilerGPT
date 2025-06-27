-include env.mk
-include version.mk

BUILDTYPE ?= debug
PROJECT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

ifeq ($(BUILDTYPE),debug)
  OPTFLAG    ?= -O0
  DBGFLAG    ?= -ggdb
endif


BINARIES := compgpt.bin logfilter.bin prettyjson.bin
HEADERS  := ./include/tool_version.hpp
SOURCES  := $(BINARY:.bin=.cc)
OBJECTS  := $(SOURCES:.cc=.o)

INCLUDES   ?= -I$(BOOST_HOME)/include -I./include
#~ BOOSTLIBS  ?= -L$(BOOST_HOME)/lib -lboost_filesystem # -lboost_system -lboost_thread
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

%.bin: %.o
	$(CXX) $(BOOSTLIBS) -pthread -o $@ $^

.phony: clean
clean:
	rm -rf *.o

.phony: pure
pure: clean
	rm -rf *.bin query.json response.txt


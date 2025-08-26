-include env.mk
-include version.mk

BUILDTYPE ?= debug
PROJECT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

ifeq ($(BUILDTYPE),debug)
  OPTFLAG    ?= -O0
  DBGFLAG    ?= -ggdb
endif


BINARIES := compgpt.bin logfilter.bin prettyjson.bin
HEADERS  :=
SOURCES  := $(BINARY:.bin=.cc)
OBJECTS  := $(SOURCES:.cc=.o)

INCLUDES   ?= -I$(BOOST_HOME)/include -I./include
#~ BOOSTLIBS  ?= -L$(BOOST_HOME)/lib -lboost_filesystem # -lboost_system -lboost_thread
CXXVERSION ?= -std=c++20
WARNFLAG   ?= -Wall -Wextra -pedantic
OPTFLAG    ?= -O3
CPUARCH    ?= -march=native
DBGFLAG    ?= -DNDEBUG=1
VERSIONDEF ?= -DTOOL_VERSION=\"$(VERSION_STRING)\"

CXXFLAGS   := $(CXXVERSION) $(WARNFLAG) $(OPTFLAG) $(CPUARCH) $(DBGFLAG) $(VERSIONDEF)

$(info $(OBJECTS))

.phony: default
default: $(BINARIES)

%.o: src/%.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DLLFLAG) -o $@ -c $<
	
llmtools.o: src/llmtools.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DLLFLAG) -o $@ -c $<	
	

libllmtools.a: llmtools.o
	ar -r $@ $<

compgpt.bin: compgpt.o libllmtools.a
	$(CXX) $(BOOSTLIBS) -pthread -o $@ $^

test-llmtools.bin: test-llmtools.o libllmtools.a
	$(CXX) $(BOOSTLIBS) -pthread -o $@ $^

%.bin: %.o
	$(CXX) $(BOOSTLIBS) -pthread -o $@ $^
	
	

.phony: clean
clean:
	rm -rf *.o 

.phony: pure
pure: clean
	rm -rf *.bin *.a query.json response.txt


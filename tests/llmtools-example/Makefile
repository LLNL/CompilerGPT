# Minimal Makefile for llmtools example

# Include parent project's environment settings
-include ../../env.mk
# Don't include version.mk directly to avoid issues

# Compiler and flags
CXX ?= c++
CXXVERSION ?= -std=c++14
WARNFLAG ?= -Wall -Wextra -pedantic
OPTFLAG ?= -O0
DBGFLAG ?= -ggdb
INCLUDES ?= -I$(BOOST_HOME)/include -I../../src

# Linking flags
LDFLAGS ?= -L$(BOOST_HOME)/lib -lboost_program_options -lboost_filesystem -pthread

# Source and target
SRC = code_optimizer.cc
TARGET = code_optimizer.bin

# Default target
all: $(TARGET)

# Build the example
$(TARGET): $(SRC) ../../libllmtools.a
	$(CXX) $(CXXVERSION) $(WARNFLAG) $(OPTFLAG) $(DBGFLAG) $(INCLUDES) -o $@ $< ../../libllmtools.a $(LDFLAGS)

# Run the example
check: $(TARGET)
	@echo "Running llmtools example..."
	@echo "Make sure LLMTOOLS_PATH and OPENAI_API_KEY environment variables are set."
	@export LLMTOOLS_PATH=$$(cd ../.. && pwd) && ./$(TARGET)

# Clean up
clean:
	rm -f $(TARGET) query.json response.json

.PHONY: all check clean

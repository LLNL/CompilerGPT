# CompilerGPT

Compiler optimization reports are an important tool for performance engineers. However, they are often too complex for non compiler experts. CompilerGPT attempts to harness the power of LLM AI models to automatically attempt optimizations suggested by the optimization reports.

CompilerGPT is a framework that submits compiler optimization reports (e.g., clang) and the source code to an LLM. The LLM is prompted to prioritize the findings in the optimization reports and then to make changes in the code accordingly. An automated test harness validates the changes. The test harness provides feedback to the LLM on any errors that were introduced to the code base.

CompilerGPT iterates with the LLM a given number of times and reports on the obtained correctness and performance results of each iteration.

CompilerGPT calls AI modules through scripts which makes it flexible to use with different frameworks. Most connections are based on CURL. A legacy configuration for OpenAI's Python framework also exists. CompilerGPT's auto-configuration currently supports OpenAI, Anthropic, openrouter, and ollama.

---

## Badges

<!-- Add build status, license, and documentation badges here -->
[![License: BSD-3](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](LICENSE)

---

## Getting Started

### Minimal Requirements

- C++ compiler supporting C++20, e.g. GCC 12+
  - make: Boost header only (boost > 1.78 such as 1.85, 1.87). Required libraries include algorithm, asio, json, process
  - cmake (>=3.24): downloads boost 1.84 automatically
- curl
- Python 3 (for prettyjson.py)

#### Installing Dependencies

- **Boost**: Download and install from https://www.boost.org/ or use cmake for installation
- **curl**: Install via your package manager (e.g., `sudo apt install curl`)
- **Python 3**: Install via your package manager (e.g., `sudo apt install python3`)

#### Install with make

Set BOOST\_HOME to the boost installation directory and call make to build compgpt.bin, logfilter.bin, and prettyjson.bin.

```bash
compilergpt> export BOOST_HOME="/path/to/boost/installation"
compilergpt> make -j8
```

#### Install with cmake

Build compgpt.bin, logfilter.bin, and prettyjson.bin with cmake. cmake will download and use a tested boost version.

```bash
compilergpt> mkdir build
compilergpt> cd build
compilergpt/build> cmake ..
compilergpt/build> make -j8
```

---

### Directory Structure

- `include/`: C++ header files
- `src/`: C++ sources (compgpt and tools)
- `scripts/`: Scripts to interact with AI (gpt, claude, ollama, openrouter)
- `tests/`: Test codes to illustrate the working of CompilerGPT
- `figures/`: Diagrams and workflow figures
- `VERSION`, `version.mk`: Versioning information

---

## Utilities

- **compgpt.bin**: Main binary for running CompilerGPT.
- **logfilter.bin**: Utility for filtering logs.  
  _Usage: `./logfilter.bin [options] < input.log > output.log`_  
  _[Add more details as needed]_
- **prettyjson.bin**: Pretty-prints JSON files (C++ version).  
  _Usage: `./prettyjson.bin input.json`_
- **prettyjson.py**: Pretty-prints JSON files (Python version).  
  _Usage: `python3 src/prettyjson.py input.json`_

---

## Scripts

| Script | Purpose | Usage Example |
|--------|---------|---------------|
| scripts/claude/exec-claude.sh | Run queries with Anthropic Claude | `bash scripts/claude/exec-claude.sh ...` |
| scripts/gpt4/exec-openai.sh | Run queries with OpenAI GPT-4 | `bash scripts/gpt4/exec-openai.sh ...` |
| scripts/gpt4-python/exec-gpt-4o.sh | Run queries with OpenAI GPT-4o (Python) | `bash scripts/gpt4-python/exec-gpt-4o.sh ...` |
| scripts/gpt4-python/query-gpt-4o.py | Python utility for GPT-4o | `python3 scripts/gpt4-python/query-gpt-4o.py ...` |
| scripts/ollama/exec-ollama.sh | Run queries with Ollama | `bash scripts/ollama/exec-ollama.sh ...` |
| scripts/openrouter/exec-openrouter.sh | Run queries with OpenRouter | `bash scripts/openrouter/exec-openrouter.sh ...` |

To add a new LLM provider, create a new subdirectory in `scripts/` and add an `exec-*.sh` script following the existing pattern.

---

## Tests

- Test suites are in `tests/`, with subdirectories for different benchmarks.
- `humaneval/` contains paired `*_declaration.cc` and `*_test.cc` files.
- Other suites: `matmul-1/`, `matmul-2/`, `matmul-3/`, `nas-benchmarks/`, `omp-prefixsum/`, `simple/`, `smith-waterman/`, `translation-dafny-ada/`, `translation-dafny-cpp/`, etc.

### Running Tests

- If a test target exists in the Makefile:  
  _Usage: `make test`_  
  Otherwise, run tests manually as described in the relevant test directory.

### Adding Tests

- Follow the structure in `humaneval/` for new test cases: create a `*_declaration.cc` and a corresponding `*_test.cc` file.
- For other suites, follow the conventions in their respective directories.

---

## Figures

- Diagrams and workflow figures are in `figures/`.
- Example:  
  ![Workflow](figures/optai.png)

---

## Environment Variables

GPT Environment: Default queries will use OpenAI's API (in scripts/gpt4/). Three environment variables control the invocation:
- `OPENAI_API_KEY`: the openai key for queries
- `SSL_CERT_FILE` (optional): the location of a root certificate to avoid SSL connection errors.

Claude Environment:
- `ANTHROPIC_API_KEY`: the key to support Claude requests

Openrouter.ai Environment:
- `OPENROUTER_API_KEY`: the key to support openrouter.ai requests

---

## Building and Running CompilerGPT

### Build CompilerGPT

Set the BOOST_HOME environment variable to your Boost installation directory and build:

    export BOOST_HOME="/path/to/boost/install/dir"
    make # builds all binaries (`compgpt.bin`, `logfilter.bin`, `prettyjson.bin`).

### Running CompilerGPT

To run CompilerGPT on a C++ source file with a configuration:

    compgpt.bin --config=configfile.json c++-source-file

The configuration is read from the specified JSON file, which defines how to interact with the target compiler and LLM system.

### Creating Configuration Files

A default configuration can be created with:

    compgpt.bin --create-config --config=configfile.json

#### Using Alternative LLMs

Claude is an alternative to the GPT model. It can be used similarly, but requires a different configuration. For example, to create a default Claude configuration:

    compgpt.bin --create-config --config=claude.json --config:ai=claude --config:model=claude-3-5-sonnet-20241022

CompilerGPT works with both clang and gcc. To generate a configuration for gcc:

    compgpt.bin --create-config --config=gcc-claude.json --config:ai=claude --config:compiler=/usr/bin/g++

#### Deriving New Configurations

New configurations can be derived from existing ones using the `--config:from` option. This is useful for creating various configurations for different AI components or compilers that share prompts and evaluation scripts:

    compgpt.bin --create-config --config=ollama.json --config:ai=ollama --config:model=llama3.3 --config:from=claude.json

### Cleanup

- `make clean` removes object files.
- `make pure` removes binaries and intermediate files.

---

## Versioning

- Version information is in `VERSION` and `version.mk`.
- Update the `VERSION` file to change the project version.


---

## Example

CompilerGPT generates different versions and a [conversation history](Example.md).

---

## Other Use Cases

In addition, compgpt can be setup to use AI for translation projects. Currently, the config file needs to be tweaked manually to support that use case.

---

## License

CompilerGPT is distributed under a BSD 3 license.

SPDX-License-Identifier: BSD-3-Clause

See the LICENSE file for details.

Third-party benchmarks distributed with CompilerGPT retain the original licensing. See the respective directories for license details.

---

## Project Status

1.x Demonstrates the basic workflow

---

## Source Code Release

Copyright (c) 2025, Lawrence Livermore National Security, LLC.
All rights reserved. LLNL-CODE-2000601

---

## Contributing

Contributions are welcome! Please open issues or pull requests.  
[Add code style and contribution guidelines here or link to CONTRIBUTING.md if available.]

---

## Platform Support

- Supported platforms: Linux (tested).  
- Other platforms may work but are not officially supported.

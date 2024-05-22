# llmopt

A framework that uses optimization reports in context with an LLM system to optimize C++ codes.

## Getting started

Requirements:
* C++ compiler supporting C++20
  + boost json (boost > 1.78)
* pyhton3

Directory structure:
* scripts: scripts to interact with AI
* src: C++ source
* tests/humaneval: test codes (source: https://github.com/THUDM/CodeGeeX.git )

Build optai:

    make

Call optai:

    optai --config=configfile.json c++-source-file

The configurtation is read from the json file. The config file specifies how to interact with a target compiler and the LLM system.

A default configuration can be created by

    optai --config=configfile.json --createconfig



## License
TBD

## Project status
WORK IN PROGRESS

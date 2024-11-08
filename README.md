# optai

A framework that uses Clang optimization reports in context with an LLM system to optimize C++ codes. OptAI can be configured for multiple AI backends (e.g., Python API with gpt and curl with claude).

## Getting started

Requirements:
* C++ compiler supporting C++20
  + boost json (boost > 1.78)
* pyhton3 (only for gpt's API)

Directory structure:
* scripts: scripts to interact with AI (e.g., gpt and claude)
* src: C++ source
* tests:
  + humaneval (source: https://github.com/THUDM/CodeGeeX.git )

GPT Environment: Default queries will use openai's API (in scripts/gpt4/). Three environemnt variables control the invocation:
* OPENAI\_API\_KEY the openai key for queries
* OPENAI\_ENV\_DIR (optional) the location of the python environment with the openai connection framework. If unspecified, python3 will be used without special environment.
* SSL\_CERT\_FILE (optional) the location of a root certificate to avoid SSL connection errors.\
  typical error message on Linux:  \[SSL: CERTIFICATE\_VERIFY\_FAILED\] certificate verify failed: self signed certificate in certificate chain

Claude Environment:
* ANTHROPIC_API_KEY the key for queries


Build optai:

    export BOOST_HOME="/path/to/boost/install/dir"
    make

Call optai:

    optai --config=configfile.json c++-source-file

The configurtation is read from the json file. The config file specifies how to interact with a target compiler and the LLM system.


A default configuration for GPT4 can be created by

    optai --config=configfile.json --createconfig=gpt4

On BSD systems (such as OS X), the script scripts/gpt4/execquery-bsd.sh can be used instead of scripts/gpt4/execquery.sh to avoid issues with the GNU readlink utility.

Claude is an alternative to the GPT model. It can be used in a similar way than gpt, but it requires a different configuration. A default configuration file can be created by

    optai --config=configfile.json --createconfig=claude

Note, the interaction with GPT uses GPT's Python library, while the interaction with
claude is based on curl. This demonstrates the flexibility of the driver.


In addition, the optai driver can be setup to use AI for translation projects. Currently, the
config file has to be tweaked manually.


## License
TBD

## Project status
WORK IN PROGRESS


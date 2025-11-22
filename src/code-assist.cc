// Code-Assist library
//   - simple filter for AI integration into IDEs (e.g., geany)
//
// Copyright (c) 2025, Lawrence Livermore National Security, LLC.
// All rights reserved.  LLNL-CODE-2001821
//
// License: SPDX BSD 3-Clause "New" or "Revised" License
//          see LICENSE file for details
//
// Authors: pirkelbauer2,liao6 (at) llnl.gov

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <string>
#include <chrono>
#include <ctime>
#include <boost/json.hpp>

#include "llmtools.hpp"


struct CoderData
{
  std::string code     = {};
  std::string prompt   = {};
  bool        followup = false;
};

struct Settings
{
  llmtools::Settings       tools;
  std::string              systemMsg    = "You are a compiler expert and an experienced C++ programmer.";
  std::string              langMarker   = "cpp";
  std::string              historyFile  = "query.json";
  std::string              provider     = "claude";
  std::vector<std::string> configFiles  = {};
  bool                     printConfig  = false;
};


/// reads input stream \p is until end into CoderData representation.
/// \param  is the input stream
/// \result generated CoderData object
/// \details
///    extractCodeData reads from \p is until it finds one of two strings
///    at the beginning of a line. This data is stored in CoderData::code.
///    The two delimiters are: "init" and "fup".
///    If the delimiter is "fup" CoderData::followup is set to true,
///    otherwise it remains false.
///    Any text that comes after the delimter is stored in CoderData::prompt.
CoderData extractCodeData(std::istream& is)
{
  CoderData res;

  const std::string  INITIAL_PROMPT = "---";
  const std::string  FOLLOW_UP      = "+++";

  std::ostringstream codeStream;
  std::string        line;
  bool               delimiterFound = false;

  while (!delimiterFound)
  {
    //~ const auto& success = ;

    if (!std::getline(is, line)) throw std::runtime_error{"Invalid Prompt Formatting"};
    //~ std::cerr << line << std::endl;

    line.append("\n");

    if (line.rfind(INITIAL_PROMPT, 0) == 0)
    {
      res.prompt     = line.substr(INITIAL_PROMPT.size());
      delimiterFound = true;
    }
    else if (line.rfind(FOLLOW_UP, 0) == 0)
    {
      res.followup   = true;
      res.prompt     = line.substr(FOLLOW_UP.size());
      delimiterFound = true;
    }
    else
    {
      res.code += line;
    }
  }

  while (std::getline(is, line))
  {
    line.append("\n");

    res.prompt += line;
  }

  return res;
}

llmtools::ConversationHistory
prepareConversationHistory(const Settings& settings, const CoderData& data)
{
  std::string        codeMarkdown;

  if (data.followup)
    return llmtools::ConversationHistory{llmtools::readJsonFile(settings.tools.historyFile())};

  std::stringstream             codeStream(data.code);
  llmtools::ConversationHistory res(settings.tools, settings.systemMsg);

  res.appendPrompt(fileToMarkdown(settings.langMarker, codeStream, llmtools::SourceRange::all()));
  return res;
}

llmtools::ConversationHistory
queryResponse(const Settings& settings, llmtools::ConversationHistory history)
{
  const bool NO_RESPONSE_FOR_TESTING = false;

  if (NO_RESPONSE_FOR_TESTING)
  {
    std::cerr << "noqueryResponse" << history << std::endl;
    return history;
  }

  return queryResponse(settings.tools, std::move(history));
}

void
storeHistory(const Settings& settings, const llmtools::ConversationHistory& history)
{
  std::ofstream histFile{settings.tools.historyFile()};

  histFile << history << std::endl;
}

void
printResponse(std::ostream& os, const Settings&, const llmtools::ConversationHistory& history)
{
  for (const llmtools::CodeSection& code : llmtools::extractCodeSections(history.lastEntry()))
    os << llmtools::CodePrinter{code.code()}
       << std::endl;
}



Settings
parseConfigFile(std::string_view configFileName, Settings settings)
{
  try
  {
    Settings             config = settings;

    boost::json::value   cnf    = llmtools::readJsonFile(std::string(configFileName));
    boost::json::object& cnfobj = cnf.as_object();

    config.tools = llmtools::settings(cnfobj["tools"].as_object(), config.tools);

    config.systemMsg      = llmtools::loadField(cnfobj, "systemMsg",       config.systemMsg);
    config.langMarker     = llmtools::loadField(cnfobj, "langMarker",      config.langMarker);
    config.historyFile    = llmtools::loadField(cnfobj, "historyFile",     config.historyFile);
    config.provider       = llmtools::loadField(cnfobj, "provider",        config.provider);

    config.tools.historyFile() = config.historyFile;

    settings = config;
  }
  catch (const std::runtime_error& err)
  {
    std::cerr << "Error in config file: " << configFileName
              << std::endl
              << err.what()
              << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown error in config file: " << configFileName
              << std::endl;
  }

  return settings;
}

void
unparseConfigFile(std::ostream& os, const Settings& settings)
{
  os << "{"
     << "\n  \"tools\":"
     << "\n     {"
     << llmtools::SettingsJsonFieldWriter{settings.tools, 8}
     << "\n     },"
     << "\n  \"systemMsg\":\"" << settings.systemMsg << "\","
     << "\n  \"langMarker\":\"" << settings.langMarker << "\","
     << "\n  \"historyFile\":\"" << settings.historyFile << "\","
     << "\n  \"provider\":\"" << settings.provider << "\""
     << "\n}"
     << std::endl;
}


/// Functor processing command line arguments
struct CommandLineArgumentParser
{
  void operator()(std::string_view arg)
  {
    if (arg.rfind(pconfigFile, 0) != std::string::npos)
      opts = parseConfigFile(arg.substr(pconfigFile.size()), std::move(opts));
    else if (arg.rfind(pprovider, 0) != std::string::npos)
      opts.provider = arg.substr(pprovider.size());
    else if (arg.rfind(pmodel, 0) != std::string::npos)
      opts.tools.modelName() = arg.substr(pmodel.size());
    else if (arg.rfind(plang, 0) != std::string::npos)
      opts.langMarker = arg.substr(plang.size());
    else if (arg.rfind(phistoryFile, 0) != std::string::npos)
      opts.tools.historyFile() = opts.historyFile = arg.substr(phistoryFile.size());
    else if (arg.rfind(pprintConfig, 0) != std::string::npos)
      opts.printConfig = true;
    else if (arg.rfind(pmodelConfig, 0) != std::string::npos)
      opts.configFiles.emplace_back(arg.substr(pmodelConfig.size()));
  }

  operator Settings() && { return std::move(opts); }

  Settings opts;

  static const std::string pconfigFile;
  static const std::string pprovider;
  static const std::string pmodel;
  static const std::string plang;
  static const std::string phistoryFile;
  static const std::string pprintConfig;
  static const std::string pmodelConfig;
};

const std::string CommandLineArgumentParser::pconfigFile  = "--config=";
const std::string CommandLineArgumentParser::pprovider    = "--provider=";
const std::string CommandLineArgumentParser::pmodel       = "--model=";
const std::string CommandLineArgumentParser::plang        = "--lang=";
const std::string CommandLineArgumentParser::phistoryFile = "--history-file=";
const std::string CommandLineArgumentParser::pprintConfig = "--print-config";
const std::string CommandLineArgumentParser::pmodelConfig = "--model-file=";



Settings parseArguments(const std::vector<std::string>& args)
{
  return std::for_each( std::next(args.begin()), args.end(),
                        CommandLineArgumentParser{}
                      );
}

bool initialized(const Settings& settings)
{
  Settings defaultSettings;

  return settings.tools.exec() != defaultSettings.tools.exec();
}

Settings configure(std::vector<std::string> args)
{
  Settings res = parseArguments(args);

  if (!initialized(res))
  {
    llmtools::Configurations toolsConfig = llmtools::initializeWithDefault();

    for (const std::string& configFileName: res.configFiles)
      toolsConfig = llmtools::initializeWithConfigFile(configFileName, std::move(toolsConfig));

    res.tools               = llmtools::configure(toolsConfig, res.provider, res.tools.modelName());
    res.tools.historyFile() = res.historyFile;
  }

  if (res.printConfig)
    unparseConfigFile(std::cout, res);

  return res;
}



int main(int argc, char* argv[])
{
  Settings settings = configure(std::vector<std::string>(argv, argv+argc));

  if (settings.printConfig)
    return 0;

  CoderData                     data    = extractCodeData(std::cin);
  llmtools::ConversationHistory history = prepareConversationHistory(settings, data);

  history = queryResponse(settings, std::move(history));

  storeHistory(settings, history);
  printResponse(std::cout, settings, history);
  return 0;
}


#if TEST_PROMPT

/// prints a standard hello world message to \p os
void helloWorld(std::ostream& os);

---

Implement the C++ function.

#endif /* TEST_PROMPT */

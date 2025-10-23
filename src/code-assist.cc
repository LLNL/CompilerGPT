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
// #include <boost/program_options.hpp>

#include "llmtools.h"


struct CoderData
{
  std::string code     = {};
  std::string prompt   = {};
  bool        followup = false;
};

struct Settings
{
  llmtools::Settings tools;
  std::string        systemMsg    = "You are a compiler expert and an experienced C++ programmer.";
  std::string        langMarker   = "cpp";
  std::string        historyFile  = "query.json";
  std::string        provider     = "claude";
  std::string        model;
  std::string        libRoot;
  bool               printConfig  = false;
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

boost::json::value
prepareConversationHistory(const Settings& settings, const CoderData& data)
{
  boost::json::value res;
  std::string        codeMarkdown;

  if (data.followup)
  {
    res = llmtools::readJsonFile(settings.tools.historyFile);
  }
  else
  {
    std::stringstream codeStream(data.code);

    res = createConversationHistory(settings.tools, settings.systemMsg);
    codeMarkdown = fileToMarkdown(settings.langMarker, codeStream, llmtools::SourceRange::all());
  }

  return llmtools::appendPrompt(std::move(res), data.prompt + codeMarkdown);
}

boost::json::value
queryResponse(const Settings& settings, boost::json::value history)
{
  //~ std::cerr << history << std::endl;
  //~ return history;
  return queryResponse(settings.tools, std::move(history));
}

void
storeHistory(const Settings& settings, const boost::json::value& history)
{
  storeQuery(settings.tools, history);
}

void
printResponse(std::ostream& os, const Settings&, const boost::json::value& history)
{
  std::vector<llmtools::CodeSection> codeSections = llmtools::extractCodeSections(llmtools::lastEntry(history));

  for (const llmtools::CodeSection& code : codeSections)
  {
    llmtools::printUnescaped(os, code.code());
    os << std::endl;
  }
}


/// returns the absolute path for an existing file path \p filename.
std::filesystem::path absolutePath(std::string_view filename)
{
  return std::filesystem::absolute(std::filesystem::path{filename}).remove_filename();
}

/// queries a string field from a JSON object.
std::string_view
loadField(const boost::json::object& cnfobj, std::string fld, const std::string& alt)
{
  const auto pos = cnfobj.find(fld);

  if (pos != cnfobj.end())
    return pos->value().as_string();

  return alt;
}


Settings
parseConfigFile(std::string_view configFileName, Settings settings)
{
  try
  {
    Settings             config = settings;
    boost::json::value   cnf    = llmtools::readJsonFile(std::string(configFileName));
    boost::json::object& cnfobj = cnf.as_object();

    {
      boost::json::value& llmcnf     = cnfobj["tools"];
      boost::json::object& llmcnfobj = llmcnf.as_object();

      config.tools.invokeai       = loadField(llmcnfobj, "invokeai",        config.tools.invokeai);
      config.tools.responseFile   = loadField(llmcnfobj, "responseFile",    config.tools.responseFile);
      config.tools.responseField  = loadField(llmcnfobj, "responseField",   config.tools.responseField);
      config.tools.systemTextFile = loadField(llmcnfobj, "systemTextFile",  config.tools.systemTextFile);
      config.tools.roleOfAI       = loadField(llmcnfobj, "roleOfAI",        config.tools.roleOfAI);
    }

    config.systemMsg      = loadField(cnfobj, "systemMsg",       config.systemMsg);
    config.langMarker     = loadField(cnfobj, "langMarker",      config.langMarker);
    config.historyFile    = loadField(cnfobj, "historyFile",     config.historyFile);
    config.model          = loadField(cnfobj, "model",           config.model);
    config.provider       = loadField(cnfobj, "testScript",      config.provider);

    config.tools.historyFile = config.historyFile;

    settings = config;
  }
  catch (...)
  {
  }

  return settings;
}

void
unparseConfigFile(std::ostream& os, const Settings& settings)
{
  os << "{"
     << "\n  \"tools\":"
     << "\n     {"
     << "\n       \"invokai\":\"" << settings.tools.invokeai << "\","
     << "\n       \"responseFile\":\"" << settings.tools.responseFile << "\","
     << "\n       \"responseField\":\"" << settings.tools.responseField << "\","
     << "\n       \"systemTextFile\":\"" << settings.tools.systemTextFile << "\","
     << "\n       \"roleOfAI\":\"" << settings.tools.roleOfAI << "\""
     << "\n     },"
     << "\n  \"systemMsg\":\"" << settings.systemMsg << "\","
     << "\n  \"langMarker\":\"" << settings.langMarker << "\","
     << "\n  \"historyFile\":\"" << settings.historyFile << "\","
     << "\n  \"model\":\"" << settings.model << "\","
     << "\n  \"provider\":\"" << settings.provider << "\""
     << "\n}"
     << std::endl;
}


/// Functor processing command line arguments
struct CommandLineArgumentParser
{
  explicit
  CommandLineArgumentParser(std::filesystem::path absPath)
  : opts()
  {
    opts.libRoot = std::move(absPath);
  }

  void operator()(std::string_view arg)
  {
    if (arg.rfind(pconfigfile, 0) != std::string::npos)
      opts = parseConfigFile(arg.substr(pconfigfile.size()), std::move(opts));
    else if (arg.rfind(plibpath, 0) != std::string::npos)
      opts.libRoot = arg.substr(plibpath.size());
    else if (arg.rfind(pprovider, 0) != std::string::npos)
      opts.provider = arg.substr(pprovider.size());
    else if (arg.rfind(pmodel, 0) != std::string::npos)
      opts.model = arg.substr(pmodel.size());
    else if (arg.rfind(plang, 0) != std::string::npos)
      opts.langMarker = arg.substr(plang.size());
    else if (arg.rfind(phistoryFile, 0) != std::string::npos)
      opts.tools.historyFile = opts.historyFile = arg.substr(phistoryFile.size());
    else if (arg.rfind(pprintConfig, 0) != std::string::npos)
      opts.printConfig = true;
  }

  operator Settings() && { return std::move(opts); }

  Settings opts;

  static const std::string pconfigfile;
  static const std::string plibpath;
  static const std::string pprovider;
  static const std::string pmodel;
  static const std::string plang;
  static const std::string phistoryFile;
  static const std::string pprintConfig;
};

const std::string CommandLineArgumentParser::pconfigfile  = "--config=";
const std::string CommandLineArgumentParser::plibpath     = "--libroot=";
const std::string CommandLineArgumentParser::pprovider    = "--provider=";
const std::string CommandLineArgumentParser::pmodel       = "--model=";
const std::string CommandLineArgumentParser::plang        = "--lang=";
const std::string CommandLineArgumentParser::phistoryFile = "--history-file=";
const std::string CommandLineArgumentParser::pprintConfig = "--print-config";



Settings parseArguments(const std::vector<std::string>& args)
{
  return std::for_each( std::next(args.begin()), args.end(),
                        CommandLineArgumentParser{absolutePath(args.at(0))}
                      );
}

bool initialized(const Settings& settings)
{
  Settings defaultSettings;

  return settings.tools.invokeai != defaultSettings.tools.invokeai;
}

Settings configure(std::vector<std::string> args)
{
  Settings              res      = parseArguments(args);

  if (!initialized(res))
  {
    llmtools::LLMProvider provider = llmtools::provider(res.provider);

    if (res.model.size() == 0)
      res.model = llmtools::defaultModel(provider);

    res.tools             = llmtools::configure(res.libRoot, provider, res.model);
    res.tools.historyFile = res.historyFile;
  }

  if (res.printConfig)
    unparseConfigFile(std::cout, res);

  return res;
}



int main(int argc, char* argv[])
{
  Settings                 settings  = configure(std::vector<std::string>(argv, argv+argc));

  if (settings.printConfig)
    return 0;

  CoderData                data      = extractCodeData(std::cin);
  boost::json::value       history   = prepareConversationHistory(settings, data);

  history = queryResponse(settings, std::move(history));

  storeHistory(settings, history);
  printResponse(std::cout, settings, history);
  return 0;
}







// LogFilter (logfilter)
//   Configured through a JSON file, logfilter
//   extracts success and timing metrics from log files.
//
// Copyright (c) 2025, Lawrence Livermore National Security, LLC.
// All rights reserved.  LLNL-CODE-2000601
//
// License: SPDX BSD 3-Clause "New" or "Revised" License
//          see LICENSE file for details
//
// Authors: pirkelbauer2,liao6 (at) llnl.gov

#include <fstream>
#include <string>
#include <iostream>
#include <regex>

#include <boost/json/src.hpp>

namespace json = boost::json;

namespace
{

struct Settings
{
  std::string  validate;
  std::string  timing;
};

}

std::vector<std::string>
getCmdlineArgs(char** beg, char** lim)
{
  return std::vector<std::string>(beg, lim);
}

json::value
parseJsonLine(std::string line)
{
  json::stream_parser p;
  boost::system::error_code ec;

  p.write(line.c_str(), line.size(), ec);

  if (ec) return nullptr;

  p.finish(ec);
  if (ec) return nullptr;

  return p.release();
}

json::value
readJsonFile(std::istream& is)
{
  // adapted from the boost json documentation:
  //   https://www.boost.org/doc/libs/1_85_0/libs/json/doc/html/json/input_output.html#json.input_output.parsing

  boost::system::error_code  ec;
  boost::json::stream_parser p;
  std::string                line;

  while (std::getline(is, line))
  {
    p.write(line, ec);

    if (ec)
    {
      std::cerr << ec << std::endl;
      throw std::runtime_error("unable to parse JSON file: " + line);
    }
  }

  p.finish(ec);


  if (ec)
  {
    std::cerr << ec << std::endl;
    throw std::runtime_error("unable to finish parsing JSON file");
  }

  return p.release();
}



std::string_view
loadField(json::object& cnfobj, std::string fld, const std::string& alt)
{
  const auto pos = cnfobj.find(fld);

  if (pos != cnfobj.end())
    return pos->value().as_string();

  return alt;
}

bool loadField(json::object& cnfobj, std::string fld, bool alt)
{
  const auto pos = cnfobj.find(fld);

  if (pos != cnfobj.end())
    return pos->value().as_bool();

  return alt;
}

std::int64_t loadField(json::object& cnfobj, std::string fld, std::int64_t alt)
{
  const auto pos = cnfobj.find(fld);

  if (pos != cnfobj.end())
  {
    if (std::int64_t* ip = pos->value().if_int64())
      return *ip;

    if (std::uint64_t* up = pos->value().if_uint64())
    {
      assert(*up < std::uint64_t(std::numeric_limits<std::int64_t>::max()));
      return *up;
    }
  }

  return alt;
}



Settings loadConfig(const std::string& configFileName)
{
  Settings      settings;
  std::ifstream configFile{configFileName};

  if (!configFile.good())
  {
    std::cerr << "The file " << configFileName << " is NOT ACCESSIBLE (or does not exist.)"
              << std::endl;
    exit(1);
  }

  try
  {
    json::value   cnf    = readJsonFile(configFile);
    json::object& cnfobj = cnf.as_object();
    Settings      config;

    config.validate = loadField(cnfobj, "validate", config.validate);
    config.timing   = loadField(cnfobj, "timing",   config.timing);

    settings = std::move(config);
  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
    exit(1);
  }
  catch (...)
  {
    std::cerr << "Unknown error: Unable to read settings file."
              << std::endl;
    exit(1);
  }

  if ((settings.validate.size() == 0) || (settings.timing.size() == 0))
  {
    std::cerr << "Unset parameter"
              << std::endl;

    exit(1);
  }

  return settings;
}


struct CmdLineArgs
{
  std::string configFileName;
  std::string outputFileName;
};

void processOutput(const Settings& settings, const std::string& logFileName)
{
  bool          isValid      = false;
  long double   measuredTime = 0.0;
  std::ifstream ifs(logFileName);
  std::regex    validation{settings.validate};
  std::regex    timing{settings.timing};

  std::string line;

  while (std::getline(ifs, line))
  {
    std::smatch res;

    if (!isValid && std::regex_search(line, res, validation))
    {
      isValid = true;
    }

    if (std::regex_search(line, res, timing) && res.size() == 2)
    {
      std::stringstream timeStream{res[1]};
      timeStream >> measuredTime;
    }
  }

  if (!isValid)
  {
    std::cerr << "invalid" << std::endl;
    exit(1);
  }

  std::cout << measuredTime << std::endl;
}

CmdLineArgs parseArguments(const std::vector<std::string>& args)
{
  assert(args.size() == 2);

  return { args[0], args[1] };
}


int main(int argc, char** argv)
{
  CmdLineArgs cmdlnargs = parseArguments(getCmdlineArgs(argv+1, argv+argc));
  Settings    settings  = loadConfig(cmdlnargs.configFileName);

  processOutput(settings, cmdlnargs.outputFileName);
  return 0;
}


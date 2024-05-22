
#include <fstream>
#include <string>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/process.hpp>
//~ #include <boost/filesystem.hpp>
#include <boost/json/src.hpp>

#include <filesystem>

namespace json = boost::json;

const std::string CC_MARKER_BEGIN = "```cpp";
const std::string CC_MARKER_LIMIT = "```";

namespace
{

const char* synopsis = "optai: code optimizations through LLMs"
                       "\n  description: feeds clang optimization report of a kernel to"
                       "\n               an LLM and asks it to optimize the source code"
                       "\n               The rewritten part is extracted from the LLM"
                       "\n               and stored in a file."
                       "\n               The file is compiled and checked for software"
                       "\n               bugs and performance using a specified test"
                       "\n               harness.";

const char* usage = "usage: optai switches c++file.cc"
                    "\n  switches:"
                    "\n    --help            displays this help message and exits."
                    "\n    --config=jsonfile config file in json format."
                    "\n                      default: jsonfile=optai.json"
                    "\n    --createconfig    creates config file and exits.";

struct Settings
{
  std::string invokeai     = "./scripts/gpt4/execquery.sh";
  std::string optcompiler  = "/usr/bin/clang";
  std::string optreport    = "-Rpass-missed=.";
  std::string optcompile   = "-c";
  std::string queryFile    = "query.json";
  std::string responseFile = "response.txt";
  bool        justResponse = false;
  bool        justCompile  = false;
  bool        jsonResponse = false;
};

struct CmdLineArgs
{
  bool        help              = false;
  bool        createConfig      = false;
  std::string configFileName    = "optai.json";
};

const char* as_string(bool v)
{
  return v ? "true" : "false";
}

}

std::vector<std::string>
getCmdlineArgs(char** beg, char** lim)
{
  return std::vector<std::string>(beg, lim);
}

template <class Iter>
using RangePrinterBase = std::tuple<Iter, Iter, std::string_view>;

template <class Iter>
struct RangePrinter : RangePrinterBase<Iter>
{
  using base = RangePrinterBase<Iter>;
  using base::base;

  Iter             begin() const { return std::get<0>(*this); };
  Iter             end()   const { return std::get<1>(*this); };
  std::string_view sep()   const { return std::get<2>(*this); };
};

template <class Iter>
std::ostream&
operator<<(std::ostream& os, RangePrinter<Iter> rng)
{
  for (const auto& el : rng) os << rng.sep() << el;

  return os;
}

template <class Sequence>
RangePrinter<typename Sequence::const_iterator>
range(const Sequence& seq, std::string_view sep = " ")
{
  return RangePrinter<typename Sequence::const_iterator>{seq.begin(), seq.end(), sep};
}



std::string
invokeCompiler(const Settings& settings, std::vector<std::string> args)
{
  std::string src = std::move(args.back());
  args.pop_back();

  args.push_back(settings.optreport);
  args.push_back(settings.optcompile);
  args.push_back(src);

  std::cerr << "inv: " << settings.optcompiler << range(args) << std::endl;

  boost::asio::io_service  ios;
  std::future<std::string> outstr;
  std::future<std::string> errstr;
  std::future<int>         exitCode;
  boost::process::child    compilation( settings.optcompiler,
                                        boost::process::args(args),
                                        boost::process::std_in.close(),
                                        boost::process::std_out > outstr,
                                        boost::process::std_err > errstr,
                                        boost::process::on_exit = exitCode,
                                        ios
                                      );

  ios.run();

/*
  std::vector<std::string> res;
  std::stringstream        report;
  std::string              line;

  report.str(errstr.get());

  while (std::getline(report, line))
    res.emplace_back(std::move(line));
*/

  std::cerr << exitCode.get() << std::endl;
  return errstr.get();
}

void compileResult(const Settings& settings, std::string newFile, std::vector<std::string> args)
{
  args.pop_back();
  args.push_back(newFile);

  std::string res = invokeCompiler(settings, args);

  std::cerr << "recompile: " << res << std::endl;
}

int invokeAI(const Settings& settings)
{
  std::cerr << "inv: " << settings.invokeai << std::endl;

  std::vector<std::string> noargs;
  boost::asio::io_service  ios;
  std::future<std::string> outstr;
  std::future<std::string> errstr;
  boost::process::child    ai( settings.invokeai,
                               boost::process::args(noargs),
                               boost::process::std_in.close(),
                               boost::process::std_out > outstr,
                               boost::process::std_err > errstr,
                               ios
                             );

  ios.run();

/*
  std::vector<std::string> res;
  std::stringstream        report;
  std::string              line;

  report.str(errstr.get());

  while (std::getline(report, line))
    res.emplace_back(std::move(line));
*/

  std::cout << outstr.get() << std::endl;
  std::cerr << errstr.get() << std::endl;

  return ai.native_exit_code();
}



std::string loadCodeQuery(const std::string& output, const std::string& filename)
{
  std::stringstream txt;
  std::ifstream     src{filename};
  std::string       line;

  txt << "Consider the following code\n" << CC_MARKER_BEGIN;

  while(std::getline(src, line)) {
    txt << line << "\n";
  }

  txt << CC_MARKER_LIMIT << '\n'
      << "Clang produces the optimization report:\n"
      << output
      << "\nPrioritize the optimizations and rewrite the code to enable better compiler optimizations.\n"
      << std::flush;

  return txt.str();
}

json::value getJsonQuery(const Settings& settings, std::string output, std::string filename)
{
  json::array  res;

  if (settings.justResponse)
  {
    // for setting the output format to JSON see:
    //   https://platform.openai.com/docs/api-reference/chat
    //

    // set output format - 1
    // output format needs to be set in the execution script ...
    if (false)
    {
      json::object line;
      json::object response_format;

      response_format["type"] = "json_object";
      line["response_format"] = std::move(response_format);

      res.emplace_back(std::move(line));
    }

    std::cerr << "json format requested: "
              << "\n  also set response_format[\"type\"] = \"json_object\""
              << "\n  in the driver script."
              << std::endl;
  }

  {
    json::object line;

    line["role"]    = "system";
    line["content"] = "You are a compiler expert for C++ code optimization. Our goal is to improve the existing code.";

    res.emplace_back(std::move(line));
  }

  if (settings.jsonResponse)
  {
    // set output format - 2

    json::object line;

    line["role"]    = "system";
    line["content"] = "Provide output in JSON format";

    res.emplace_back(std::move(line));
  }

  {
    json::object q;

    q["role"]    = "user";
    q["content"] = loadCodeQuery(output, filename);

    res.emplace_back(std::move(q));
  }

  return res;
}

void storeQuery(const Settings& settings, const json::value& query)
{
  std::ofstream queryfile{settings.queryFile};

  queryfile << query << std::endl;
}

json::value
parseJsonLine(std::string line)
{
  json::stream_parser p;
  json::error_code    ec;

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
    std::cerr << line << std::endl;
    p.write(line, ec);

    if (ec)
    {
      std::cerr << ec << std::endl;
      throw std::runtime_error("unable to parse AI response");
    }
  }


  p.finish(ec);

  std::cerr << ec << std::endl;

  if (ec) throw std::runtime_error("unable to finish parsing AI response");

  return p.release();
}

std::string generateNewFileName(const std::string& filename, int iteration)
{
  std::size_t pos = filename.find_last_of('.');
  std::string res;

  if (pos == std::string::npos)
  {
    res = filename;
    res.append(std::to_string(iteration));
  }
  else
  {
    const auto beg = filename.begin();

    std::copy(beg, beg+pos, std::back_inserter(res));
    res.append(std::to_string(iteration));
    std::copy(beg+pos, filename.end(), std::back_inserter(res));
  }

  if (std::filesystem::exists(res))
    return generateNewFileName(filename, iteration+1);

  return res;
}


std::string storeGeneratedFile(std::string_view vw, const std::string& filename, int iteration = 1)
{
  std::string         newFile   = generateNewFileName(filename, iteration);
  const std::size_t   mark      = vw.find(CC_MARKER_BEGIN);
  if (mark == std::string::npos)
  {
    std::cerr << vw << std::endl;
    throw std::runtime_error{"Cannot find C++ code in AI output."};
  }

  const std::size_t   beg       = mark + CC_MARKER_BEGIN.size();
  const std::size_t   lim       = vw.find(CC_MARKER_LIMIT, beg);
  if (beg == std::string::npos) throw std::runtime_error{"Cannot find C++ code end in AI output."};

  std::string_view    code = vw.substr(beg, lim - beg);

  std::ofstream       outf(newFile);

  char last = ' ';

  // print to file while replacing "\\n" with "\n".
  for (char ch : code)
  {
    if ((last == '\\') && (ch == 'n'))
    {
      outf << "\n";
      last = ' ';
    }
    else
    {
      if (last == '\\')
      {
        outf << last;
        last = ' ';
      }

      if (ch == '\\')
        last = ch;
      else
        outf << ch;
    }
  }

  //~ outf << code << std::endl;
  return newFile;
}


std::string
readTxtFile(std::istream& is)
{
  // adapted from the boost json documentation:
  //   https://www.boost.org/doc/libs/1_85_0/libs/json/doc/html/json/input_output.html#json.input_output.parsing

  std::string line;
  std::string res;

  while (std::getline(is, line))
  {
    if (res.size())
    {
      res.reserve(res.size() + line.size() + 1);
      res.append("\n").append(line);
    }
    else
    {
      res = std::move(line);
    }
  }

  return res;
}


std::string loadAIResponseJson(const Settings& settings)
{
  std::ifstream responseFile{settings.responseFile};

  json::value         obj = readJsonFile(responseFile);
  const json::object& jsObj     = obj.as_object();
  const json::value&  jsContent = jsObj.at("content");
  const json::string& content   = jsContent.as_string();
  std::string_view    contView(content.begin(), content.size());

  return std::string(contView);
}


std::string loadAIResponseTxt(const Settings& settings)
{
  std::ifstream responseFile{settings.responseFile};

  return readTxtFile(responseFile);
}

std::string loadAIResponse(const Settings& settings)
{
  if (settings.jsonResponse)
    return loadAIResponseJson(settings);

  return loadAIResponseTxt(settings);
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

Settings loadConfig(const std::string& configFileName)
{
  Settings settings;

  try
  {
    std::ifstream configFile{configFileName};
    json::value   cnf    = readJsonFile(configFile);
    json::object& cnfobj = cnf.as_object();

    settings.invokeai     = loadField(cnfobj, "invokeai",     settings.invokeai);
    settings.optcompiler  = loadField(cnfobj, "optcompiler",  settings.optcompiler);
    settings.optreport    = loadField(cnfobj, "optreport",    settings.optreport);
    settings.optcompile   = loadField(cnfobj, "optcompile",   settings.optcompile);
    settings.justResponse = loadField(cnfobj, "justResponse", settings.justResponse);
    settings.justCompile  = loadField(cnfobj, "justCompile",  settings.justCompile);
    settings.queryFile    = loadField(cnfobj, "queryFile",    settings.queryFile);
    settings.responseFile = loadField(cnfobj, "responseFile", settings.responseFile);
  }
  catch (...)
  {
    std::cerr << "Unable to read settings file. Using default values." << std::endl;
  }

  settings.justResponse = settings.responseFile.ends_with(".json");

  if (settings.justResponse & settings.justCompile)
  {
    std::cerr << "only justResponse or justCompile can be set."
              << std::endl;

    exit(0);
  }

  return settings;
}

void createDefaultConfig(const std::string& configFileName)
{
  if (std::filesystem::exists(configFileName))
  {
    std::cerr << "config file " << configFileName << " exists."
              << "\n  not creating a new file! (delete file or choose different file name)"
              << std::endl;

    return;
  }

  std::ofstream ofs(configFileName);
  Settings      defaults;

  ofs << "{"
      << "\n  \"invokeai\":\""     << defaults.invokeai << "\","
      << "\n  \"optcompiler\":\""  << defaults.optcompiler << "\","
      << "\n  \"optreport\":\""    << defaults.optreport << "\","
      << "\n  \"optcompile\":\""   << defaults.optcompile << "\","
      << "\n  \"justResponse\":\"" << as_string(defaults.justResponse) << "\","
      << "\n  \"justCompile\":\""  << as_string(defaults.justCompile) << "\","
      << "\n  \"queryFile\":\""    << defaults.queryFile << "\","
      << "\n  \"responseFile\":\"" << defaults.responseFile << "\""
      << "\n}" << std::endl;
}


struct CmdLineProc
{
  void operator()(const std::string& arg)
  {
    if (arg.rfind(phelp) != std::string::npos)
      opts.help = true;
    if (arg.rfind(pcreateconfig) != std::string::npos)
      opts.createConfig = true;
    else if (arg.rfind(pconfig) != std::string::npos)
      opts.configFileName = arg.substr(pconfig.size());
  }

  operator CmdLineArgs() && { return std::move(opts); }

  CmdLineArgs opts;

  static std::string phelp;
  static std::string pcreateconfig;
  static std::string pconfig;
};

std::string CmdLineProc::phelp         = "--help";
std::string CmdLineProc::pcreateconfig = "--createconfig";
std::string CmdLineProc::pconfig       = "--config=";

CmdLineArgs parseArguments(const std::vector<std::string>& args)
{
  return std::for_each(args.begin(), args.end(), CmdLineProc{});
}




int main(int argc, char** argv)
{
  std::vector<std::string> args      = getCmdlineArgs(argv+1, argv+argc);
  CmdLineArgs              cmdlnargs = parseArguments(args);

  if (cmdlnargs.help)
  {
    std::cout << synopsis << std::endl
              << usage << std::endl
              << std::endl;
    return 0;
  }

  if (cmdlnargs.createConfig)
  {
    createDefaultConfig(cmdlnargs.configFileName);
    return 0;
  }

  Settings                 settings = loadConfig(cmdlnargs.configFileName);

  if (!settings.justResponse)
  {
    std::string            output = invokeCompiler(settings, args);

    if (settings.justCompile) return 0;

    json::value            query  = getJsonQuery(settings, std::move(output), args.back());
    /*std::string              queryArgFile =*/ storeQuery(settings, query);
    /*int                      result =*/ invokeAI(settings);
  }

  std::string              response = loadAIResponse(settings);
  std::string              newFile  = storeGeneratedFile(response, args.back());

  compileResult(settings, newFile, args);

  return 0;
}


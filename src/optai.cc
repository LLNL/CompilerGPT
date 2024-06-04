
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
                    "\n    --create-config   creates config file and exits."
                    "\n    --harness-param=p sets an optional parameter for the test harness."
                    "\n                      default: none"
                    "\n                      If set, the parameter is passed as second"
                    "\n                      argument to the test script configured in"
                    "\n                      the config file."
                    "\n                      The tester is invoked with two arguments,"
                    "\n                      the generated file and param:"
                    "\n                        testScript genfile.cc p"
                    "\n"
                    "\n  TestScript: success or failure is returned through the exit status"
                    "\n              a numeric quality score is returned on the last"
                    "\n              non-empty line on stdout."
                    "\n              The lower the quality score the better (e.g., runtime)."
                    ;

struct Settings
{
  std::string  invokeai     = "./scripts/gpt4/execquery.sh";
  std::string  optcompiler  = "/usr/bin/clang";
  std::string  optreport    = "-Rpass-missed=.";
  std::string  optcompile   = "-c";
  std::string  queryFile    = "query.json";
  std::string  responseFile = "response.txt";
  std::string  testScript   = "";
  bool         justResponse = false;
  bool         justCompile  = false;
  bool         jsonResponse = false;
  std::int64_t iterations   = 1;
};

struct CmdLineArgs
{
  bool                     help              = false;
  bool                     createConfig      = false;
  std::string              configFileName    = "optai.json";
  std::string              harness           = "";
  std::vector<std::string> args;
};

const char* as_string(bool v)
{
  return v ? "true" : "false";
}

}

/*
struct CompilationError : std::runtime_error
{
  using base = std::runtime_error;
  using base::base;
};
*/

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

using CompilationResultBase = std::tuple<std::string, bool>;

struct CompilationResult : CompilationResultBase
{
  using base = CompilationResultBase;
  using base::base;

  const std::string& output()  const { return std::get<0>(*this); }
  bool               success() const { return std::get<1>(*this); }
};


CompilationResult
invokeCompiler(const Settings& settings, std::vector<std::string> args)
{
  std::string src = std::move(args.back());
  args.pop_back();

  args.push_back(settings.optreport);
  args.push_back(settings.optcompile);
  args.push_back(src);

  std::cerr << "compile: " << settings.optcompiler << range(args) << std::endl;

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

  const bool success = exitCode.get() == 0;

  return { errstr.get(), success };
}



CompilationResult
compileResult(const Settings& settings, std::string newFile, std::vector<std::string> args)
{
  args.pop_back();
  args.push_back(newFile);

  return invokeCompiler(settings, args);
}

int invokeAI(const Settings& settings)
{
  std::cerr << "askgpt: " << settings.invokeai << std::endl;

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

template <class F>
F nanValue() { return std::numeric_limits<F>::quiet_NaN(); }

/// reads the string s and returns the numeric value of the last
///   non-empty line.
long double
testScore(std::string_view s)
{
  std::size_t       beg = 0;
  std::size_t       pos = s.find('\n', beg);
  std::string_view  line;

  while (pos != std::string::npos)
  {
    std::string_view cand = s.substr(beg, pos-beg-1);

    if (!cand.empty()) line = cand;

    beg = pos + 1;
    pos = s.find('\n', beg);
  }

  std::string_view cand = s.substr(beg, s.size() - beg);

  if (!cand.empty()) line = cand;

  long double res = nanValue<long double>();

  try
  {
    res = std::stold(std::string(line));
  }
  catch (const std::invalid_argument& ex) { std::cerr << ex.what() << std::endl; }
  catch (const std::out_of_range& ex)     { std::cerr << ex.what() << std::endl; }

  return res;
}

using TestResultBase = std::tuple<bool, long double>;

struct TestResult : TestResultBase
{
  using base = TestResultBase;
  using base::base;

  bool        success() const { return std::get<0>(*this); }
  long double score()   const { return std::get<1>(*this); }
};


TestResult
invokeTestScript(const Settings& settings, const std::string& filename, const std::string& harness)
{
  if (settings.testScript.empty())
  {
    std::cerr << "No test-script configured. Not running tests."
              << std::endl;

    return { true, 0.0 };
  }

  std::vector<std::string> args{filename};

  if (!harness.empty())
    args.push_back(harness);

  boost::asio::io_service  ios;
  std::future<std::string> outstr;
  std::future<std::string> errstr;
  boost::process::child    tst( settings.testScript,
                                boost::process::args(args),
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

  std::string outs = outstr.get();
  std::string errs = errstr.get();

  std::cout << outs << std::endl;
  std::cerr << errs << std::endl;

  return { tst.native_exit_code() == 0, testScore(outs) };
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

json::value initialPrompt(const Settings& settings, std::string output, std::string filename)
{
  json::array res;

  if (settings.jsonResponse)
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

json::value
appendSuccessPrompt(json::value val, std::string output, std::string /*filename*/)
{
  json::array&      res = val.as_array();
  json::object      q;
  std::stringstream txt;

  txt << "The previous code works. Following is the current Clang optimization report:\n"
      << output
      << "\nPrioritize the optimizations and rewrite the code to enable better compiler optimizations.\n"
      << std::flush;

  q["role"]    = "user";
  q["content"] = txt.str();

  res.emplace_back(std::move(q));
  return val;
}

json::value
appendFailurePrompt(json::value val, std::string output, std::string /*filename*/)
{
  json::array&      res = val.as_array();
  json::object      q;
  std::stringstream txt;

  txt << "The previous code did not compile. Here is the error report:\n"
      << output
      << "\nFix the compile errors and generate code that compiles.\n"
      << std::flush;

  q["role"]    = "user";
  q["content"] = txt.str();

  res.emplace_back(std::move(q));
  return val;
}


json::value
appendResponse(json::value val, std::string response)
{
  json::array& res = val.as_array();
  json::object line;

  line["role"]    = "system";
  line["content"] = response;

  res.emplace_back(std::move(line));
  return val;
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
  Settings settings;

  try
  {
    std::ifstream configFile{configFileName};
    json::value   cnf    = readJsonFile(configFile);
    json::object& cnfobj = cnf.as_object();
    Settings      config;

    config.invokeai     = loadField(cnfobj, "invokeai",     config.invokeai);
    config.optcompiler  = loadField(cnfobj, "optcompiler",  config.optcompiler);
    config.optreport    = loadField(cnfobj, "optreport",    config.optreport);
    config.optcompile   = loadField(cnfobj, "optcompile",   config.optcompile);
    config.justResponse = loadField(cnfobj, "justResponse", config.justResponse);
    config.justCompile  = loadField(cnfobj, "justCompile",  config.justCompile);
    config.queryFile    = loadField(cnfobj, "queryFile",    config.queryFile);
    config.responseFile = loadField(cnfobj, "responseFile", config.responseFile);
    config.testScript   = loadField(cnfobj, "testScript",   config.testScript);
    config.iterations   = loadField(cnfobj, "iterations",   config.iterations);

    settings = std::move(config);
  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what()
              << "\nUsing default values."
              << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown error: Unable to read settings file. Using default values."
              << std::endl;
  }

  settings.justResponse = settings.responseFile.ends_with(".json");

  if (settings.justResponse & settings.justCompile)
  {
    std::cerr << "only justResponse or justCompile can be set."
              << std::endl;

    exit(1);
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

  // print pretty json by hand, as boost does not pretty print by default.
  // \todo consider using https://www.boost.org/doc/libs/1_80_0/libs/json/doc/html/json/examples.html
  ofs << "{"
      << "\n  \"invokeai\":\""     << defaults.invokeai << "\","
      << "\n  \"optcompiler\":\""  << defaults.optcompiler << "\","
      << "\n  \"optreport\":\""    << defaults.optreport << "\","
      << "\n  \"optcompile\":\""   << defaults.optcompile << "\","
      << "\n  \"justResponse\":"   << as_string(defaults.justResponse) << ","
      << "\n  \"justCompile\":"    << as_string(defaults.justCompile) << ","
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
    else if (arg.rfind(pharness) != std::string::npos)
      opts.harness = arg.substr(pharness.size());
    else
      opts.args.push_back(arg);
  }

  operator CmdLineArgs() && { return std::move(opts); }

  CmdLineArgs opts;

  static std::string phelp;
  static std::string pcreateconfig;
  static std::string pconfig;
  static std::string pharness;
};

std::string CmdLineProc::phelp         = "--help";
std::string CmdLineProc::pcreateconfig = "--create-config";
std::string CmdLineProc::pconfig       = "--config=";
std::string CmdLineProc::pharness      = "--harness-param=";

CmdLineArgs parseArguments(std::vector<std::string> args)
{
  return std::for_each(args.begin(), args.end(), CmdLineProc{});
}

using RevisionBase = std::tuple<std::string, TestResult>;

struct Revision : RevisionBase
{
  using base = RevisionBase;
  using base::base;

  const std::string& fileName() const { return std::get<0>(*this); }
  const TestResult&  result()   const { return std::get<1>(*this); }
};

std::string
qualityText(const std::vector<Revision>& variants)
{
  assert(variants.size() > 2);

  int               el   = variants.size() - 1;
  const long double curr = variants[el].result().score();

  --el;
  long double       prev = variants[el].result().score();

  while (std::isnan(prev) && (el>0))
  {
    --el;
    prev = variants[el].result().score();
  }

  assert(!std::isnan(prev));

  if (std::isnan(curr))
    return " were not assessed";

  if (curr < prev)
    return " improved";

  if (prev < curr)
    return " got worse";

  return " stayed the same";
}


int main(int argc, char** argv)
{
  CmdLineArgs cmdlnargs = parseArguments(getCmdlineArgs(argv+1, argv+argc));

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

  Settings          settings = loadConfig(cmdlnargs.configFileName);
  //~ CompilationResult compres{"", true};
  CompilationResult compres  = compileResult(settings, cmdlnargs.args.back(), cmdlnargs.args);

  if (!compres.success())
  {
    std::cerr << "Input file does not compile: \n"
              << compres.output()
              << std::endl;
    exit(1);
  }

  if (settings.justCompile) return 0;


  json::value           query      = initialPrompt( settings,
                                                    std::move(compres.output()),
                                                    cmdlnargs.args.back()
                                                  );
  int                   iterations = settings.iterations;
  std::vector<Revision> variants;

  variants.emplace_back( cmdlnargs.args.back(),
                         invokeTestScript(settings, cmdlnargs.args.back(), cmdlnargs.harness)
                       );

  while (iterations > 0)
  {
    storeQuery(settings, query);
    invokeAI(settings);

    std::string response = loadAIResponse(settings);
    std::string newFile  = storeGeneratedFile(response, cmdlnargs.args.back());

    query = appendResponse(std::move(query), response);
    compres = compileResult(settings, newFile, cmdlnargs.args);

    if (compres.success())
    {
      variants.emplace_back( newFile,
                             invokeTestScript(settings, newFile, cmdlnargs.harness)
                           );

      if (variants.back().result().success())
      {
        std::cerr << "Compiled and tested, results "
                  << qualityText(variants) << "."
                  << std::endl;

        // \todo add quality assessment to prompt
        query = appendSuccessPrompt(std::move(query), compres.output(), variants.back().fileName());
      }
      else
      {
        std::cerr << "Compiled but regression test failed... " << std::endl;
        // what can we do here?
      }
    }
    else
    {
      // ask to correct the code
      std::cerr << "Generated code did not compile" << std::endl;

      variants.emplace_back( newFile, TestResult{false, nanValue<long double>()} );

      query = appendFailurePrompt(std::move(query), compres.output(), variants.back().fileName());
    }

    --iterations;
  }

  return 0;
}


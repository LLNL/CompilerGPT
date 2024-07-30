
#include <fstream>
#include <string>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/process.hpp>
//~ #include <boost/filesystem.hpp>
#include <boost/json/src.hpp>

#include <filesystem>

namespace json = boost::json;

const std::string CC_MARKER_BEGIN = "```";
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
                    "\n    -h"
                    "\n    -help"
                    "\n    --help            displays this help message and exits."
                    "\n    --help-config     prints config file documentation."
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


const char* confighelp = "The following configuration parameters can be set in the config file."
                         "\n"
                         "\nCompiler and optimization report settings:"
                         "\n  optcompiler    a string pointing to a compiler"
                         "\n  optreport      arguments passed to 'optcompiler' to generate an optimization report"
                         "\n  optcompile     arguments passed to 'optcompiler' to validate compilation"
                         "\n  useOptReport   boolean value. false turns off the use of optimization reports"
                         "\n"
                         "\nInteraction with AI"
                         "\n  invokeai       a string pointing to an executable (script) to call the external AI"
                         "\n  queryFile      a json file storing the conversation history. The external AI"
                         "\n                 needs to read the query from this file."
                         "\n  responseFile   a text file where the AI stores the query response"
                         "\n  inputLang      language delimiter for the input language. Used to generate"
                         "\n                 the initial prompt."
                         "\n  outputLang     language delimiter for the AI response."
                         "\n"
                         "\nCode validation and quality scoring:"
                         "\n  newFileExt     a string for the extension of the generated file."
                         "\n                 if not set, the original file extension will be used."
                         "\n                 note: the setting may be useful for language translation tasks"
                         "\n  testScript     an optional string pointing to an executable that assesses the AI output"
                         "\n                 the test is called with one or two arguments"
                         "\n                    testScript generatedFile [harnessParam]"
                         "\n                       harnessParam is provided by the command line argument --harness-param="
                         "\n                 a non-0 return value indicates testing failure"
                         "\n                 the last output line should contain a quality score (floating point)."
                         "\n                   (lower is better)."
                         "\n                 if the testScript is not set, it is assumed that the generated"
                         "\n                 code passes the regression tests with a quality score of 0."
                         "\n"
                         "\nPrompting:"
                         "\n  contextMessage a string for setting the context/role in the AI communication."
                         "\n  onePromptTask  a string for the initial prompt (onePromptTask code onePromptSteps)"
                         "\n  onePromptSteps a string for the initial prompt (onePromptTask code onePromptSteps)"
                         "\n  compFailPrompt a string when the AI generated code does not compile"
                         "\n  testFailPrompt a string when the AI generated code produces errors with the test harness."
                         "\n"
                         "\nIteration control:"
                         "\n  iterations     integer number specifying the maximum number of iterations."
                         "\n  stopOnSuccess  boolean value. if true, the program terminates as soon as testScript reports success."
                         "\n"
                         "\nGeneral:"
                         "\n  A file with default settings can be generated using --create-config."
                         "\n  The config file only needs entries when a default setting is overridden."
                         ;

struct Settings
{
  std::string  invokeai        = "./scripts/gpt4/execquery.sh";
  std::string  optcompiler     = "/usr/bin/clang";
  std::string  optreport       = "-Rpass-missed=.";
  std::string  optcompile      = "-c";
  std::string  queryFile       = "query.json";
  std::string  responseFile    = "response.txt";
  std::string  testScript      = "";
  std::string  newFileExt      = "";
  std::string  inputLang       = "cpp";
  std::string  outputLang      = "cpp";  // same as input language if not specified
  std::string  contextMessage  = "You are a compiler expert for C++ code optimization. Our goal is to improve the existing code.";
  std::string  onePromptTask   = "Consider the following input code in C++:\n";
  std::string  onePromptSteps  = "Prioritize the optimizations and rewrite the code to enable better compiler optimizations.";
  std::string  compFailPrompt  = "This code did not compile. Here are the error messages, try again.\n";
  std::string  testFailPrompt  = "This version failed the regression tests. Try again.\n";

  bool         jsonResponse    = false;
  bool         useOptReport    = true;
  bool         stopOnSuccess   = false;
  std::int64_t iterations      = 1;
};

struct CmdLineArgs
{
  bool                     help              = false;
  bool                     helpConfig        = false;
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

void splitArgs(const std::string& s, std::vector<std::string>& args)
{
  std::istringstream all{s};
  std::string        arg;

  while (all >> arg)
    args.emplace_back(std::move(arg));
}


CompilationResult
invokeCompiler(const Settings& settings, std::vector<std::string> args)
{
  if (!settings.useOptReport)
  {
    std::cerr << "No compiler configured. Skipping compile test."
              << std::endl;

    return { "", true };
  }

  assert(settings.optcompiler.size() > 0);

  std::string src = std::move(args.back());
  args.pop_back();

  splitArgs(settings.optreport, args);
  splitArgs(settings.optcompile, args);
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
  std::cerr << "success(compile): " << success << std::endl;

  return { errstr.get(), success };
}



CompilationResult
compileResult(const Settings& settings, std::string newFile, std::vector<std::string> args)
{
  args.pop_back();
  args.push_back(newFile);

  return invokeCompiler(settings, args);
}

bool invokeAI(const Settings& settings)
{
  std::cerr << "askAI: " << settings.invokeai << std::endl;

  std::vector<std::string> noargs;
  boost::asio::io_service  ios;
  std::future<std::string> outstr;
  std::future<std::string> errstr;
  std::future<int>         exitCode;
  boost::process::child    ai( settings.invokeai,
                               boost::process::args(noargs),
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

  std::cout << outstr.get() << std::endl;
  std::cerr << errstr.get() << std::endl;

  const bool success = exitCode.get() == 0;
  std::cerr << "success (askAI): " << success << std::endl;

  return success;
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
    std::string_view cand = s.substr(beg, pos-beg);

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

using TestResultBase = std::tuple<bool, long double, std::string>;

struct TestResult : TestResultBase
{
  using base = TestResultBase;
  using base::base;

  bool               success() const { return std::get<0>(*this); }
  long double        score()   const { return std::get<1>(*this); }
  const std::string& errors()  const { return std::get<2>(*this); }
};

std::ostream& operator<<(std::ostream& os, const TestResult& res)
{
  return os << as_string(res.success()) << "  score: " << res.score();
}


TestResult
invokeTestScript(const Settings& settings, const std::string& filename, const std::string& harness)
{
  if (settings.testScript.empty())
  {
    std::cerr << "No test-script configured. Not running tests."
              << std::endl;

    return { true, 0.0, "" };
  }

  std::vector<std::string> args{filename};

  if (!harness.empty())
    args.push_back(harness);

  std::cerr << "test: " << settings.testScript << range(args) << std::endl;

  boost::asio::io_service  ios;
  std::future<std::string> outstr;
  std::future<std::string> errstr;
  std::future<int>         exitCode;
  boost::process::child    tst( settings.testScript,
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

  const bool success = exitCode.get() == 0;

  std::string outs = outstr.get();
  std::string errs = errstr.get();

  std::cout << outs << std::endl;
  std::cerr << errs << std::endl;

  std::cerr << "success(test): " << success << std::endl;
  return { success, testScore(outs), std::move(errs) };
}


std::string loadCodeQuery(const Settings& settings, const std::string& output, const std::string& filename)
{
  std::stringstream txt;
  std::ifstream     src{filename};
  std::string       line;

  txt << settings.onePromptTask << CC_MARKER_BEGIN << settings.inputLang << "\n";

  while(std::getline(src, line)) {
    txt << line << "\n";
  }

  txt << CC_MARKER_LIMIT << '\n';

  if (settings.useOptReport)
  {
    txt << settings.optcompiler << " produces the optimization report:\n"
        << output << '\n';
  }

  txt << settings.onePromptSteps
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
    line["content"] = settings.contextMessage;

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
    q["content"] = loadCodeQuery(settings, output, filename);

    res.emplace_back(std::move(q));
  }

  return res;
}

json::value
appendSuccessPrompt(const Settings& settings, json::value val, std::string output, std::string /*filename*/)
{
  json::array&      res = val.as_array();
  json::object      q;
  std::stringstream txt;

  txt << "The previous code works.\n";

  if (settings.useOptReport)
  {
    txt << "Following is the new optimization report:\n"
        << output << "\n"
        << settings.onePromptSteps
        << std::endl;
  }
  else
  {
    txt << "Optimize the code further.\n";
  }

  txt << std::flush;

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

  txt << output
      << "\nFix the errors in the code.\n"
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
    p.write(line, ec);

    if (ec)
    {
      std::cerr << ec << std::endl;
      throw std::runtime_error("unable to parse JSON file: " + line);
    }
  }


  p.finish(ec);

  std::cerr << ec << std::endl;

  if (ec) throw std::runtime_error("unable to finish parsing JSON file");

  return p.release();
}

std::string generateNewFileName(std::string_view fileName, std::string_view newFileExt, int iteration)
{
  std::size_t pos = fileName.find_last_of('.');
  std::string res;

  if (pos == std::string::npos)
  {
    res = fileName;
    res.append(std::to_string(iteration));
    res.append(newFileExt);
    std::copy(newFileExt.begin(), newFileExt.end(), std::back_inserter(res));
  }
  else
  {
    const auto beg = fileName.begin();

    if (newFileExt.size() == 0)
      newFileExt = std::string_view(beg+pos, fileName.end());

    std::copy(beg, beg+pos, std::back_inserter(res));
    res.append(std::to_string(iteration));

    std::copy(newFileExt.begin(), newFileExt.end(), std::back_inserter(res));
  }

  if (std::filesystem::exists(res))
    return generateNewFileName(fileName, newFileExt, iteration+1);

  return res;
}


std::string
storeGeneratedFile( const Settings& settings,
                    std::string_view response,
                    std::string_view fileName,
                    int iteration = 1
                  )
{
  const std::string   marker    = CC_MARKER_BEGIN + settings.outputLang;
  const std::string   newFile   = generateNewFileName(fileName, settings.newFileExt, iteration);
  const std::size_t   mark      = response.find(marker);
  if (mark == std::string::npos)
  {
    std::cerr << response
              << "\n  missing code marker " << marker
              << std::endl;
    throw std::runtime_error{"Cannot find code markers in AI output."};
  }

  const std::size_t   beg       = mark + marker.size();
  const std::size_t   lim       = response.find(CC_MARKER_LIMIT, beg);
  if (beg == std::string::npos)
  {
    std::cerr << response
              << "\n  missing code delimiter " << CC_MARKER_LIMIT
              << std::endl;

    throw std::runtime_error{"Cannot find code delimiter in AI output."};
  }

  std::string_view    code = response.substr(beg, lim - beg);
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
    else if ((last == '\\') && (ch == '\''))
    {
      outf << "'";
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

  json::value         obj       = readJsonFile(responseFile);
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

    config.invokeai        = loadField(cnfobj, "invokeai",        config.invokeai);
    config.optcompiler     = loadField(cnfobj, "optcompiler",     config.optcompiler);
    config.optreport       = loadField(cnfobj, "optreport",       config.optreport);
    config.optcompile      = loadField(cnfobj, "optcompile",      config.optcompile);
    config.queryFile       = loadField(cnfobj, "queryFile",       config.queryFile);
    config.responseFile    = loadField(cnfobj, "responseFile",    config.responseFile);
    config.testScript      = loadField(cnfobj, "testScript",      config.testScript);
    config.newFileExt      = loadField(cnfobj, "newFileExt",      config.newFileExt);
    config.inputLang       = loadField(cnfobj, "inputLang",       config.inputLang);
    config.outputLang      = loadField(cnfobj, "outputLang",      config.inputLang); // out is in if not set
    config.onePromptTask   = loadField(cnfobj, "onePromptTask",   config.onePromptTask);
    config.onePromptSteps  = loadField(cnfobj, "onePromptSteps",  config.onePromptSteps);
    config.compFailPrompt  = loadField(cnfobj, "compFailPrompt",  config.compFailPrompt);
    config.testFailPrompt  = loadField(cnfobj, "testFailPrompt",  config.testFailPrompt);
    config.contextMessage  = loadField(cnfobj, "contextMessage",  config.contextMessage);
    config.useOptReport    = loadField(cnfobj, "useOptReport",    config.useOptReport);
    config.stopOnSuccess   = loadField(cnfobj, "stopOnSuccess",   config.stopOnSuccess);
    config.iterations      = loadField(cnfobj, "iterations",      config.iterations);

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

  return settings;
}

void writeSettings(std::ostream& os, const Settings& settings)
{
  // print pretty json by hand, as boost does not pretty print by default.
  // \todo consider using https://www.boost.org/doc/libs/1_80_0/libs/json/doc/html/json/examples.html
  os << "{"
     << "\n  \"invokeai\":\""       << settings.invokeai << "\","
     << "\n  \"optcompiler\":\""    << settings.optcompiler << "\","
     << "\n  \"optreport\":\""      << settings.optreport << "\","
     << "\n  \"optcompile\":\""     << settings.optcompile << "\","
     << "\n  \"queryFile\":\""      << settings.queryFile << "\","
     << "\n  \"responseFile\":\""   << settings.responseFile << "\"" << ","
     << "\n  \"testScript\":\""     << settings.testScript << "\"" << ","
     << "\n  \"newFileExt\":\""     << settings.newFileExt << "\"" << ","
     << "\n  \"inputLang\":\""      << settings.inputLang << "\"" << ","
     << "\n  \"outputLang\":\""     << settings.outputLang << "\"" << ","
     << "\n  \"contextMessage\":\"" << settings.contextMessage << "\"" << ","
     << "\n  \"onePromptTask\":\""  << settings.onePromptTask << "\"" << ","
     << "\n  \"onePromptSteps\":\"" << settings.onePromptSteps << "\"" << ","
     << "\n  \"compFailPrompt\":\"" << settings.compFailPrompt << "\"" << ","
     << "\n  \"testFailPrompt\":\"" << settings.testFailPrompt << "\"" << ","
     << "\n  \"useOptReport\":"     << as_string(settings.useOptReport) << ","
     << "\n  \"stopOnSuccess\":"    << as_string(settings.stopOnSuccess) << ","
     << "\n  \"iterations\":"       << settings.iterations
     << "\n}" << std::endl;
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

  writeSettings(ofs, defaults);
}


struct CmdLineProc
{
  void operator()(const std::string& arg)
  {
    if (arg.rfind(phelpconfig) != std::string::npos)
      opts.helpConfig = true;
    else if (arg.rfind(phelp) != std::string::npos)
      opts.help = true;
    else if (arg.rfind(phelp2) != std::string::npos)
      opts.help = true;
    else if (arg.rfind(phelp3) != std::string::npos)
      opts.help = true;
    else if (arg.rfind(pcreateconfig) != std::string::npos)
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
  static std::string phelp2;
  static std::string phelp3;
  static std::string phelpconfig;
  static std::string pcreateconfig;
  static std::string pconfig;
  static std::string pharness;
};

std::string CmdLineProc::phelp         = "--help";
std::string CmdLineProc::phelp2        = "-help";
std::string CmdLineProc::phelp3        = "-h";
std::string CmdLineProc::phelpconfig   = "--help-config";
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

std::ostream& operator<<(std::ostream& os, const Revision& rev)
{
  return os << rev.fileName() << ": " << rev.result();
}


std::string
qualityText(const std::vector<Revision>& variants)
{
  assert(!variants.empty());

  if (variants.size() == 1)
    return "initial result";

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

  if (cmdlnargs.helpConfig)
  {
    std::cout << confighelp << std::endl
              << std::endl;
    return 0;
  }


  if (cmdlnargs.createConfig)
  {
    createDefaultConfig(cmdlnargs.configFileName);
    return 0;
  }

  Settings          settings   = loadConfig(cmdlnargs.configFileName);

  std::cout << "Settings: ";
  writeSettings(std::cout, settings);
  std::cout << std::endl;

  CompilationResult compres    = compileResult(settings, cmdlnargs.args.back(), cmdlnargs.args);
  int               iterations = settings.iterations;

  std::cerr << "compiled " << compres.success() << std::endl;

  if (!compres.success())
  {
    std::cerr << "Input file does not compile: \n"
              << compres.output()
              << std::endl;

    // do we really need to exit; maybe compiler GPT can fix the output..
    exit(1);
  }

  std::vector<Revision> variants;

  variants.emplace_back( cmdlnargs.args.back(),
                         invokeTestScript(settings, cmdlnargs.args.back(), cmdlnargs.harness)
                       );

  if (!variants.back().result().success())
    std::cerr << "warning: baseline test failed!" << std::endl;

  // initial prompt
  json::value           query;

  std::cerr << "iterations = " << iterations << std::endl;

  if (iterations > 0)
    query = initialPrompt( settings, std::move(compres.output()), cmdlnargs.args.back());

  while (iterations > 0)
  {
    --iterations;

    const bool promptAI = iterations != 0;

    storeQuery(settings, query);
    invokeAI(settings);

    std::string response = loadAIResponse(settings);
    std::string newFile  = storeGeneratedFile(settings, response, cmdlnargs.args.back());

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

        if (settings.stopOnSuccess)
        {
          iterations = 0;
        }
        else if (promptAI)
        {
          // \todo add quality assessment to prompt
          query = appendSuccessPrompt(settings, std::move(query), compres.output(), variants.back().fileName());
        }
      }
      else
      {
        std::cerr << "Compiled but test failed... " << std::endl;

        if (promptAI)
        {
          // \todo append output here..
          std::string prompt = settings.testFailPrompt;

          prompt += variants.back().result().errors();
          query = appendFailurePrompt(std::move(query), prompt, variants.back().fileName());
        }
      }
    }
    else
    {
      // ask to correct the code
      std::cerr << "Compilation failed..." << std::endl;

      variants.emplace_back( newFile, TestResult{false, nanValue<long double>(), compres.output()} );

      if (promptAI)
      {
        std::string prompt = settings.compFailPrompt;

        prompt += compres.output();
        query = appendFailurePrompt(std::move(query), prompt, variants.back().fileName());
      }
    }
  }

  //
  // prepare final report

  // store query and results for review
  storeQuery(settings, query);

  for (const Revision& r : variants)
    std::cout << r << std::endl;

  // go over results and rank them based on success and results

  return 0;
}


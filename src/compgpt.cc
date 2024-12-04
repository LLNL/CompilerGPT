
#include <fstream>
#include <string>
#include <iostream>
#include <numeric>
#include <charconv>
#include <regex>

#include <boost/asio.hpp>
#include <boost/process.hpp>
//~ #include <boost/filesystem.hpp>
#include <boost/json/src.hpp>
#include <boost/algorithm/string.hpp>

//~ #include <boost/algorithm/string/predicate.hpp>

#include <filesystem>

namespace json = boost::json;

const std::string CC_MARKER_BEGIN = "```";
const std::string CC_MARKER_LIMIT = "```";

namespace
{

const char* synopsis = "compgpt: code optimizations through LLMs"
                       "\n  description: feeds clang optimization report of a kernel to"
                       "\n               an LLM and asks it to optimize the source code"
                       "\n               The rewritten part is extracted from the LLM"
                       "\n               and stored in a file."
                       "\n               The file is compiled and checked for software"
                       "\n               bugs and performance using a specified test"
                       "\n               harness.";

const char* usage = "usage: compgpt switches source-file"
                    "\n  switches:"
                    "\n    -h"
                    "\n    -help"
                    "\n    --help            displays this help message and exits."
                    "\n    --help-config     prints config file documentation."
                    "\n    --config=jsonfile config file in json format."
                    "\n                      default: jsonfile=compgpt.json"
                    "\n    --create-config   creates config file and exits."
                    "\n    --create-config=p creates config file for a specified AI model, and exits."
                    "\n                      p in {gpt4,claude}"
                    "\n                      default: p=gpt4"
                    "\n    --harness-param=p sets an optional parameter for the test harness."
                    "\n                      default: none"
                    "\n                      If set, the parameter is passed as second"
                    "\n                      argument to the test script configured in"
                    "\n                      the config file."
                    "\n                      The tester is invoked with two arguments,"
                    "\n                      the generated file and param:"
                    "\n                        testScript genfile.cc p"
                    "\n    --kernel=range    chooses a specific code segment for optimization."
                    "\n                      range is specified in terms of line and optional column"
                    "\n                      number."
                    "\n                      The following are examples of valid options:"
                    "\n                        0-10    Lines 0-10 (excluding Line 10)."
                    "\n                        7:4-10  Lines 7 (starting at column 4) to Line 10."
                    "\n                        7:2-10:8 Lines 7 (starting at column 2) to Line 10"
                    "\n                                 (up to column 8)."
                    "\n                      default: the whole input file"
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
                         "\n  responseFile   a file [text or json] where the AI stores the query response"
                         "\n  responseField  a JSON path in the form of [field {'.' field} ] identifying the content."
                         "\n                 ignored when responseFile is a text file."
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
                         "\n  systemText     a string for setting the context/role in the AI communication."
                         "\n  systemTextFile if set compgpt writes the system text into a file instead of"
                         "\n                 passing it as first message in the message list."
                         "\n  roleOfAI       the name of the AI role in the conversation context."
                         "\n  onePromptTask  a string for the initial prompt (onePromptTask code onePromptSteps)."
                         "\n  onePromptSteps a string for the initial prompt (onePromptTask code onePromptSteps)."
                         "\n  compFailPrompt a string when the AI generated code does not compile (compFailPrompt compileErrors compFailSteps)."
                         "\n  compFailSteps  a string when the AI generated code does not compile (compFailPrompt compileErrors compFailSteps)."
                         "\n  testFailPrompt a string when the AI generated code produces errors with the test harness (testFailPrompt eval testFailPSteps)"
                         "\n  testFailSteps  a string when the AI generated code produces errors with the test harness (testFailPrompt eval testFailPSteps)"
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
  std::string  invokeai       = "./scripts/gpt4/execquery.sh";
  std::string  optcompiler    = "/usr/bin/clang";
  std::string  optreport      = "-Rpass-missed=.";
  std::string  optcompile     = "-c";
  std::string  queryFile      = "query.json";
  std::string  responseFile   = "response.txt";
  std::string  responseField  = "";
  std::string  testScript     = "";
  std::string  newFileExt     = "";
  std::string  inputLang      = "cpp";
  std::string  outputLang     = "cpp";  // same as input language if not specified
  std::string  systemText     = "You are a compiler expert for C++ code optimization. Our goal is to improve the existing code.";
  std::string  roleOfAI       = "system";
  std::string  systemTextFile = "";
  std::string  onePromptTask  = "Consider the following input code in C++:\n";
  std::string  onePromptSteps = "Prioritize the optimizations and rewrite the code to enable better compiler optimizations.";
  std::string  compFailPrompt = "This code did not compile. Here are the error messages:\n";
  std::string  compFailSteps  = "Try again\n";
  std::string  testFailPrompt = "This version failed the regression tests. Here is the evaluation: \n";
  std::string  testFailSteps  = "Try again.\n";

  bool         useOptReport   = true;
  bool         stopOnSuccess  = false;
  std::int64_t iterations     = 1;
};

Settings setupGPT4(Settings settings)
{
  return settings;
}

Settings setupClaude(Settings settings)
{
  settings.responseFile   = "response.json";
  settings.responseField  = "content[0].text";
  settings.systemTextFile = "system.txt";
  settings.roleOfAI       = "assistant";
  settings.invokeai       = "./scripts/claude/exec-claude.sh";

  return settings;
}

using SourcePointBase = std::tuple<std::size_t, std::size_t>;
struct SourcePoint : SourcePointBase
{
  using base = SourcePointBase;
  using base::base;

  std::size_t line() const { return std::get<0>(*this); }
  std::size_t col()  const { return std::get<1>(*this); }

  static
  SourcePoint origin();

  static
  SourcePoint eof();
};

// static
SourcePoint
SourcePoint::origin()
{
  return { 0, 0 };
}

// static
SourcePoint
SourcePoint::eof()
{
  return { std::numeric_limits<std::size_t>::max(),
           std::numeric_limits<std::size_t>::max()
         };
}

using SourceRangeBase = std::tuple<SourcePoint, SourcePoint>;
struct SourceRange : SourceRangeBase
{
  using base = SourceRangeBase;
  using base::base;

  SourcePoint beg() const { return std::get<0>(*this); }
  SourcePoint lim() const { return std::get<1>(*this); }

  bool entireFile() const
  {
    return (  (beg() == SourcePoint::origin())
           && (lim() == SourcePoint::eof())
           );
  }
};



struct CmdLineArgs
{
  enum Model { none = 0, error = 1, gpt4 = 2, claude = 3 };

  bool                     help              = false;
  bool                     helpConfig        = false;
  Model                    configModel       = none;
  std::string              configFileName    = "compgpt.json";
  std::string              harness           = "";
  SourceRange              kernel            = { SourcePoint::origin(), SourcePoint::eof() };
  std::vector<std::string> all;
};

const char* as_string(bool v, bool align = false)
{
  if (!v) return "false";
  if (!align) return "true";

  return "true ";
}

Settings modelDefaults(CmdLineArgs::Model m)
{
  Settings defaults;

  if (m == CmdLineArgs::gpt4)   return setupGPT4(defaults);
  if (m == CmdLineArgs::claude) return setupClaude(defaults);

  throw std::runtime_error{"Cannot configure unknown model."};
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

std::vector<std::string>
splitString(const std::string& input, char splitch = '\n')
{
  std::vector<std::string> res;
  std::size_t curr = 0;
  std::size_t next = 0;

  while ( (next = input.find(splitch, curr)) != std::string::npos )
  {
    res.emplace_back(input.substr(curr, next - curr));
    curr = next + 1;
  }

  // Handle the last line if it doesn't end with a newline
  if (curr < input.length())
    res.emplace_back(input.substr(curr));

  return res;
}

using DiagnosticBase = std::tuple<std::string, SourcePoint, std::vector<std::string> >;
struct Diagnostic : DiagnosticBase
{
  using base = DiagnosticBase;
  using base::base;

  const std::string&              file()     const { return std::get<0>(*this); }
  SourcePoint                     location() const { return std::get<1>(*this); }
  const std::vector<std::string>& message()  const { return std::get<2>(*this); }

  bool empty() const { return message().empty(); }

  std::string&              file()     { return std::get<0>(*this); }
  SourcePoint&              location() { return std::get<1>(*this); }
  std::vector<std::string>& message()  { return std::get<2>(*this); }
};

std::tuple<std::string, SourcePoint>
fileLocation(const std::string& s)
{
  const std::regex patLocation{"(.*):([0-9]*):([0-9]*)"};
  std::smatch      matches;

  if (std::regex_search(s, matches, patLocation))
  {
    assert(matches.size() == 4);
    return { matches[1], { std::stoi(matches[2]), std::stoi(matches[3]) } };
  }

  return {};
}


struct DiagnosticFilter2
{
  void appendCurrentDiagnostic()
  {
    diagnosed.emplace_back();
    diagnosed.back().swap(curr);
  }

  void operator()(const std::string& s)
  {
    const bool  isIncludeTrace = s.rfind("In file included from", 0) == 0;

    if (isIncludeTrace)
      return;

    std::string file;
    SourcePoint loc;

    std::tie(file, loc) = fileLocation(s);

    const bool  containsSourceLoc = !file.empty();

    if (containsSourceLoc)
    {
      appendCurrentDiagnostic();

      curr = Diagnostic{ file, loc, {} };
    }

    curr.message().push_back(s);
  }

  operator std::vector<Diagnostic>() &&
  {
    appendCurrentDiagnostic();
    return std::move(diagnosed);
  }

  Diagnostic              curr;
  std::vector<Diagnostic> diagnosed;
};

#if 0
struct DiagnosticFilter
{
  // returns the line number (expected to be at the beginning of line).
  //   if no line number can be found return max(std::size_t) to indicate
  //   a non match.
  std::size_t lineNumber(std::string_view line) const
  {
    if (line.at(0) == ':') line.remove_prefix(1);

    std::size_t res;
    auto const  result = std::from_chars(line.data(), line.data() + line.size(), res);

    if (result.ptr == line.data())
    {
      std::cerr << "unable to find line number in: " << line
                << std::endl;
      return std::numeric_limits<std::size_t>::max();
    }

    return res;
  }

  bool isMainFile(std::string_view line) const
  {
    std::size_t const pos = line.find(mainFile);

    if (pos == std::string::npos)
      return false;

    if (rng.empty())
      return true;

    line.remove_prefix(pos+mainFile.size());

    std::size_t const ln = lineNumber(line);
    std::cerr << "ln = " << ln << std::endl;

    return (rng.beg().line() <= ln) && (ln < rng.lim().line());
  };

  void moveMessageToResult()
  {
    auto const beg = std::make_move_iterator(pendingMessageTrace.begin());
    auto const lim = std::make_move_iterator(pendingMessageTrace.end());

    std::copy( beg, lim, std::back_inserter(res) );
    pendingMessageTrace.clear();
    pendingIncludeTrace.clear();
  }

  void operator()(const std::string& s)
  {
    const bool isIncludeTrace = s.rfind("In file included from", 0) == 0;

    if (!isIncludeTrace)
    {
      if (isMainFile(s))
      {
        if (refsMainFile)
        {
          std::cerr << "+m1 " << s << std::endl;
          moveMessageToResult();
        }
        else
        {
          std::cerr << "-c1 " << s << std::endl;
          pendingMessageTrace.clear();
          refsMainFile = true;
        }
      }
      else if (refsMainFile)
      {
        std::cerr << "+m2 " << s << std::endl;
        moveMessageToResult();
        refsMainFile = false;
      }
      //~ else
      //~ {
        //~ std::cerr << "-c2 " << s << std::endl;
        //~ pendingMessageTrace.clear();
      //~ }

      std::cerr << "+p1 " << s << std::endl;
      pendingMessageTrace.push_back(s);
    }
    else if (pendingMessageTrace.empty())
    {
      std::cerr << "+p2 " << s << std::endl;
      pendingIncludeTrace.push_back(s);
    }
    else if (refsMainFile)
    {
      refsMainFile = false;
      std::cerr << "+m2 " << std::endl;
      moveMessageToResult();
    }
  }

  operator std::vector<std::string>() &&
  {
    moveMessageToResult();
    return std::move(res);
  }

  std::string_view         mainFile;
  SourceRange              rng;
  bool                     refsMainFile         = false;
  std::vector<std::string> pendingIncludeTrace  = {};
  std::vector<std::string> pendingMessageTrace  = {};
  std::vector<std::string> res                  = {};
};
#endif

// std::string

CompilationResult
filterMessageOutput(const std::string& out, std::string_view filename, SourceRange rng, bool success)
{
  if (!success) return { out, success };

  std::vector<std::string> lines = splitString(out);
  std::vector<Diagnostic>  diagnosed = std::for_each( lines.begin(), lines.end(),
                                                      //~ DiagnosticFilter{filename, rng}
                                                      DiagnosticFilter2{}
                                                    );

  auto outsideSourceRange =
           [rng, filename](const Diagnostic& el) -> bool
           {
             if (el.file().find(filename) == std::string::npos)
               return true;

             return (  (rng.beg()     <= el.location())
                    && (el.location() <  rng.lim())
                    );
           };
  auto beg = diagnosed.begin();
  auto pos = std::remove_if( beg, diagnosed.end(), outsideSourceRange );

  return { std::accumulate( beg, pos,
                            std::string{},
                            [](std::string lhs, const Diagnostic& rhs) -> std::string
                            {
                              for (const std::string& s : rhs.message())
                              {
                                lhs += '\n';
                                lhs += s;
                              }

                              return lhs;
                            }
                          ),
           success
         };
}


CompilationResult
invokeCompiler(const Settings& settings, const CmdLineArgs& cmdline, std::vector<std::string> args)
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

  return filterMessageOutput(errstr.get(), args.back(), cmdline.kernel, success);
}

CompilationResult
compileResult(const Settings& settings, const CmdLineArgs& cmdline, std::string newFile)
{
  std::vector<std::string> args = cmdline.all;

  args.pop_back();
  args.push_back(newFile);

  CompilationResult res = invokeCompiler(settings, cmdline, args);

  std::cerr << res.output() << std::endl;

  return res;
}

CompilationResult
compileResult(const Settings& settings, const CmdLineArgs& cmdline)
{
  return compileResult(settings, cmdline, cmdline.all.back());
}


void invokeAI(const Settings& settings)
{
  std::cerr << "!CallAI: " << settings.invokeai << std::endl;
  return;

  std::cerr << "CallAI: " << settings.invokeai << std::endl;

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

  const int  ec      = exitCode.get();

  std::cerr << "CallAI - exitcode: " << ec << std::endl;

  if (ec != 0) throw std::runtime_error{"AI invocation error."};
}

template <class F>
F nanValue() { return std::numeric_limits<F>::quiet_NaN(); }

/// reads the string s and returns the numeric value of the last
///   non-empty line.
long double
testScore(bool success, std::string_view s)
{
  if (!success) return nanValue<long double>();

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

struct TestResultPrinter
{
  const TestResult& obj;
};

std::ostream& operator<<(std::ostream& os, const TestResultPrinter& el)
{
  return os << as_string(el.obj.success(), true) << "  score: " << el.obj.score();
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
  return { success, testScore(success, outs), std::move(errs) };
}


std::string loadCodeQuery( const Settings& settings,
                           const std::string& output,
                           const std::string& filename,
                           SourceRange rng
                         )
{
  std::stringstream txt;

  txt << settings.onePromptTask;

  if (!rng.entireFile())
    txt << "The code's first line number is: " << rng.beg().line()
        << "\n";

  txt << CC_MARKER_BEGIN << settings.inputLang
      << "\n";

  std::ifstream     src{filename};
  std::string       line;
  std::size_t const begLine = rng.beg().line();
  std::size_t const limLine = rng.lim().line();
  std::size_t       linectr = 1; // source code starts at line 1

  // skip beginning lines
  while ((linectr < begLine) && std::getline(src, line)) ++linectr;

  // copy code segment
  while ((linectr < limLine) && std::getline(src, line))
  {
    ++linectr;
    txt << line << "\n";
  }

  txt << CC_MARKER_LIMIT << '\n';

  if (settings.useOptReport)
  {
    txt << settings.optcompiler << " produces the following optimization report:\n"
        << output << '\n';
  }

  txt << settings.onePromptSteps
      << std::flush;

  return txt.str();
}

json::value initialPrompt(const Settings& settings, const CmdLineArgs& args, std::string output)
{
  // do not generate a prompt in this case
  if (settings.iterations == 0)
    return {};

  json::array res;

  if (settings.systemTextFile.empty())
  {
    json::object line;

    line["role"]    = "system";
    line["content"] = settings.systemText;

    res.emplace_back(std::move(line));
  }
  else
  {
    std::ofstream ofs{settings.systemTextFile};

    ofs << settings.systemText;
  }

  {
    json::object q;

    q["role"]    = "user";
    q["content"] = loadCodeQuery(settings, output, args.all.back(), args.kernel);

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
appendResponse(const Settings& settings, json::value val, std::string response)
{
  json::array& res = val.as_array();
  json::object line;

  line["role"]    = settings.roleOfAI;
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

void
printUnescaped(std::ostream& os, std::string_view code)
{
  char last = ' ';

  // print to os while handling escaped characters
  for (char ch : code)
  {
    if (last == '\\')
    {
      switch (ch)
      {
        case 'f':  /* form feed */
                   os << "\n\n";
                   break;

        case 'n':  os << '\n';
                   break;

        case 't':  os << "  ";
                   break;

        case 'a':  /* bell */
        case 'v':  /* vertical tab */
        case 'r':  /* carriage return */
                   break;

        case '\'':
        case '"' :
        case '?' :
        case '\\': os << ch;
                   break;

        default:   os << last << ch;
      }

      last = ' ';
    }
    else if (ch == '\\')
      last = ch;
    else
      os << ch;
  }
}

void
copyFromOriginalFile( std::string_view fileName,
                      std::ostream& os,
                      SourcePoint beg,
                      SourcePoint lim
                    )
{
  std::filesystem::path p{fileName};
  std::ifstream         inp(p);
  std::string           line;
  std::size_t           linectr = 0;

  while ((linectr < beg.line()) && std::getline(inp, line)) ++linectr;

  // copy code segment
  while ((linectr < lim.line()) && std::getline(inp, line))
  {
    ++linectr;
    os << line << std::endl;
  }
}

std::string
storeGeneratedFile( const Settings& settings,
                    const CmdLineArgs& cmdline,
                    std::string_view response
                  )
{
  std::string_view    fileName  = cmdline.all.back();
  const int           iteration = 1;
  const std::string   newFile   = generateNewFileName(fileName, settings.newFileExt, iteration);
  const std::string   marker    = CC_MARKER_BEGIN + settings.outputLang;
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

  std::ofstream outf{newFile};

  copyFromOriginalFile(fileName, outf, SourcePoint::origin(), cmdline.kernel.beg());
  printUnescaped(outf, response.substr(beg, lim - beg));
  copyFromOriginalFile(fileName, outf, cmdline.kernel.lim(), SourcePoint::eof());

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

std::string jsonField(const json::value& val, std::string_view fld)
{
  std::cerr << '{' << val << '}'
            << "\n'" << fld
            << std::endl;

  if (fld.empty())
  {
    const json::string& content = val.as_string();
    std::string_view    contView(content.begin(), content.size());

    return std::string{contView};
  }

  if (fld[0] == '.')
    return jsonField(val, fld.substr(1));

  if (fld[0] == '[')
  {
    // must be an array index
    const std::size_t   lim = fld.find_first_of("]");
    assert((lim > 0) && (lim != std::string_view::npos));
    std::string_view    idx = fld.substr(1, lim-1);
    const json::array&  arr = val.as_array();
    int                 num = 0;
    auto                [ptr, ec] = std::from_chars(idx.data(), idx.data() + idx.size(), num);

    std::cerr << "i'" << idx << " " << lim << std::endl;

    if (ec != std::errc{})
      throw std::runtime_error{"Not a valid json array index (int expected)"};

    return jsonField(arr.at(num), fld.substr(lim+1));
  }

  const std::size_t pos = fld.find_first_of(".[");

  if (pos == std::string_view::npos)
  {
    const json::object& obj = val.as_object();

    return jsonField(obj.at(fld), std::string_view{});
  }

  assert(pos != 0);
  const json::object& obj = val.as_object();
  std::string_view    lhs = fld.substr(0, pos);

  return jsonField(obj.at(lhs), fld.substr(pos));
}


std::string loadAIResponseJson(const Settings& settings)
{
  std::ifstream responseFile{settings.responseFile};

  return jsonField(readJsonFile(responseFile), settings.responseField);
}


std::string loadAIResponseTxt(const Settings& settings)
{
  std::ifstream responseFile{settings.responseFile};

  return readTxtFile(responseFile);
}

std::string loadAIResponse(const Settings& settings)
{
  const std::string jsonSuffix{".JSON"};

  if (boost::iends_with(settings.responseFile, jsonSuffix))
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


std::string replace_nl(std::string s)
{
  boost::replace_all(s, "\n", "\\n");

  return s;
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
     << "\n  \"responseField\":\""  << settings.responseField << "\"" << ","
     << "\n  \"testScript\":\""     << settings.testScript << "\"" << ","
     << "\n  \"newFileExt\":\""     << settings.newFileExt << "\"" << ","
     << "\n  \"inputLang\":\""      << settings.inputLang << "\"" << ","
     << "\n  \"outputLang\":\""     << settings.outputLang << "\"" << ","
     << "\n  \"systemText\":\""     << replace_nl(settings.systemText) << "\"" << ","
     << "\n  \"roleOfAI\":\""       << settings.roleOfAI << "\"" << ","
     << "\n  \"systemTextFile\":\"" << settings.systemTextFile << "\"" << ","
     << "\n  \"onePromptTask\":\""  << replace_nl(settings.onePromptTask) << "\"" << ","
     << "\n  \"onePromptSteps\":\"" << replace_nl(settings.onePromptSteps) << "\"" << ","
     << "\n  \"compFailPrompt\":\"" << replace_nl(settings.compFailPrompt) << "\"" << ","
     << "\n  \"compFailSteps\":\""  << replace_nl(settings.compFailSteps) << "\"" << ","
     << "\n  \"testFailPrompt\":\"" << replace_nl(settings.testFailPrompt) << "\"" << ","
     << "\n  \"testFailSteps\":\""  << replace_nl(settings.testFailSteps) << "\"" << ","
     << "\n  \"useOptReport\":"     << as_string(settings.useOptReport) << ","
     << "\n  \"stopOnSuccess\":"    << as_string(settings.stopOnSuccess) << ","
     << "\n  \"iterations\":"       << settings.iterations
     << "\n}" << std::endl;
}


Settings loadConfig(const std::string& configFileName)
{
  Settings      settings;
  std::ifstream configFile{configFileName};

  if (!configFile.good())
  {
    std::cerr << "The file " << configFileName << " is NOT ACCESSIBLE (or does not exist.)"
              << "\n  => Using default values."
              << std::endl;
    return settings;
  }

  try
  {
    json::value   cnf    = readJsonFile(configFile);
    json::object& cnfobj = cnf.as_object();
    Settings      config;

    config.invokeai       = loadField(cnfobj, "invokeai",        config.invokeai);
    config.optcompiler    = loadField(cnfobj, "optcompiler",     config.optcompiler);
    config.optreport      = loadField(cnfobj, "optreport",       config.optreport);
    config.optcompile     = loadField(cnfobj, "optcompile",      config.optcompile);
    config.queryFile      = loadField(cnfobj, "queryFile",       config.queryFile);
    config.responseFile   = loadField(cnfobj, "responseFile",    config.responseFile);
    config.responseField  = loadField(cnfobj, "responseField",   config.responseField);
    config.testScript     = loadField(cnfobj, "testScript",      config.testScript);
    config.newFileExt     = loadField(cnfobj, "newFileExt",      config.newFileExt);
    config.inputLang      = loadField(cnfobj, "inputLang",       config.inputLang);
    config.outputLang     = loadField(cnfobj, "outputLang",      config.inputLang); // out is in if not set
    config.onePromptTask  = loadField(cnfobj, "onePromptTask",   config.onePromptTask);
    config.onePromptSteps = loadField(cnfobj, "onePromptSteps",  config.onePromptSteps);
    config.compFailPrompt = loadField(cnfobj, "compFailPrompt",  config.compFailPrompt);
    config.compFailSteps  = loadField(cnfobj, "compFailSteps",   config.compFailSteps);
    config.testFailPrompt = loadField(cnfobj, "testFailPrompt",  config.testFailPrompt);
    config.testFailSteps  = loadField(cnfobj, "testFailSteps",   config.testFailSteps);
    config.systemText     = loadField(cnfobj, "systemText",      config.systemText);
    config.systemTextFile = loadField(cnfobj, "systemTextFile",  config.systemTextFile);
    config.roleOfAI       = loadField(cnfobj, "roleOfAI",        config.roleOfAI);
    config.useOptReport   = loadField(cnfobj, "useOptReport",    config.useOptReport);
    config.stopOnSuccess  = loadField(cnfobj, "stopOnSuccess",   config.stopOnSuccess);
    config.iterations     = loadField(cnfobj, "iterations",      config.iterations);

    settings = std::move(config);
  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what()
              << "\n  => Using default values."
              << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown error: Unable to read settings file."
              << "\n  => Using default values."
              << std::endl;
  }

  std::cerr << "---" << std::endl;
  writeSettings(std::cerr, settings);
  return settings;
}


void createDefaultConfig(const CmdLineArgs& args)
{
  if (std::filesystem::exists(args.configFileName))
  {
    std::cerr << "config file " << args.configFileName << " exists."
              << "\n  not creating a new file! (delete file or choose different file name)"
              << std::endl;

    return;
  }

  std::ofstream ofs(args.configFileName);

  writeSettings(ofs, modelDefaults(args.configModel));
}


struct CmdLineProc
{
  CmdLineArgs::Model
  parseModel(std::string_view m)
  {
    if (m == "gpt4")   return CmdLineArgs::gpt4;
    if (m == "claude") return CmdLineArgs::claude;

    return CmdLineArgs::error;
  }

  std::tuple<std::size_t, std::string_view>
  parseNum(std::string_view s, std::function<std::string_view(std::string_view)> follow)
  {
    std::size_t res = 0;

    while (!s.empty() && (s[0] >= '0') && (s[0] <= '9'))
    {
      res = res*10 + s[0] - '0';
      s.remove_prefix(1);
    }

    return {res, follow(s)};
  }

  std::function<std::string_view(std::string_view)>
  parseChar(char c)
  {
    return [c](std::string_view s) -> std::string_view
           {
             if ( (s.size() < 1) || (s[0] != c) )
               throw std::runtime_error{"unable to parse integer"};

             s.remove_prefix(1);
             return s;
           };
  }

  std::function<std::string_view(std::string_view)>
  parseOptionalChar(char c)
  {
    return [c](std::string_view s) -> std::string_view
           {
             if ( (s.size() < 1) || (s[0] != c) )
               return s;

             s.remove_prefix(1);
             return s;
           };
  }

  std::tuple<SourcePoint, std::string_view>
  parseSourcePoint(std::string_view s0, std::function<std::string_view(std::string_view)> follow)
  {
    auto [ln, s1] = parseNum(s0, parseOptionalChar(':'));
    auto [cl, s2] = parseNum(s1, follow);

    return { {ln,cl}, s2 };
  }

  SourceRange
  parseSourceRange(std::string_view s0)
  {
    auto [beg, s1] = parseSourcePoint(s0, parseChar('-'));
    auto [lim, s2] = parseSourcePoint(s1, parseOptionalChar(';'));

    if (!s2.empty())
    {
      std::cerr << "source range has trailing characters; source range ignored."
                << std::endl;
      return {SourcePoint{},SourcePoint{}};
    }

    return {beg,lim};
  }

  void operator()(const std::string& arg)
  {
    if (arg.rfind(phelpconfig, 0) != std::string::npos)
      opts.helpConfig = true;
    else if (arg.rfind(phelp, 0) != std::string::npos)
      opts.help = true;
    else if (arg.rfind(phelp2, 0) != std::string::npos)
      opts.help = true;
    else if (arg.rfind(phelp3, 0) != std::string::npos)
      opts.help = true;
    else if (arg.rfind(pcreateconfig2, 0) != std::string::npos)
      opts.configModel = parseModel(arg.substr(pcreateconfig2.size()));
    else if (arg.rfind(pcreateconfig, 0) != std::string::npos)
      opts.configModel = CmdLineArgs::gpt4;
    else if (arg.rfind(pconfig, 0) != std::string::npos)
      opts.configFileName = arg.substr(pconfig.size());
    else if (arg.rfind(pharness, 0) != std::string::npos)
      opts.harness = arg.substr(pharness.size());
    else if (arg.rfind(pkernel, 0) != std::string::npos)
      opts.kernel = parseSourceRange(arg.substr(pkernel.size()));
    else
      opts.all.push_back(arg);
  }

  operator CmdLineArgs() && { return std::move(opts); }

  CmdLineArgs opts;

  static std::string phelp;
  static std::string phelp2;
  static std::string phelp3;
  static std::string phelpconfig;
  static std::string pcreateconfig;
  static std::string pcreateconfig2;
  static std::string pconfig;
  static std::string pharness;
  static std::string pkernel;
};

std::string CmdLineProc::phelp          = "--help";
std::string CmdLineProc::phelp2         = "-help";
std::string CmdLineProc::phelp3         = "-h";
std::string CmdLineProc::phelpconfig    = "--help-config";
std::string CmdLineProc::pcreateconfig  = "--create-config";
std::string CmdLineProc::pcreateconfig2 = "--create-config=";
std::string CmdLineProc::pconfig        = "--config=";
std::string CmdLineProc::pharness       = "--harness-param=";
std::string CmdLineProc::pkernel        = "--kernel=";

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

struct RevisionPrinter
{
  const Revision& obj;
};

std::ostream& operator<<(std::ostream& os, const RevisionPrinter& el)
{
  constexpr std::size_t filenamelen = 20;
  constexpr std::size_t prefix      = 4;
  constexpr std::size_t suffix      = filenamelen - prefix - 1;

  std::string_view vw = el.obj.fileName();

  if (vw.size() > filenamelen)
    os << vw.substr(0, prefix) << "*" << vw.substr(vw.size() - suffix);
  else
    os << vw << std::setw(filenamelen - vw.size()) << "";

  return os << ": " << TestResultPrinter{ el.obj.result() };
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

  if (std::isnan(prev))
    return "";

  if (std::isnan(curr))
    return "were not assessed";

  if (curr < prev) // consider adding x% to allow for performance variability
    return "improved";

  if (prev < curr) // consider adding x%
    return "got worse";

  return "stayed the same";
}


Revision
initialAssessment(const Settings& settings, const CmdLineArgs& cmdlnargs)
{
  std::string fileName = cmdlnargs.all.back();

  if (settings.inputLang != settings.outputLang)
    return { fileName,
             TestResult{false, nanValue<long double>(), "test harness not run (inputLang != outputLang)"}
           };

  return { fileName, invokeTestScript(settings, fileName, cmdlnargs.harness) };
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


  if (cmdlnargs.configModel != CmdLineArgs::none)
  {
    createDefaultConfig(cmdlnargs);
    return 0;
  }

  Settings          settings   = loadConfig(cmdlnargs.configFileName);

  std::cout << "Settings: ";
  writeSettings(std::cout, settings);
  std::cout << std::endl;

  CompilationResult compres    = compileResult(settings, cmdlnargs);
  int               iterations = settings.iterations;

  std::cerr << "compiled " << compres.success() << std::endl;

  if (!compres.success())
  {
    std::cerr << "Input file does not compile:\n"
              << compres.output()
              << std::endl;

    // do we really need to exit; maybe compiler GPT can fix the output..
    exit(1);
  }

  std::vector<Revision> variants{ initialAssessment(settings, cmdlnargs) };

  // initial prompt
  json::value           query = initialPrompt( settings, cmdlnargs, std::move(compres.output()) );

  try
  {
    while (iterations > 0)
    {
      --iterations;

      const bool promptAI = iterations != 0;

      storeQuery(settings, query);
      invokeAI(settings);

      std::string response = loadAIResponse(settings);
      std::string newFile  = storeGeneratedFile(settings, cmdlnargs, response);

      query   = appendResponse(settings, std::move(query), response);
      compres = compileResult(settings, cmdlnargs, newFile);

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
            std::string prompt = settings.testFailPrompt;

            prompt += variants.back().result().errors();
            prompt += settings.testFailSteps;

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
          prompt += settings.compFailSteps;

          query = appendFailurePrompt(std::move(query), prompt, variants.back().fileName());
        }
      }
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << "ERROR:\n" << ex.what() << "\nterminating"
              << std::endl;
  }
  catch (...)
  {
    std::cerr << "UNKOWN ERROR:" << "\nterminating"
              << std::endl;
  }

  //
  // prepare final report

  // store query and results for review
  storeQuery(settings, query);

  for (const Revision& r : variants)
    std::cout << RevisionPrinter{r} << std::endl;

  // go over results and rank them based on success and results

  return 0;
}


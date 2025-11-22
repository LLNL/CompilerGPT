// CompilerGPT (compgpt)
//   Communicates iteratively with AI models to
//   - improve code bases with an optional optimization report
//   - translate code between languages
//
// Copyright (c) 2025, Lawrence Livermore National Security, LLC.
// All rights reserved.  LLNL-CODE-2001821
//
// License: SPDX BSD 3-Clause "New" or "Revised" License
//          see LICENSE file for details
//
// Authors: pirkelbauer2,liao6 (at) llnl.gov

#include <fstream>
#include <string>
#include <iostream>
#include <numeric>
#include <charconv>
#include <regex>
#include <filesystem>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/version.hpp>

#include "tool_version.hpp"


#if BOOST_VERSION < 108800

#include <boost/process.hpp>

namespace boostprocess = boost::process;

#else
// for boost 1.88 and later, include extra header for backward compatibility

#include <boost/process/v1/args.hpp>
#include <boost/process/v1/system.hpp>
#include <boost/process/v1/io.hpp>

namespace boostprocess = boost::process::v1;

#endif /* BOOST_VERSION */

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

#include "llmtools.hpp"


#include <boost/utility/string_view.hpp>

using StringView = boost::string_view;



namespace json = boost::json;

const std::string CC_MARKER_BEGIN = "```";
const std::string CC_MARKER_LIMIT = "```";

namespace
{

const char* synopsis = "compgpt: driver for code optimizations through LLMs"
                       "\n  description: driver feeds a compiler optimization report of a kernel"
                       "\n               to an LLM and prompts it to optimize the source code."
                       "\n               The rewritten part is extracted from the LLM and"
                       "\n               stored in a file."
                       "\n               The file is compiled and checked for software bugs"
                       "\n               and performance using a user-specified test harness."
                       ;

const char* usage    = "usage: compgpt switches source-file"
                       "\n  switches:"
                       "\n    --version             displays version information and exits."
                       "\n    -h"
                       "\n    -help"
                       "\n    --help                displays this help message and exits."
                       "\n    --help-config         prints config file documentation and exits."
                       "\n    --help-variables      prints information about recognized prompt variables."
                       "\n    --config=jsonfile     config file in json format."
                       "\n                          default: jsonfile=compgpt.json"
                       "\n    --create-config       creates config file and exits."
                       "\n    --create-doc-config   creates config file with documentation fields and exits."
                       "\n    --config:ai=p         creates config file for a specified AI."
                       "\n                          p in {openai,claude,ollama,openrouter,llama-cli}"
                       "\n                          default: p=openai"
                       "\n    --config:model=m      specifies a submodel for AIs (e.g., gpt-4o)."
                       "\n    --config:compiler=p   specifies a path to a compiler."
                       "\n                          CompilerGPT will set ${optreport}, ${compileflags},"
                       "\n                          and ${compilerfamily} if the compiler can be recognized."
                       "\n    --config:from=f       specifies another config file f to initialize the"
                       "\n                          config settings. The AI and compiler settings will"
                       "\n                          be overridden by the corresponding settings (if provided)."
                       "\n    --kernel=range        chooses a specific code segment for optimization."
                       "\n                          range is specified in terms of line numbers."
                       "\n                          The following are examples of valid options:"
                       "\n                            1-10    Lines 1-10 (excluding Line 10)."
                       "\n                            The range can be accessed using ${kernelstart}"
                       "\n                            and ${kernellimit}."
                       //~ "\n                            7:4-10  Lines 7 (starting at column 4) to Line 10."
                       //~ "\n                            7:2-10:8 Lines 7 (starting at column 2) to Line 10"
                       //~ "\n                                     (up to column 8)."
                       "\n                          default: the entire input file"
                       "\n    --var:n=t             Introduces a variable named n and sets it to t."
                       "\n                          The variable can be accessed using ${n}."
                       "\n"
                       "\n  TestScript: success or failure is returned through the exit status"
                       "\n              a numeric quality score is returned on the last non-empty line on stdout."
                       "\n              The lower the quality score the better (e.g., runtime)."
                       "\n              The score is available through variables ${score} and ${scoreint}."
                       ;


// llmtools Settings doc
// \todo move into llmtools
const char* execDoc           = "a string pointing to an executable (script) that calls the external AI (${exec})";
const char* execFlagsDoc      = "arguments to the executable ${execflags}";
const char* historyFileDoc    = "a JSON file storing the conversation history ${historyFile}. invokeai will read the prompt from this file.";
const char* responseFileDoc   = "a file [.txt or .json] where the AI stores the query response";
const char* responseFieldDoc  = "a JSON path in the form of [field ['[' literal ']'] {'.' field ['[' literal ']']} ]"
                                "\n  identifying the response in a JSON output file."
                                "\n  (ignored when responseFile is a text file)";
const char* roleOfAIDoc       = "The name of the AI role in the conversation history. Typically assistant.";
const char* systemTextFileDoc = "If set CompilerGPT writes the system text into the file instead of"
                                "\n  passing it as first message in the conversation history.";
const char* apiKeyNameDoc     = "The name of the API key defined in the environment";
const char* modelNameDoc      = "The name of the model to use";
const char* promptFileDoc     = "The name of the temporary prompt file";

// CompilerGPT Settings doc
const char* compilerDoc       = "a string pointing to a compiler (${compiler})";
const char* compileflagsDoc   = "compile flags passed to compiler (${compileflags})";
const char* compilerfamilyDoc = "a short name for the compiler (${compilerfamily})";
const char* optreportDoc      = "compiler flags that generate the optimization report";
const char* leanOptReportDoc  = "an integer value from 0-2 indicating pruning level of redundant lines in the report."
                                "\n  0 (no pruning);"
                                "\n  1 (remove same messages if adjacent);"
                                "\n  2 (remove same messages).";
const char* testScriptDoc     = "an optional string pointing to an executable that assesses the AI output."
                                "\n  CompilerGpt variables in the string are expanded before the test script"
                                "\n  is invoked."
                                "\n  CompilerGpt variables include: ${compiler}, ${compilerfamily}"
                                "\n  ${compileflags}, ${invokeai}, ${optreport}, ${filename}."
                                "\n  A non-zero exit value indicates that testing faied."
                                "\n  If successful, the last output line should contain a quality score"
                                "\n  (may use floating points). A lower score indicates better results."
                                "\n  If the testScript is not set, it is assumed that the generated"
                                "\n  code passes the regression tests with a quality score of 0.";
const char* testRunsDoc       = "The evaluation harness is run testRuns times and accumulates the score over all runs.";
const char* testOutliersDoc   = "Removes num outliers from each side of the test score range.";
const char* newFileExtDoc     = "a string for the extension of the generated file."
                                "\n  (if not set, the original file extension will be used.)"
                                "\n  This setting is mostly useful for language translation tasks.";
const char* inputLangDoc      = "language delimiter for the input language. Used to delineate ${code} sections.";
const char* outputLangDoc     = "language delimiter for the AI response. (if not set defaults to inputLang).";
const char* systemTextDoc     = "A string setting the context/role in the AI communication.";
const char* firstPromptDoc    = "The initial prompt.";
const char* successPromptDoc  = "Follow up prompt when the previous iteration returned a successful code.";
const char* compFailPromptDoc = "Prompt when the AI generated code does not compile";
const char* testFailPromptDoc = "prompt when the AI generated code produces errors with the test harness";
const char* stopOnSuccessDoc  = "boolean value. if true, the program terminates as soon as testScript reports success";
const char* iterationsDoc     = "Integer number specifying the maximum number of iterations.";

void printVariablesHelp(std::ostream& os)
{
  os << "The following configuration parameters can be used in prompts and config files."
     << "\n"
     << "\n  Commandline arguments:"
     << "\n    kernelstart     first line of the source code as specified by --kernel"
     << "\n                    w/o --kernel kernelstart is: '1'"
     << "\n    kernellimit     one past the last line of the source code as specified by --kernel"
     << "\n                    w/o --kernel kernellimit is: 'end of file'"
     << "\n"
     << "\n  Configuration settings:"
     << "\n    invokeai        a path to an external program controlling the AI interaction"
     << "\n    compiler        a path to a compiler"
     << "\n    compilerfamily  the compiler family (i.e., gcc, clang)."
     << "\n    compileflags    the compile flags"
     << "\n    optreport       the optimization report flags"
     << "\n    historyFile     the file containing the conversation history"
     << "\n"
     << "\n  CompilerGPT generated:"
     << "\n    filename        the latest file (either initial file, or extracted from response)"
     << "\n    code            the code (segment) passed to the AI"
     << "\n    report          the optimization report, log with regression test output or errors/warnings"
     << "\n    score           the score as reported by the evaluation as floating point value"
     << "\n    scoreint        the score as reported by the evaluation as integer value (truncated score)"
     << std::endl;
}

std::string
align(std::string doc, const std::string& spaces)
{
  boost::replace_all(doc, "\n  ", spaces);
  return doc;
}


void printConfigHelp(std::ostream& os)
{
  static std::string indent = "\n" + std::string(17, ' ');

  os << "The following configuration parameters can be set in the config file."
     << "\n"
     << "\nCompiler and optimization report settings:"
     << "\n  compiler       " << align(compilerDoc, indent)
     << "\n  compileflags   " << align(compileflagsDoc, indent)
     << "\n  compilerfamily " << align(compilerfamilyDoc, indent)
     << "\n  optreport      " << align(optreportDoc, indent)
     << "\n  leanOptReport  " << align(leanOptReportDoc, indent)
     << "\n"
     << "\nInteraction with AI"
     << "\n  exec           " << align(execDoc, indent)
     << "\n  historyFile    " << align(historyFileDoc, indent)
     << "\n  responseFile   " << align(responseFileDoc, indent)
     << "\n  responseField  " << align(responseFieldDoc, indent)
     << "\n  inputLang      " << align(inputLangDoc, indent)
     << "\n  outputLang     " << align(outputLangDoc, indent)
     << "\n"
     << "\nCode validation and quality scoring:"
     << "\n  newFileExt     " << align(newFileExtDoc, indent)
     << "\n  testScript     " << align(testScriptDoc, indent)
     << "\n  testRuns       " << align(testRunsDoc, indent)
     << "\n  testOutliers   " << align(testOutliersDoc, indent)
     << "\n"
     << "\nPrompting:"
     << "\n  systemText     " << align(systemTextDoc, indent)
     << "\n  systemTextFile " << align(systemTextFileDoc, indent)
     << "\n  roleOfAI       " << align(roleOfAIDoc, indent)
     << "\n"
     << "\nPrompt text"
     << "\n  firstPrompt    " << align(firstPromptDoc, indent)
     << "\n  successPrompt  " << align(successPromptDoc, indent)
     << "\n  compFailPrompt " << align(compFailPromptDoc, indent)
     << "\n  testFailPrompt " << align(testFailPromptDoc, indent)
     << "\n"
     << "\n  Prompt text can contain variables ${code} [only with firstPrompt] and ${report}."
     << "\n"
     << "\nIteration control:"
     << "\n  iterations     " << align(iterationsDoc, indent)
     << "\n  stopOnSuccess  " << align(stopOnSuccessDoc, indent)
     << "\n"
     << "\nNote:"
     << "\n  A file with default settings can be generated using --create-config and --create-doc-config."
     << "\n  The config file only needs entries when a default setting is overridden."
     << std::endl;
}


/// encapsulates all settings that can be configured through a JSON file.
struct Settings
{
  llmtools::Settings llmSettings;

  std::string  compiler       = "clang";
  std::string  compilerfamily = "clang";
  std::string  compileflags   = "-O3 -march=native -DNDEBUG=1";
  std::string  optreport      = "-Rpass-missed=. -c";
  std::string  testScript     = "";
  std::int64_t testRuns       = 1;
  std::int64_t testOutliers   = 0;
  std::string  newFileExt     = "";
  std::string  inputLang      = "cpp";
  std::string  outputLang     = "cpp";  // same as input language if not specified
  std::string  systemText     = "You are a compiler expert for C++ code optimization. Our goal is to improve the existing code.";
  std::string  firstPrompt    = "Given the following input code in C++:\n${code}\nThe compiler optimization report is as follows:\n${report}\nTask 1: Recognize the coding patterns.\nTask 2: Make pattern specific optimizations to the code. Do not use OpenMP.\nTask 3: Consider the optimization report and prioritize the missed optimizations in terms of expected improvement.\nTask 4: Use the prioritized list to improve the input code further.";
  std::string  successPrompt = "The compiler optimization report for the latest version is as follows:\n${report}\nTask 1: Consider the optimization report and prioritize the missed optimizations in terms of expected improvement.\nTask 2: Use the prioritized list to improve the input code further.";
  std::string  compFailPrompt = "This version did not compile. Here are the error messages:\n${report}\nTry again.";
  std::string  testFailPrompt = "This version failed the regression tests. Here are the error messages:\n${report}\nTry again.";

  bool         stopOnSuccess  = false;
  std::int64_t leanOptReport  = 2;
  std::int64_t iterations     = 1;

  bool languageTranslation() const
  {
    return (  outputLang.size()
           && (inputLang != outputLang)
           );
  }
};


/// collection of variables (i.e., timing related).
struct GlobalVars
{
  std::size_t aiTime      = 0;
  std::size_t compileTime = 0;
  std::size_t evalTime    = 0;
};


/// simple class to help measuring program timing
struct MeasureRuntime
{
    using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

    explicit
    MeasureRuntime(std::size_t& accum)
    : accu(accum), start(std::chrono::high_resolution_clock::now())
    {}

    ~MeasureRuntime()
    {
      const time_point  stop = std::chrono::high_resolution_clock::now();
      const std::size_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();

      accu += elapsed;
    }

  private:
    std::size_t& accu;
    time_point   start;
};

#if 0
/// returns the absolute path for an existing file path \p filename.
std::filesystem::path absolutePath(std::string_view filename)
{
  return std::filesystem::absolute(std::filesystem::path{filename}).remove_filename();
}

#endif

struct MissingCodeError : std::runtime_error
{
  using base = std::runtime_error;
  using base::base;
};

struct MultipleCodeSectionsError : std::runtime_error
{
  using base = std::runtime_error;
  using base::base;
};


void trace(std::ostream& os)
{
  os << std::flush;
}

template <class Arg, class... Rest>
void trace(std::ostream& os, Arg&& arg, Rest&&... rest)
{
  os << std::forward<Arg>(arg);
  trace(os, std::forward<Rest>(rest)...);
}





using PlaceholderMap = llmtools::VariableMap;


/// encapsulates all command line switches and their settings
struct CmdLineArgs
{
  enum CompilerFamily { gcc=0, clang=1, nocomp=2, errcomp=3 };

  bool                          help                 = false;
  bool                          helpConfig           = false;
  bool                          helpVariables        = false;
  bool                          showVersion          = false;
  bool                          configCreate         = false;
  bool                          withDocFields        = false;
  llmtools::LLMProvider         configAI             = llmtools::LLMnone;
  CompilerFamily                configCompilerFamily = nocomp;
  std::string                   configModel          = "";
  std::string                   configFileName       = "compgpt.json";
  std::string                   configFrom           = "";
  std::string                   configCompiler       = "";
  std::filesystem::path         programPath          = "compgpt.bin";
  llmtools::SourceRange         kernel               = llmtools::SourceRange::all();
  std::string                   csvsummary           = "";
  std::vector<std::string>      configFiles          = {};
  std::vector<std::string_view> all;
  PlaceholderMap                vars;
};


/// returns a C string for a boolean. If \p align is set true gets a trailing blank.
/// the returned string must not be freed or overwritten.
const char* as_string(bool v, bool align = false)
{
  if (!v) return "false";
  if (!align) return "true";

  return "true ";
}


using CompilerSetupBase = std::tuple<const char*, const char*>;
struct CompilerSetup : CompilerSetupBase
{
  using base = CompilerSetupBase;
  using base::base;

  const char* reportFlags()    const { return std::get<0>(*this); }
  const char* compilerFamily() const { return std::get<1>(*this); }
};

Settings setupCompiler(Settings settings, const CmdLineArgs& args, CompilerSetup setup)
{
  settings.compiler       = args.configCompiler;
  settings.optreport      = setup.reportFlags();
  settings.compileflags   = "-O3 -march=native -DNDEBUG=1";
  settings.compilerfamily = setup.compilerFamily();

  return settings;
}




/// creates default settings for supported AI models.
Settings
createSettings(const llmtools::Configurations& toolConfigs, Settings settings, const CmdLineArgs& args)
{
  using CompilerFamilySetup = std::unordered_map<CmdLineArgs::CompilerFamily, CompilerSetup>;

  static const CompilerFamilySetup compilerFamilySetup
      = { { CmdLineArgs::gcc,       {"-fopt-info-missed -c", "gcc"} },
          { CmdLineArgs::clang,     {"-Rpass-missed=. -c",   "clang"} }
        };

  if ((args.configAI != llmtools::LLMnone) && (args.configAI != llmtools::LLMerror))
    settings.llmSettings = llmtools::configure(toolConfigs, args.configAI, args.configModel);
  else
    trace(std::cerr, "Not (re)configuring AI. (Unknown or unspecified AI)", "\n");

  if (auto pos = compilerFamilySetup.find(args.configCompilerFamily); pos != compilerFamilySetup.end())
    settings = setupCompiler(std::move(settings), args, pos->second);
  else
    trace(std::cerr, "Not (re)configuring compiler. (Unknown or unspecified compiler)", "\n");

  return settings;
}

}


/// returns C-style command line arguments as std::vector<std::string>
std::vector<std::string>
getCmdlineArgs(char** beg, char** lim)
{
  return std::vector<std::string>(beg, lim);
}


/// an sbtractions to print a sequence of values that are seoarated by a seperator.
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


/// encapsulates a compilation's result
using CompilationResultBase = std::tuple<std::string, bool>;

struct CompilationResult : CompilationResultBase
{
  using base = CompilationResultBase;
  using base::base;

  const std::string& output()  const { return std::get<0>(*this); }
  bool               success() const { return std::get<1>(*this); }
};

/// separates a string \p s at whitespaces and appends them as individual
///   command line arguments to a vector \p args.
void splitArgs(const std::string& s, std::vector<std::string>& args)
{
  std::istringstream all{s};
  std::string        arg;

  while (all >> arg)
    args.emplace_back(std::move(arg));
}

/// separates a string \p input at an configurable whitespace \p ch and returns
///   them as vector<string>.
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



/// Adds extra variables to the map
/// \{
PlaceholderMap
addToMap(PlaceholderMap m, PlaceholderMap extras)
{
  for ( PlaceholderMap::value_type ex : extras )
    m.emplace(ex.first, std::move(ex.second));

  return m;
}

PlaceholderMap
addToMap(PlaceholderMap m, llmtools::SourceRange rng)
{
  std::string one  = "1";
  std::string limit = "end of file";

  if (!rng.entireFile())
  {
    one = std::to_string(rng.beg().line());
    limit = std::to_string(rng.lim().line());
  }

  m.emplace("kernelstart", one);
  m.emplace("kernellimit", limit);
  return m;
}

PlaceholderMap
addToMap(PlaceholderMap m, const Settings& settings)
{
  m.emplace("exec",           settings.llmSettings.exec());
  m.emplace("execflags",      settings.llmSettings.execFlags());
  m.emplace("compiler",       settings.compiler);
  m.emplace("compilerfamily", settings.compilerfamily);
  m.emplace("compileflags",   settings.compileflags);
  m.emplace("optreport",      settings.optreport);
  m.emplace("historyFile",    settings.llmSettings.historyFile());
/*
     << "\n  \"leanOptReport\":"    << settings.leanOptReport << ","
     << "\n  \"responseFile\":\""   << settings.responseFile << "\"" << ","
     << "\n  \"responseField\":\""  << settings.responseField << "\"" << ","
     << "\n  \"testScript\":\""     << settings.testScript << "\"" << ","
     << "\n  \"newFileExt\":\""     << settings.newFileExt << "\"" << ","
     << "\n  \"inputLang\":\""      << settings.inputLang << "\"" << ","
     << "\n  \"outputLang\":\""     << settings.outputLang << "\"" << ","
     << "\n  \"systemText\":\""     << replace_nl(settings.systemText) << "\"" << ","
     << "\n  \"roleOfAI\":\""       << settings.llmSettings.roleOfAI << "\"" << ","
     << "\n  \"systemTextFile\":\"" << settings.llmSettings.systemTextFile << "\"" << ","
     << "\n  \"firstPrompt\":\""    << replace_nl(settings.firstPrompt) << "\","
     << "\n  \"successPrompt\":\""  << replace_nl(settings.successPrompt) << "\","
     << "\n  \"compFailPrompt\":\"" << replace_nl(settings.compFailPrompt) << "\","
     << "\n  \"testFailPrompt\":\"" << replace_nl(settings.testFailPrompt) << "\","
     << "\n  \"stopOnSuccess\":"    << as_string(settings.stopOnSuccess) << ","
     << "\n  \"iterations\":"       << settings.iterations
*/

  return m;
}
/// \}

PlaceholderMap makeVariables(PlaceholderMap m)
{
  return m;
}

template <class Additions, class... MoreAdditions>
PlaceholderMap makeVariables(PlaceholderMap m, Additions&& args, MoreAdditions&&... more)
{
  return makeVariables( addToMap(std::move(m), std::forward<Additions>(args)),
                        std::forward<MoreAdditions>(more)...
                      );
}

template <class... Additions>
std::string
expandText(const std::string& prompt, PlaceholderMap m, Additions... extras)
{
  return llmtools::expandText(prompt, makeVariables(std::move(m), std::forward<Additions>(extras)...));
}


using DiagnosticBase = std::tuple<std::string, llmtools::SourcePoint, std::vector<std::string> >;

/// encapsulates diagnostic output (warnings, errors, optimization diagnostics).
struct Diagnostic : DiagnosticBase
{
  using base = DiagnosticBase;
  using base::base;

  const std::string&              file()     const { return std::get<0>(*this); }
  llmtools::SourcePoint           location() const { return std::get<1>(*this); }
  const std::vector<std::string>& message()  const { return std::get<2>(*this); }

  bool empty() const { return message().empty(); }

  std::string&              file()     { return std::get<0>(*this); }
  llmtools::SourcePoint&    location() { return std::get<1>(*this); }
  std::vector<std::string>& message()  { return std::get<2>(*this); }
};

struct EqualDiagnostic
{
  bool operator()(const Diagnostic& lhs, const Diagnostic& rhs) const
  {
    if (  lhs.file() != rhs.file()
       || lhs.location() != rhs.location()
       )
      return false;

    const std::vector<std::string>& lhsmsg = lhs.message();
    const std::vector<std::string>& rhsmsg = rhs.message();

    return std::equal( lhsmsg.begin(), lhsmsg.end(),
                       rhsmsg.begin(), rhsmsg.end()
                     );
  }
};

struct LessThanDiagnostic
{
  bool operator()(const Diagnostic& lhs, const Diagnostic& rhs) const
  {
    if (lhs.file() < rhs.file())
      return true;

    if (lhs.file() > rhs.file())
      return false;

    if (lhs.location() < rhs.location())
      return true;

    if (lhs.location() > rhs.location())
      return false;

    const std::vector<std::string>& lhsmsg = lhs.message();
    const std::vector<std::string>& rhsmsg = rhs.message();

    auto res = std::mismatch( lhsmsg.begin(), lhsmsg.end(),
                              rhsmsg.begin(), rhsmsg.end()
                            );

    if ((res.first == lhsmsg.end()) && (res.second != rhsmsg.end()))
      return true;

    if ((res.first != lhsmsg.end()) && (res.second == rhsmsg.end()))
      return false;

    if ((res.first == lhsmsg.end()) && (res.second == rhsmsg.end()))
      return false;

    return *res.first < *res.second;
  }
};


/// uses a regex to find a file location within a string \p s.
std::tuple<std::string, llmtools::SourcePoint>
fileLocation(const std::string& s)
{
  const std::regex patLocation{"(.*):([0-9]*):([0-9]*):"};
  std::smatch      matches;

  if (std::regex_search(s, matches, patLocation))
  {
    assert(matches.size() == 4);
    return { matches[1], { std::stoi(matches[2]), std::stoi(matches[3]) } };
  }

  return {};
}

/// processes a log to capture diagnostic output together with its location information.
/// filters out any include header trace.
struct DiagnosticFilter
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

    const auto [file, loc]       = fileLocation(s);
    const bool containsSourceLoc = !file.empty();

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

void
trimOptReport(Diagnostic& diag)
{
  auto lim = diag.message().end();
  auto pos = std::unique(diag.message().begin(), lim);

  diag.message().erase(pos, lim);
}

/// filters diagnostic output for source location
CompilationResult
filterMessageOutput( const Settings& settings,
                     const std::string& out,
                     std::string_view filename,
                     llmtools::SourceRange rng,
                     bool success
                   )
{
  if (!success) return { out, success };

  std::vector<std::string> lines = splitString(out);
  std::vector<Diagnostic>  diagnosed = std::for_each( lines.begin(), lines.end(),
                                                      DiagnosticFilter{}
                                                    );

  auto outsideSourceRange =
           [rng, filename](const Diagnostic& el) -> bool
           {
             if (el.file().find(filename) == std::string::npos)
               return true;

             // test whether el is OUTSIDE source range
             return (  (el.location() <  rng.beg())
                    || (el.location() >= rng.lim())
                    );
           };

  auto beg = diagnosed.begin();
  auto pos = std::remove_if( beg, diagnosed.end(), outsideSourceRange );

  if (settings.leanOptReport > 0)
  {
    if (settings.leanOptReport > 1)
      std::sort(beg, pos, LessThanDiagnostic{});

    pos = std::unique(beg, pos, EqualDiagnostic{});
    std::for_each(beg, pos, trimOptReport);
  }

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

/// calls the compiler component
CompilationResult
invokeCompiler(const Settings& settings, GlobalVars& globals, llmtools::SourceRange kernelrng, std::vector<std::string> args)
{
  MeasureRuntime timer(globals.compileTime);

  if (settings.compiler.size() == 0)
  {
    std::cerr << "No compiler configured. Skipping compile test."
              << std::endl;

    return { "", true };
  }

  std::string src = std::move(args.back());
  args.pop_back();

  splitArgs(settings.optreport, args);
  splitArgs(settings.compileflags, args);
  args.push_back(src);

  trace(std::cerr, "compile: ", settings.compiler, range(args), '\n');

  boost::asio::io_context  ios;
  std::future<std::string> outstr;
  std::future<std::string> errstr;
  std::future<int>         exitCode;
  boostprocess::child      compilation( settings.compiler,
                                        boostprocess::args(args),
                                        boostprocess::std_in.close(),
                                        boostprocess::std_out > outstr,
                                        boostprocess::std_err > errstr,
                                        boostprocess::on_exit = exitCode,
                                        ios
                                      );

  ios.run();

  const bool success = exitCode.get() == 0;
  trace(std::cerr, "success(compile): ", success, '\n');

  return filterMessageOutput(settings, errstr.get(), args.back(), kernelrng, success);
}

/// calls the compiler and returns the compilation result.
CompilationResult
compileResult( const Settings& settings,
               const CmdLineArgs& cmdline,
               GlobalVars& globals,
               std::string_view newFile,
               llmtools::SourceRange kernelrng
             )
{
  trace(std::cerr, "compile: ", newFile, "@", kernelrng, '\n');

  std::vector<std::string> args;

  std::transform( cmdline.all.begin(), std::prev(cmdline.all.end()),
                  std::back_inserter(args),
                  [](std::string_view vw) -> std::string
                  {
                    return std::string(vw);
                  }
                );

  args.emplace_back(std::string(newFile));

  CompilationResult res = invokeCompiler(settings, globals, kernelrng, args);

  trace(std::cerr, res.output(), '\n');
  return res;
}

/// convenience function for first iteration
CompilationResult
compileResult(const Settings& settings, const CmdLineArgs& cmdline, GlobalVars& globals)
{
  return compileResult(settings, cmdline, globals, cmdline.all.back(), cmdline.kernel);
}

/// returns nan for a given floating point type \p F.
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

/// encapsulates test results
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
  const char*       sep = "  score: ";
};

std::ostream& operator<<(std::ostream& os, const TestResultPrinter& el)
{
  return os << as_string(el.obj.success(), true) << el.sep << el.obj.score();
}

using RevisionBase = std::tuple<std::string, TestResult>;

struct Revision : RevisionBase
{
  using base = RevisionBase;
  using base::base;

  const std::string& fileName() const { return std::get<0>(*this); }
  const TestResult&  result()   const { return std::get<1>(*this); }
};

/// prints Revision objects in "result" format
struct ResultPrinter
{
  const Revision& obj;
};

std::ostream& operator<<(std::ostream& os, const ResultPrinter& el)
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

/// prints Revision objects in CSV format
struct CsvResultPrinter
{
  const Revision& obj;
};

std::ostream& operator<<(std::ostream& os, const CsvResultPrinter& el)
{
  return os << el.obj.fileName() << "," << TestResultPrinter{ el.obj.result(), "," };
}


/// invokes the test script
TestResult
invokeTestScript(const Settings& settings, GlobalVars& globals, const PlaceholderMap& vars, const std::string& filename)
{
  static constexpr long double zeroScore = 0.0;
  static const     std::string prmFileName = "filename";

  MeasureRuntime timer(globals.evalTime);

  if (settings.testScript.empty())
  {
    std::cerr << "Settings.testScript not configured. Not running tests."
              << std::endl;

    return { true, 0.0, "" };
  }

  if (settings.testRuns == 0)
  {
    std::cerr << "Settings.testRuns is 0. Not running tests."
              << std::endl;

    return { true, 0.0, "" };
  }

  std::string testCall = expandText( settings.testScript,
                                     vars,
                                     std::ref(settings),
                                     PlaceholderMap{ {prmFileName, filename} }
                                   );

  std::vector<long double> scores;
  std::vector<std::string> args;

  splitArgs(testCall, args);

  std::string              testHarness = args.front();

  args.erase(args.begin());

  for (std::int64_t i = 0; i < settings.testRuns; ++i)
  {
    trace(std::cerr, "test: ", testHarness, " ", range(args), '\n');

    boost::asio::io_context  ios;
    std::future<std::string> outstr;
    std::future<std::string> errstr;
    std::future<int>         exitCode;
    boostprocess::child      tst( testHarness,
                                  boostprocess::args(args),
                                  boostprocess::std_in.close(),
                                  boostprocess::std_out > outstr,
                                  boostprocess::std_err > errstr,
                                  boostprocess::on_exit = exitCode,
                                  ios
                                );

    ios.run();

    const bool success = exitCode.get() == 0;

    std::string outs = outstr.get();
    std::string errs = errstr.get();

    std::cout << outs << std::endl;
    std::cerr << errs << std::endl;

    trace(std::cerr, "success(test): ", success, '\n');

    if (!success)
      return { false, nanValue<long double>(), std::move(errs) };

    scores.push_back(testScore(success, outs));
  }

  std::sort(scores.begin(), scores.end());

  const auto  scoreBeg   = std::next(scores.begin(), settings.testOutliers);
  const auto  scoreLim   = std::prev(scores.end(),   settings.testOutliers);
  long double totalScore = std::accumulate(scoreBeg, scoreLim, zeroScore, std::plus<long double>{});

  return { true, totalScore, "" };
}


/// generates a conversation history containing the first prompt.
llmtools::ConversationHistory
initialPrompt(const Settings& settings, const CmdLineArgs& args, std::string output, const Revision& rev)
{
  llmtools::ConversationHistory res{settings.llmSettings, settings.systemText};

  if (settings.iterations == 0)
    return res;

  long double    score = rev.result().score();
  std::int64_t   iscore = score;
  PlaceholderMap extras{ {"code",     fileToMarkdown(settings.inputLang, std::string(args.all.back()), args.kernel)},
                         {"report",   output},
                         {"score",    std::to_string(score)},
                         {"scoreint", std::to_string(iscore)}
                       };

  res.appendPrompt( expandText( settings.firstPrompt,
                                args.vars,
                                args.kernel,
                                std::ref(settings),
                                std::move(extras)
                              )
                  );

  return res;
}


/// parses JSON input from a line.
json::value
parseJsonLine(std::string line)
{
  json::stream_parser       p;
  boost::system::error_code ec;

  p.write(line.c_str(), line.size(), ec);

  if (ec) return nullptr;

  p.finish(ec);
  if (ec) return nullptr;

  return p.release();
}



/// generates a unique new filename by appending a number at the end.
/// \param  fileName   original file name
/// \param  newFileExt new file extension (for language translation)
/// \param  iteration  the number to append.
/// \return the name of a non existing file
/// \details
///   if (fileName-ext)iteration.newFileExt does not exist return the name,
///   otherwise try with iteration+1.
std::string
generateNewFileName(std::string_view fileName, std::string_view newFileExt, int iteration)
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

/// extracts the file content from the AI \p response and use it to generate
///   a new input file. The new file name is returned.
std::tuple<std::string, llmtools::SourceRange>
storeGeneratedFile( const Settings& settings,
                    const CmdLineArgs& cmdline,
                    std::string_view response
                  )
{
  using CodeSections = std::vector<llmtools::CodeSection>;

  std::string_view  fileName  = cmdline.all.back();
  const int         iteration = 1;
  const std::string newFile   = generateNewFileName(fileName, settings.newFileExt, iteration);
  CodeSections      codeSections = llmtools::extractCodeSections(std::string(response));

  if (codeSections.empty())
  {
    trace(std::cerr, response, "\n  missing markdown code block ", settings.outputLang, '\n');
    throw MissingCodeError{"Cannot find markdown code block in AI output."};
  }

  if (codeSections.size() > 1)
  {
    trace(std::cerr, response, "\n  found multiple code sections\n");
    throw MultipleCodeSectionsError{"Found multiple code sections."};
  }

  std::ofstream     outf{newFile};
  std::ifstream     srcf{std::string(fileName)};
  llmtools::SourceRange newRange = replaceSourceSection(outf, srcf, cmdline.kernel, codeSections.back());

  return { newFile, newRange };
}


/// replaces new line characters in a string with escaped newline
std::string replace_nl(std::string s)
{
  boost::replace_all(s, "\n", "\\n");

  return s;
}

std::string fieldDoc(bool gen, const char* field, const char* docString)
{
  if (!gen) return {};

  std::string res;
  std::string doc = align(docString, " ");

  res += "\n  \"";
  res += field;
  res += "\":";
  res += boost::json::string(doc);
  res += ",";

  return res;
}

/// pretty prints settings to JSON format
void writeSettings(std::ostream& os, const CmdLineArgs& args, const Settings& settings)
{
  const bool        genDoc = args.withDocFields;
  const std::string nostring;

  // print pretty json by hand, as boost does not pretty print by default.
  // \todo consider using https://www.boost.org/doc/libs/1_80_0/libs/json/doc/html/json/examples.html
     //~ << "\n  \"optcompiler\":\""    << settings.optcompiler << "\","
     //~ << "\n  \"optcompile\":\""     << settings.optcompile << "\","

  os << "{"
     // write out fields as defined by llmtools
     // \todo consider using llmtools::SettingsJsonFieldWriter
     << fieldDoc(genDoc, "exec-doc", execDoc)
     << "\n  \"exec\":"           << boost::json::string(settings.llmSettings.exec()) << ","
     << fieldDoc(genDoc, "execFlags-doc", execFlagsDoc)
     << "\n  \"execFlags\":"      << boost::json::string(settings.llmSettings.execFlags()) << ","
     << fieldDoc(genDoc, "historyFile-doc", historyFileDoc)
     << "\n  \"historyFile\":"    << boost::json::string(settings.llmSettings.historyFile()) << ","
     << fieldDoc(genDoc, "responseFile-doc", responseFileDoc)
     << "\n  \"responseFile\":"   << boost::json::string(settings.llmSettings.responseFile()) << ","
     << fieldDoc(genDoc, "responseField-doc", responseFieldDoc)
     << "\n  \"responseField\":"  << boost::json::string(settings.llmSettings.responseField()) << ","
     << fieldDoc(genDoc, "systemTextFile-doc", systemTextFileDoc)
     << "\n  \"systemTextFile\":" << boost::json::string(settings.llmSettings.systemTextFile()) << ","
     << fieldDoc(genDoc, "roleOfAI-doc", roleOfAIDoc)
     << "\n  \"roleOfAI\":"       << boost::json::string(settings.llmSettings.roleOfAI()) << ","
     << fieldDoc(genDoc, "apiKeyName-doc", apiKeyNameDoc)
     << "\n  \"apiKeyName\":"     << boost::json::string(settings.llmSettings.apiKeyName()) << ","
     << fieldDoc(genDoc, "modelName-doc", modelNameDoc)
     << "\n  \"modelName\":"      << boost::json::string(settings.llmSettings.modelName()) << ","
     << fieldDoc(genDoc, "promptFile-doc", promptFileDoc)
     << "\n  \"promptFile\":"     << settings.llmSettings.promptFile() << ","

     // CompilerGPT settings
     << fieldDoc(genDoc, "compiler-doc", compilerDoc)
     << "\n  \"compiler\":\""       << settings.compiler << "\","
     << fieldDoc(genDoc, "compileflags-doc", compileflagsDoc)
     << "\n  \"compileflags\":\""   << settings.compileflags << "\","
     << fieldDoc(genDoc, "compilerfamily-doc", compileflagsDoc)
     << "\n  \"compilerfamily\":\"" << settings.compilerfamily << "\","
     << fieldDoc(genDoc, "optreport-doc", optreportDoc)
     << "\n  \"optreport\":\""      << settings.optreport << "\","
     << fieldDoc(genDoc, "leanOptReport-doc", leanOptReportDoc)
     << "\n  \"leanOptReport\":"    << settings.leanOptReport << ","
     << fieldDoc(genDoc, "testScript-doc", testScriptDoc)
     << "\n  \"testScript\":\""     << settings.testScript << "\"" << ","
     << fieldDoc(genDoc, "testRuns-doc", testRunsDoc)
     << "\n  \"testRuns\":"         << settings.testRuns << ","
     << fieldDoc(genDoc, "testOutliers-doc", testOutliersDoc)
     << "\n  \"testOutliers\":"     << settings.testOutliers << ","
     << fieldDoc(genDoc, "newFileExt-doc", newFileExtDoc)
     << "\n  \"newFileExt\":\""     << settings.newFileExt << "\"" << ","
     << fieldDoc(genDoc, "inputLang-doc", inputLangDoc)
     << "\n  \"inputLang\":\""      << settings.inputLang << "\"" << ","
     << fieldDoc(genDoc, "outputLang-doc", outputLangDoc)
     << "\n  \"outputLang\":\""     << settings.outputLang << "\"" << ","
     << fieldDoc(genDoc, "systemText-doc", systemTextDoc)
     << "\n  \"systemText\":\""     << replace_nl(settings.systemText) << "\"" << ","
     << fieldDoc(genDoc, "firstPrompt-doc", firstPromptDoc)
     << "\n  \"firstPrompt\":\""    << replace_nl(settings.firstPrompt) << "\","
     << fieldDoc(genDoc, "successPrompt-doc", successPromptDoc)
     << "\n  \"successPrompt\":\""  << replace_nl(settings.successPrompt) << "\","
     << fieldDoc(genDoc, "compFailPrompt-doc", compFailPromptDoc)
     << "\n  \"compFailPrompt\":\"" << replace_nl(settings.compFailPrompt) << "\","
     << fieldDoc(genDoc, "testFailPrompt-doc", testFailPromptDoc)
     << "\n  \"testFailPrompt\":\"" << replace_nl(settings.testFailPrompt) << "\","
     << fieldDoc(genDoc, "stopOnSuccess-doc", stopOnSuccessDoc)
     << "\n  \"stopOnSuccess\":"    << as_string(settings.stopOnSuccess) << ","
     << fieldDoc(genDoc, "iterations-doc", iterationsDoc)
     << "\n  \"iterations\":"       << settings.iterations
     << "\n}" << std::endl;
}

void configVersionCheck(const json::value& cnf)
{
  const std::string X = "x@123";
  const bool oldConfigFile = (  (llmtools::loadField(cnf, "invokeai", X) != X)
                             || (llmtools::loadField(cnf, "optcompiler", X) != X)
                             || (llmtools::loadField(cnf, "optcompile", X) != X)
                             || (llmtools::loadField(cnf, "queryFile", X) != X)
                             );

  if (oldConfigFile)
  {
    std::cerr << "The config file was created for a previous CompilerGPT version."
              << "\n  Please rename the following fields"
              << "\n    invokeai    => exec"
              << "\n       [also add execFlags]"
              << "\n    optcompiler => compiler"
              << "\n    optcompile  => compileflags"
              << "\n    queryFile   => historyFile"
              << std::endl;

    exit(1);
  }
}

/// loads settings from a JSON file \p configFileName
Settings readSettings(const std::string& configFileName)
{
  Settings settings;

  try
  {
    json::value cnf = llmtools::readJsonFile(configFileName);
    Settings    config;

    configVersionCheck(cnf);

    config.llmSettings = llmtools::settings(cnf, config.llmSettings);

    config.compiler       = llmtools::loadField(cnf, "compiler",        config.compiler);
    config.compileflags   = llmtools::loadField(cnf, "compileflags",    config.compileflags);
    config.compilerfamily = llmtools::loadField(cnf, "compilerfamily",  config.compilerfamily);
    config.optreport      = llmtools::loadField(cnf, "optreport",       config.optreport);
    config.testScript     = llmtools::loadField(cnf, "testScript",      config.testScript);
    config.testRuns       = llmtools::loadField(cnf, "testRuns",        config.testRuns);
    config.testOutliers   = llmtools::loadField(cnf, "testOutliers",    config.testOutliers);
    config.newFileExt     = llmtools::loadField(cnf, "newFileExt",      config.newFileExt);
    config.inputLang      = llmtools::loadField(cnf, "inputLang",       config.inputLang);
    config.outputLang     = llmtools::loadField(cnf, "outputLang",      config.inputLang); // out is in if not set
    config.systemText     = llmtools::loadField(cnf, "systemText",      config.systemText);
    config.stopOnSuccess  = llmtools::loadField(cnf, "stopOnSuccess",   config.stopOnSuccess);
    config.leanOptReport  = llmtools::loadField(cnf, "leanOptReport",   config.leanOptReport);
    config.iterations     = llmtools::loadField(cnf, "iterations",      config.iterations);

    config.firstPrompt    = llmtools::loadField(cnf, "firstPrompt",     config.firstPrompt);
    config.successPrompt  = llmtools::loadField(cnf, "successPrompt",   config.successPrompt);
    config.compFailPrompt = llmtools::loadField(cnf, "compFailPrompt",  config.compFailPrompt);
    config.testFailPrompt = llmtools::loadField(cnf, "testFailPrompt",  config.testFailPrompt);

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

  if (settings.testRuns <= 2*settings.testOutliers)
  {
    trace(std::cerr, settings.testRuns, "(testRuns) <= ", 2*settings.testOutliers, "(2*testOutliers)\n");
    throw std::runtime_error{"settings.testRuns <= 2*settings.testOutliers"};
  }

  return settings;
}

/// creates JSON file with default values
void createConfigFile(const llmtools::Configurations& toolConfigs, CmdLineArgs args)
{
  if (std::filesystem::exists(args.configFileName))
  {
    std::cerr << "config file " << args.configFileName << " exists."
              << "\n  not creating a new file! (delete file or choose different file name)"
              << std::endl;
    return;
  }

  if (args.configAI == llmtools::LLMnone)
  {
    std::cerr << "Unspecified AI component."
              << std::endl;

    if (args.configFrom.empty())
    {
      std::cerr << "** Using default model: OpenAI/gpt-4o **"
                <<  std::endl;

      args.configAI = "gpt4o";
    }
  }

  Settings      settings = readSettings(args.configFrom);
  std::ofstream ofs(args.configFileName);

  writeSettings(ofs, args, createSettings(toolConfigs, settings, args));
}

/// Functor processing command line arguments
struct CmdLineProc
{
  std::tuple<std::string, CmdLineArgs::CompilerFamily>
  determineCompilerFamily(std::string_view comparg)
  {
    std::string comp = std::string(comparg);

    if (comp.empty())
    {
      std::cerr << "No compiler specified. Use as in: --config:compiler=/PATH/TO/COMPILER"
                << std::endl;
      return { comp, CmdLineArgs::nocomp };
    }

    if (comp.rfind("/", 0) != 0)
      comp = boostprocess::search_path(comp).string();

    std::vector<std::string> args{"--version"};
    boost::asio::io_context  ios;
    std::future<std::string> outstr;
    std::future<std::string> errstr;
    std::future<int>         exitCode;
    boostprocess::child      compilerCheck( comp,
                                            boostprocess::args(args),
                                            boostprocess::std_in.close(),
                                            boostprocess::std_out > outstr,
                                            boostprocess::std_err > errstr,
                                            boostprocess::on_exit = exitCode,
                                            ios
                                          );

    ios.run();

    const bool success = exitCode.get() == 0;

    if (!success)
    {
      std::cerr << "Automatic compiler check failed: " << comp << " --version"
                << std::endl;
      return { comp, CmdLineArgs::nocomp };
    }

    const std::string outtxt = outstr.get();
    const std::string outcap = boost::to_upper_copy(outtxt);

    if (outcap.find("GCC") != std::string::npos)
      return { comp, CmdLineArgs::gcc };

    if (outcap.find("CLANG") != std::string::npos)
      return { comp, CmdLineArgs::clang };

    std::cerr << "Unable to configure compiler: " << comparg << "[" << comp << "]\n"
              << outtxt
              << std::endl;

    return { comp, CmdLineArgs::nocomp };
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

  std::tuple<llmtools::SourcePoint, std::string_view>
  parseSourcePoint(std::string_view s0, std::function<std::string_view(std::string_view)> follow)
  {
    auto [ln, s1] = parseNum(s0, parseOptionalChar(':'));
    auto [cl, s2] = parseNum(s1, follow);

    return { {ln,cl}, s2 };
  }

  llmtools::SourceRange
  parseSourceRange(std::string_view s0)
  {
    auto [beg, s1] = parseSourcePoint(s0, parseChar('-'));
    auto [lim, s2] = parseSourcePoint(s1, parseOptionalChar(';'));

    if (!s2.empty())
    {
      std::cerr << "source range has trailing characters; source range ignored."
                << std::endl;
      return {llmtools::SourcePoint{},llmtools::SourcePoint{}};
    }

    return {beg,lim};
  }

  void
  parseUserVar(PlaceholderMap& vars, std::string_view s0)
  {
    std::cerr << "parsing ]" << s0 << std::endl;

    auto pos = s0.find('=');

    if (pos == std::string_view::npos)
    {
      std::cerr << "invalid variable declaration: " << s0 << " <ignored>"
                << std::endl;
      return;
    }

    std::cerr << "add ]" << s0.substr(0, pos) << " " << s0.substr(pos+1)
              << std::endl;
    vars[std::string(s0.substr(0, pos))] = s0.substr(pos+1);
  }

  void operator()(std::string_view arg)
  {
    if (arg.rfind(phelpconfig, 0) == 0)
      opts.helpConfig = true;
    else if (arg.rfind(phelpvariables, 0) == 0)
      opts.helpVariables = true;
    else if (arg.rfind(pversion, 0) == 0)
      opts.showVersion = true;
    else if (arg.rfind(phelp, 0) == 0)
      opts.help = true;
    else if (arg.rfind(phelp2, 0) == 0)
      opts.help = true;
    else if (arg.rfind(phelp3, 0) == 0)
      opts.help = true;
    else if (arg.rfind(pconfigmodel, 0) == 0)
      opts.configModel = arg.substr(pconfigmodel.size());
    else if (arg.rfind(pconfigai, 0) == 0)
      opts.configAI = arg.substr(pconfigai.size());
    else if (arg.rfind(pconfigcreate, 0) == 0)
      opts.configCreate = true;
    else if (arg.rfind(pdocconfigcreate, 0) == 0)
      opts.configCreate = opts.withDocFields = true;
    else if (arg.rfind(pconfigfrom, 0) == 0)
      opts.configFrom = arg.substr(pconfigfrom.size());
    else if (arg.rfind(pconfigcompiler, 0) == 0)
      std::tie(opts.configCompiler, opts.configCompilerFamily)
          = determineCompilerFamily(arg.substr(pconfigcompiler.size()));
    else if (arg.rfind(pconfig, 0) == 0)
      opts.configFileName = arg.substr(pconfig.size());
    else if (arg.rfind(pkernel, 0) == 0)
      opts.kernel = parseSourceRange(arg.substr(pkernel.size()));
    else if (arg.rfind(pvar, 0) == 0)
      parseUserVar(opts.vars, arg.substr(pvar.size()));
    else if (arg.rfind(pcsvsummary, 0) == 0)
      opts.csvsummary = arg.substr(pcsvsummary.size());
    else if (arg.rfind(pmodelConfig, 0) == 0)
      opts.configFiles.emplace_back(arg.substr(pmodelConfig.size()));
    else
      opts.all.push_back(arg);
  }

  operator CmdLineArgs() && { return std::move(opts); }

  CmdLineArgs opts;

  static const std::string pversion;
  static const std::string phelp;
  static const std::string phelp2;
  static const std::string phelp3;
  static const std::string phelpconfig;
  static const std::string phelpvariables;
  static const std::string pconfigcreate;
  static const std::string pdocconfigcreate;
  static const std::string pconfigai;
  static const std::string pconfigmodel;
  static const std::string pconfigfrom;
  static const std::string pconfigcompiler;
  static const std::string pconfig;
  static const std::string pvar;
  static const std::string pkernel;
  static const std::string pcsvsummary;
  static const std::string pmodelConfig;
};

const std::string CmdLineProc::pversion         = "--version";
const std::string CmdLineProc::phelp            = "--help";
const std::string CmdLineProc::phelp2           = "-help";
const std::string CmdLineProc::phelp3           = "-h";
const std::string CmdLineProc::phelpconfig      = "--help-config";
const std::string CmdLineProc::phelpvariables   = "--help-variables";
const std::string CmdLineProc::pconfigcreate    = "--create-config";
const std::string CmdLineProc::pdocconfigcreate = "--create-doc-config";
const std::string CmdLineProc::pconfigai        = "--config:ai=";
const std::string CmdLineProc::pconfigmodel     = "--config:model=";
const std::string CmdLineProc::pconfigfrom      = "--config:from=";
const std::string CmdLineProc::pconfigcompiler  = "--config:compiler=";
const std::string CmdLineProc::pconfig          = "--config=";
const std::string CmdLineProc::pvar             = "--var:";
const std::string CmdLineProc::pkernel          = "--kernel=";
const std::string CmdLineProc::pcsvsummary      = "--csvsummary=";
const std::string CmdLineProc::pmodelConfig     = "--model-file=";

CmdLineArgs parseArguments(const std::vector<std::string>& args)
{
  return std::for_each( std::next(args.begin()), args.end(),
                        CmdLineProc{}
                      );
}


/// generates a quality description
/// (not used for prompting currently)
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


/// generates an assessment for the initial code
Revision
initialAssessment(const Settings& settings, const CmdLineArgs& cmdlnargs, GlobalVars& globals)
{
  static constexpr const char* harrnessNotRun = "test harness not run on inital code (language translation)";

  std::string fileName(cmdlnargs.all.back());

  if (settings.languageTranslation())
    return { fileName, TestResult{false, nanValue<long double>(), harrnessNotRun } };

  return { fileName, invokeTestScript(settings, globals, cmdlnargs.vars, fileName) };
}

/// prints revision information with specified printer
template <class Printer>
void reportResults(std::ostream& os, const std::vector<Revision>& variants)
{
  for (const Revision& r : variants)
    os << Printer{r} << std::endl;
}


std::tuple<llmtools::ConversationHistory, std::vector<Revision>, bool>
promptResponseEval( const CmdLineArgs& cmdlnargs,
                    const Settings& settings,
                    GlobalVars& globals,
                    llmtools::ConversationHistory conversation,
                    std::vector<Revision> variants,
                    bool lastIteration
                  )
{
  {
    MeasureRuntime aiTime{globals.aiTime};

    conversation = llmtools::queryResponse(settings.llmSettings, std::move(conversation));
  }

  try
  {
    const auto [newFile, kernelrange] = storeGeneratedFile(settings, cmdlnargs, conversation.lastEntry());
    CompilationResult compres         = compileResult(settings, cmdlnargs, globals, newFile, kernelrange);

    if (compres.success())
    {
      variants.emplace_back( newFile,
                             invokeTestScript(settings, globals, cmdlnargs.vars, newFile)
                           );

      if (variants.back().result().success())
      {
        trace(std::cerr, "Compiled and tested, results ", qualityText(variants), ".\n");

        if (settings.stopOnSuccess)
          return { std::move(conversation), std::move(variants), true };

        if (!lastIteration)
        {
          long double    score = variants.back().result().score();
          std::int64_t   iscore = score;
          PlaceholderMap extras{ {"report",   compres.output()},
                                 {"score",    std::to_string(score)},
                                 {"scoreint", std::to_string(iscore)}
                               };
          std::string    prompt = expandText( settings.successPrompt,
                                              cmdlnargs.vars,
                                              kernelrange,
                                              std::ref(settings),
                                              std::move(extras)
                                            );

          // \todo add quality assessment to prompt
          conversation.appendPrompt(prompt);
        }
      }
      else
      {
        trace(std::cerr, "Code compiled but test failed...\n");

        if (!lastIteration)
        {
          PlaceholderMap extras{ {"report",   variants.back().result().errors()},
                                 {"score",    "NaN"},
                                 {"scoreint", "regression tests failed"}
                               };
          std::string    prompt = expandText( settings.testFailPrompt,
                                              cmdlnargs.vars,
                                              kernelrange,
                                              std::ref(settings),
                                              std::move(extras)
                                            );

          conversation.appendPrompt(prompt);
        }
      }
    }
    else
    {
      // ask to correct the code
      trace(std::cerr, "Compilation failed...\n");

      variants.emplace_back( newFile, TestResult{false, nanValue<long double>(), compres.output()} );

      if (!lastIteration)
      {
        PlaceholderMap extras{ {"report",   compres.output()},
                               {"score",    "NaN"},
                               {"scoreint", "compilation failed"}
                             };
        std::string prompt = expandText( settings.compFailPrompt,
                                         cmdlnargs.vars,
                                         kernelrange,
                                         std::ref(settings),
                                         std::move(extras)
                                       );

        conversation.appendPrompt(prompt);
      }
    }
  }
  catch (const MissingCodeError&)
  {
    std::string prompt = "Unable to find the markdown code block. Respond by putting the optimized code in a markdown code block.";

    conversation.appendPrompt(prompt);
    variants.emplace_back("--no-code--", TestResult{false, nanValue<long double>(), "<no code marker>"});
  }
  catch (const MultipleCodeSectionsError&)
  {
    std::string prompt = "There were multiple code sections in the response. Return the optimized code within a single markdown code block.";

    conversation.appendPrompt(prompt);
    variants.emplace_back("|codes-section|>1", TestResult{false, nanValue<long double>(), "<too may code segments>"});
  }

  return { std::move(conversation), std::move(variants), false };
}

/// core loop managing AI/tool interactions
/// \return the conversation and the code revisions
std::tuple<llmtools::ConversationHistory, std::vector<Revision> >
driver(const CmdLineArgs& cmdlnargs, const Settings& settings, GlobalVars& globals)
{
  CompilationResult     compres    = compileResult(settings, cmdlnargs, globals);
  int                   iterations = settings.iterations;

  trace(std::cerr, "compiled ", compres.success(), '\n');

  if (!compres.success())
  {
    std::cerr << "Input file does not compile:\n"
              << compres.output()
              << std::endl;

    // do we really need to exit; maybe CompilerGPT can fix the output..
    exit(1);
  }

  std::vector<Revision>         variants = { initialAssessment(settings, cmdlnargs, globals) };
  llmtools::ConversationHistory hist = initialPrompt(settings, cmdlnargs, compres.output(), variants.back());

  try
  {
    bool stopEarly = false;

    while (iterations > 0 && !stopEarly)
    {
      --iterations;

      std::tie(hist, variants, stopEarly) = promptResponseEval( cmdlnargs,
                                                                settings,
                                                                globals,
                                                                std::move(hist),
                                                                std::move(variants),
                                                                iterations == 0
                                                              );
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

  return { std::move(hist), std::move(variants) };
}


/// main driver file
int main(int argc, char** argv)
{
  assert(argc > 0);

  // note: cxxargs must survive to allow string_views reference the values.
  const std::vector<std::string> cxxargs   = getCmdlineArgs(argv, argv+argc);
  CmdLineArgs                    cmdlnargs = parseArguments(cxxargs);

  if (cmdlnargs.showVersion)
  {
    std::cout << TOOL_VERSION << std::endl;
    return 0;
  }

  if (cmdlnargs.help)
  {
    std::cout << synopsis << std::endl
              << usage << std::endl
              << std::endl;
    return 0;
  }

  if (cmdlnargs.helpConfig)
  {
    printConfigHelp(std::cout);
    return 0;
  }

  if (cmdlnargs.helpVariables)
  {
    printVariablesHelp(std::cout);
    return 0;
  }

  // build tools configuration
  llmtools::Configurations toolsConfig = llmtools::initializeWithDefault();

  for (const std::string& configFileName: cmdlnargs.configFiles)
    toolsConfig = llmtools::initializeWithConfigFile(configFileName, std::move(toolsConfig));

  if (cmdlnargs.configCreate)
  {
    createConfigFile(toolsConfig, cmdlnargs);
    return 0;
  }

  if (cmdlnargs.all.size() == 0)
  {
    std::cout << argv[0] << std::endl
              << "  no arguments where provided" << std::endl
              << "  try --help for more information" << std::endl;
    return 1;
  }

  GlobalVars        globals;
  Settings          settings   = readSettings(cmdlnargs.configFileName);

  std::cout << "Settings: ";
  writeSettings(std::cout, cmdlnargs, settings);
  std::cout << std::endl;

  std::cout << "CmdlineArgs: " << cmdlnargs.all.back() << "@" << cmdlnargs.kernel
            << std::endl;

  const auto [conversationHistory, revisions] = driver(cmdlnargs, settings, globals);

  //
  // prepare final report

  // store query and results for review
  {
    std::ofstream histFile(settings.llmSettings.historyFile());

    histFile << conversationHistory << std::endl;
  }

  // \todo go over results and rank them based on success and results
  reportResults<ResultPrinter>(std::cout, revisions);

  if (cmdlnargs.csvsummary.size())
  {
    std::ofstream of{cmdlnargs.csvsummary, std::ios_base::app};

    of << "---" << std::endl;
    reportResults<CsvResultPrinter>(of, revisions);
  }

  std::cerr << "Timing: "
            << globals.aiTime << "ms (AI)   "
            << globals.evalTime << "ms (eval)   "
            << globals.compileTime << "ms (compile)"
            << std::endl;

  return 0;
}

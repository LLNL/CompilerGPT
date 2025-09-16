#include "llmtools.h"

#include <string>
//~ #include <iostream>

#include <boost/asio.hpp>
#include <boost/version.hpp>

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

#include <boost/json/src.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/lexical_cast.hpp>

namespace json = boost::json;
namespace boostfs = boost::filesystem;

using StringView = boost::string_view;


namespace /*anonymous*/
{
  const std::string CC_MARKER_BEGIN = "```";
  const std::string CC_MARKER_LIMIT = "```";

  using LLMSetupBase = std::tuple<const char*, const char*, const char*, const char*, const char*, const char*, const char*>;
  struct LLMSetup : LLMSetupBase
  {
    using base = LLMSetupBase;
    using base::base;

    const char* script()         const { return std::get<0>(*this); }
    const char* defaultModel()   const { return std::get<1>(*this); }
    const char* responseFile()   const { return std::get<2>(*this); }
    const char* responseField()  const { return std::get<3>(*this); }
    const char* systemTextFile() const { return std::get<4>(*this); }
    const char* roleOfAI()       const { return std::get<5>(*this); }
    const char* queryFile()      const { return std::get<6>(*this); }
  };

  using ModelSetup = std::unordered_map<llmtools::LLMProvider, LLMSetup>;

  static const ModelSetup modelSetup
      = { { llmtools::openai,     { "scripts/gpt4/exec-openai.sh", // script
                                    "gpt-4o",                      // defaultModel
                                    "response.json",               // responseFile
                                    "choices[0].message.content",  // responseField
                                    "",                            // systemTextFile
                                    "assistant",                   // role
                                    "query.json"                   // conversation history
                                  }
          },
          { llmtools::claude,     { "scripts/claude/exec-claude.sh",
                                    "claude-3-5-sonnet-20241022",
                                    "response.json",
                                    "content[0].text",
                                    "system.txt",
                                    "assistant",
                                    "query.json"
                                  }
          },
          { llmtools::ollama,     { "scripts/ollama/exec-ollama.sh",
                                    "llama3.3",
                                    "response.json",
                                    "message.content",
                                    "",
                                    "assistant",
                                    "query.json"
                                  }
          },
          { llmtools::openrouter, { "scripts/openrouter/exec-openrouter.sh",
                                    "google/gemini-2.0-flash-exp:free",
                                    "response.json",
                                    "message.content",
                                    "",
                                    "assistant",
                                    "query.json"
                                  }
          },
          { llmtools::llamaCli,   { "scripts/llama-cli/exec-llama.sh",
                                    "-hf ggml-org/gemma-3-1b-it-GGUF",
                                    "response.log",
                                    "",
                                    "system.txt",
                                    "model",
                                    "query.txt"
                                  }
          }
        };



bool fileExists(const std::string& fullPath)
{
  std::ifstream f{fullPath.c_str()};

  return f.good();
}

bool endsWith(StringView str, StringView suffix)
{
  return (  str.size() >= suffix.size()
         && std::equal(suffix.rbegin(), suffix.rend(), str.rbegin())
         );
}

bool isJsonFile(StringView filename)
{
  return endsWith(filename, ".json");
}

bool isTxtFile(StringView filename)
{
  return endsWith(filename, ".txt");
}

bool isLogFile(StringView filename)
{
  return endsWith(filename, ".log");
}

std::string checkInvocation(StringView filename)
{
  std::string scriptFile{filename};

  if (fileExists(scriptFile))
    return scriptFile;

  std::stringstream err;

  err << "Error: default script " << filename << " not found\n"
      << "modify the configuration!"
      << std::endl;

  return err.str();
}


/// separates a string \p s at whitespaces and appends them as individual
///   command line arguments to a vector \p args.
void splitArgs(const std::string& s, std::vector<std::string>& args)
{
  std::istringstream all{s};
  std::string        arg;

  while (all >> arg)
    args.emplace_back(std::move(arg));
}

/// returns the content of stream \p is as string.
std::string
readStream(std::istream& is)
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

std::string
readTxtFile(StringView filename)
{
  std::ifstream is{std::string{filename}};

  return readStream(is);
}


/// extracts a json string with a known path from a json value.
std::string jsonField(const json::value& val, StringView fld)
{
  //~ trace(std::cerr, '{', val, '}', "\n'", fld, '\n');

  if (fld.empty())
  {
    const json::string& content = val.as_string();
    StringView    contView(content.begin(), content.size());

    return std::string{contView};
  }

  if (fld[0] == '.')
    return jsonField(val, fld.substr(1));

  if (fld[0] == '[')
  {
    // must be an array index
    const std::size_t   lim = fld.find_first_of("]");
    assert((lim > 0) && (lim != StringView::npos));

    StringView          idx = fld.substr(1, lim-1);
    int                 num = boost::lexical_cast<int>(idx);
    const json::array&  arr = val.as_array();

#if CXX_20
    //~ auto                [ptr, ec] = std::from_chars(idx.data(), idx.data() + idx.size(), num);

    //~ trace(std::cerr, "i'", idx, " ", lim, '\n');

    //~ if (ec != std::errc{})
      //~ throw std::runtime_error{"Not a valid json array index (int expected)"};
#endif /*CXX_20*/

    return jsonField(arr.at(num), fld.substr(lim+1));
  }

  const std::size_t pos = fld.find_first_of(".[");

  if (pos == StringView::npos)
  {
    const json::object& obj = val.as_object();

    return jsonField(obj.at(fld), StringView{});
  }

  assert(pos != 0);
  const json::object& obj = val.as_object();
  StringView          lhs = fld.substr(0, pos);

  return jsonField(obj.at(lhs), fld.substr(pos));
}

StringView extractResponseFromLog(StringView text, StringView input)
{
  std::size_t const startOfInput = text.find(input);
  assert(startOfInput != StringView::npos);

  return text.substr(startOfInput + input.size());
}


StringView extractResponseFromLog(StringView text, StringView input, StringView delimiter)
{
  assert(delimiter.size());

  StringView        response = extractResponseFromLog(text, input);
  std::size_t const delim   = response.find(delimiter);
  assert(delim != StringView::npos);

  return response.substr(0, delim);
}

json::object
createResponse(const llmtools::Settings& settings, StringView response)
{
  json::object res;

  res["role"]    = settings.roleOfAI;
  res["content"] = response;
  return res;
}


/// loads the AI response from a JSON object
json::object
loadAIResponseJson(const llmtools::Settings& settings)
{
  std::ifstream is{settings.responseFile};
  json::value   output = llmtools::readJsonStream(is);
  json::object  res = createResponse(settings, jsonField(output, settings.responseField));

  try
  {
    std::string stopReason = jsonField(output, "stop_reason");

    res["stop_reason"] = stopReason;
  }
  catch (...) {}

  return res;
}

/// loads the AI response from a text file
json::object
loadAIResponseTxt(const llmtools::Settings& settings, const boost::json::value& query)
{
  std::string txt = readTxtFile(settings.responseFile);

  if (isLogFile(settings.responseFile))
  {
    std::string history = llmtools::lastEntry(query) + "\nmodel";

    txt = std::string{extractResponseFromLog(txt, history, "[end of text]")};
  }

  return createResponse(settings, txt);
}

/// loads the AI response
json::object
loadAIResponse(const llmtools::Settings& settings, const boost::json::value& query)
{
  if (isJsonFile(settings.responseFile))
    return loadAIResponseJson(settings);

  return loadAIResponseTxt(settings, query);
}


/// calls AI and returns result in response file
json::object
invokeAI(const llmtools::Settings& settings, const boost::json::value& query)
{
  storeQuery(settings, query);

  //~ MeasureRuntime timer(globals.aiTime);
  //~ trace(std::cerr, "CallAI: ", settings.invokeai, '\n');

  std::vector<std::string> args;

  splitArgs(settings.invokeparams, args);

  boost::asio::io_context  ios;
  std::future<std::string> outstr;
  std::future<std::string> errstr;
  std::future<int>         exitCode;
  boostprocess::child      ai( settings.invokeai,
                               boostprocess::args(args),
                               boostprocess::std_in.close(),
                               boostprocess::std_out > outstr,
                               boostprocess::std_err > errstr,
                               boostprocess::on_exit = exitCode,
                               ios
                             );

  ios.run();

  //~ std::cout << outstr.get() << std::endl;
  //~ std::cerr << errstr.get() << std::endl;

  const int  ec      = exitCode.get();

  //~ trace(std::cerr, "CallAI - exitcode: ", ec, '\n');

  if (ec != 0)
  {
    throw std::runtime_error{"AI invocation error: " + errstr.get()};
  }

  return loadAIResponse(settings, query);
}

/// appends a response \p response to a conversation history \p val.
json::value
appendResponse(json::value val, json::object response)
{
  json::array& res = val.as_array();

  res.emplace_back(std::move(response));
  return val;
}

struct CommandR
{
  StringView startOfTurnToken() const { return "<start_of_turn>"; }
  StringView endOfTurnToken()   const { return "<end_of_turn>"; }

  const json::value&           value;
  bool  withInitiateResponse = true;
  // bool               withSystem = false;
};

std::ostream& operator<<(std::ostream& os, CommandR comr)
{
  const json::array& arr = comr.value.as_array();

  for (const json::value& msg : arr)
  {
    const json::object& obj = msg.as_object();

    // Extract role and content
    std::string role    = jsonField(obj, "role");
    std::string content = jsonField(obj, "content");

    assert(!role.empty() && !content.empty());

    if (role == "system")
      continue;

    os << comr.startOfTurnToken() << role
       << "\n" << content << comr.endOfTurnToken()
       << std::endl;
  }

  if (comr.withInitiateResponse)
    os << comr.startOfTurnToken() << "model"
       << std::endl;

  return os;
}


using PromptVariableBase = std::tuple<std::size_t, std::string>;

/// encapsulates any text placeholder that gets substituted with
///   programmatic information (reports, source code).
struct PromptVariable : PromptVariableBase
{
  using base = PromptVariableBase;
  using base::base;

  std::size_t      offsetInString() const { return std::get<0>(*this); }
  std::string_view token()          const { return std::get<1>(*this); }
};


PromptVariable
nextVariable(std::string_view prompt, const llmtools::VariableMap& m)
{
  if (std::size_t pos = prompt.find("<<"); pos != std::string_view::npos)
  {
    if (std::size_t lim = prompt.find(">>", pos+2); lim != std::string_view::npos)
    {
      std::string_view cand = prompt.substr(pos+2, lim-(pos+2));

      if (m.find(std::string(cand)) != m.end())
        return PromptVariable{pos, cand};
    }
  }

  return PromptVariable{prompt.size(), ""};
}

std::vector<std::string>
readSourceFromStream(std::istream& is)
{
  std::vector<std::string> res;
  std::string line;

  while (std::getline(is, line))
    res.push_back(line);

  return res;
}


}


namespace llmtools
{


boost::json::value
queryResponse(const Settings& settings, boost::json::value query)
{
  json::object response = invokeAI(settings, query);

  return appendResponse(std::move(query), std::move(response));
}

/// processes a JSON stream.
json::value
readJsonStream(std::istream& is)
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
      //~ std::cerr << "ec = " << ec << std::endl;
      throw std::runtime_error("unable to parse JSON file: " + line);
    }
  }

  p.finish(ec);

  if (ec)
  {
    //~ std::cerr << "ec = " << ec << std::endl;
    throw std::runtime_error("unable to finish parsing JSON file");
  }

  return p.release();
}

/// writes out conversation history to a file so it can be used for the
///   next AI invocation.

void storeQuery(const std::string& historyfile, const boost::json::value& conversationHistory)
{
  std::ofstream out{historyfile};

  if (isJsonFile(historyfile))
    out << conversationHistory << std::endl;
  else if (isTxtFile(historyfile))
    out << CommandR{conversationHistory} << std::endl;
  else
    throw std::runtime_error{"Unknown history file format (file extension not in {.json, .txt})."};
}

void storeQuery(const Settings& settings, const json::value& history)
{
  storeQuery(settings.historyFile, history);
}


std::string defaultModel(LLMProvider provider)
{
  return modelSetup.at(provider).defaultModel();
}

// setup connection for curl scripts
Settings configure(const std::string& basePath, LLMProvider provider, const std::string& llmmodel)
{
  const LLMSetup& setup    = modelSetup.at(provider);
  std::string     invokeai = basePath + "/" + setup.script();
  std::string     model    = llmmodel;

  if (model.empty())
    model = setup.defaultModel();

  //~ settings.llmSettings.invokeai       = invokeai;
  //~ settings.llmSettings.responseFile   = "response.json";
  //~ settings.llmSettings.responseField  = setup.responseField();
  //~ settings.llmSettings.systemTextFile = setup.systemTextFile();
  //~ settings.llmSettings.roleOfAI       = "assistant";
  //~ settings.llmSettings.invokeai       = invokeai + " " + model;

  return { checkInvocation(invokeai),
           model,
           setup.responseFile(),
           setup.responseField(),
           setup.roleOfAI(),
           setup.systemTextFile(),
           setup.queryFile()
         };
}


  /// gets the last response from the \p conversationHistory
std::string
lastEntry(const boost::json::value& conversationHistory)
{
  const json::array&  arr = conversationHistory.as_array();
  const json::value&  last = arr.back();
  const json::object& obj = last.as_object();
  const json::string  str = obj.at("content").as_string();

  return std::string(str.data(), str.size());
}


boost::json::value
createConversationHistory(const Settings& settings, const std::string& systemText)
{
  json::array res;

  if (settings.systemTextFile.empty())
  {
    json::object line;

    line["role"]    = "system";
    line["content"] = systemText;

    res.emplace_back(std::move(line));
  }
  else
  {
    std::ofstream ofs{settings.systemTextFile};

    ofs << systemText;
  }

  return res;
}


boost::json::value
appendPrompt(boost::json::value conversationHistory, const std::string& prompt)
{
  json::array& convHistory = conversationHistory.as_array();
  json::object promptValue;

  promptValue["role"]    = "user";
  promptValue["content"] = prompt;

  convHistory.emplace_back(std::move(promptValue));
  return conversationHistory;
}

LLMProvider provider(const std::string& providerName)
{
  if (  (boost::iequals(providerName, "openai"))
     || (boost::iequals(providerName, "chatgpt"))
     )
    return openai;

  if (  (boost::iequals(providerName, "anthropic"))
     || (boost::iequals(providerName, "claude"))
     )
    return claude;

  if (boost::iequals(providerName, "openrouter"))
    return openrouter;

  if (boost::iequals(providerName, "ollama"))
    return ollama;

  if (  (boost::iequals(providerName, "llamacli"))
     || (boost::iequals(providerName, "llama-cli"))
     )
    return llamaCli;

  if (  (boost::iequals(providerName, "custom"))
     || (boost::iequals(providerName, "user-defined"))
     || (boost::iequals(providerName, "userdefined"))
     )
   return LLMnone;

  return LLMerror;
}


/// replaces known placeholders with their text
std::string
expandPrompt(const std::string& rawprompt, const VariableMap& m)
{
  std::stringstream txt;
  std::string_view  prompt = rawprompt;
  PromptVariable    var    = nextVariable(prompt, m);

  while (var.token().size() != 0)
  {
    const std::size_t prefixlen = var.offsetInString() + var.token().size() + 4 /* "<<" and ">>" */;

    txt << prompt.substr(0, var.offsetInString())
        << m.at(std::string(var.token()));

    prompt.remove_prefix(prefixlen);
    var = nextVariable(prompt, m);
  }

  txt << prompt;
  return txt.str();
}


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

std::ostream&
operator<<(std::ostream& os, SourcePoint p)
{
  if (p == SourcePoint::origin())
    return os << "\u03b1";

  if (p == SourcePoint::eof())
    return os << "\u03a9";

  return os << p.line() << ":" << p.col();
}

bool SourceRange::entireFile() const
{
  return (  (beg() == SourcePoint::origin())
         && (lim() == SourcePoint::eof())
         );
}

std::ostream&
operator<<(std::ostream& os, SourceRange p)
{
  return os << p.beg() << "-" << p.lim();
}

/// loads the specified subsection of a code into a string.
std::string
fileToMarkdown(const std::string& langmarker, std::istream& src, SourceRange rng)
{
  std::stringstream txt;

  txt << CC_MARKER_BEGIN << langmarker
      << "\n";

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
  return txt.str();
}

std::string
fileToMarkdown(const std::string& langmarker, const std::string& filename, SourceRange rng)
{
  std::ifstream src{std::string(filename)};

  return fileToMarkdown(langmarker, src, rng);
}


std::vector<CodeSection>
extractCodeSections(const std::string& markdownText)
{
  std::vector<CodeSection> res;
  StringView               text = markdownText;

  while (true)
  {
    // Find the opening ```
    const std::size_t secbeg = text.find(CC_MARKER_BEGIN);
    if (secbeg == std::string::npos)
      break;

    // Find the end of the opening line (could have language marker)
    const std::size_t postmarker = secbeg + CC_MARKER_BEGIN.size();
    const std::size_t eoLine = text.find('\n', postmarker);
    if (eoLine == std::string::npos)
      break;

    // Extract language marker (if any)
    StringView langMarker = text.substr(postmarker, eoLine - postmarker);

    // Remove leading/trailing whitespace from language marker
    {
      const auto lmrend = langMarker.rend();
      const int  endPos = std::distance(std::find_if_not(langMarker.rbegin(), lmrend, std::iswspace), lmrend);
      const auto lmbeg  = langMarker.begin();
      const int  begPos = std::distance(lmbeg, std::find_if_not(lmbeg,  langMarker.end(), std::iswspace));
      const int  newLen = std::max(endPos - begPos, 0);

      // Create a new string_view representing the trimmed portion
      langMarker = langMarker.substr(begPos, newLen);
    }

    // Find the closing ```
    const std::size_t seclim = text.find(CC_MARKER_LIMIT, eoLine + 1);
    if (seclim == std::string::npos)
      break;

    // Extract code
    StringView        code = text.substr(eoLine + 1, seclim - (eoLine + 1));

    res.emplace_back(langMarker, code);

    // Move pos past the closing ```
    text.remove_prefix(seclim + CC_MARKER_LIMIT.size());
  }

  return res;
}

SourceRange
replaceSourceSection(std::ostream& os, std::istream& is, SourceRange sourceRange, const CodeSection& codesec)
{
  // Get start and end points of the range to replace
  SourcePoint beg = sourceRange.beg();
  SourcePoint lim = sourceRange.lim();

  const std::vector<std::string> sourceLines = readSourceFromStream(is);
  auto linePrinter =
          [&os](const std::string& line)->void
          {
            os << line << std::endl;
          };

  // Write lines before the replacement
  assert(sourceLines.size() >= beg.line());
  auto const prebeg = sourceLines.begin();
  auto const prelim = sourceLines.begin()+beg.line();

  std::for_each(prebeg, prelim, linePrinter);

  // Write the part of the line before the replacement (if any)
  // os << inputLines.at(beg.line()).substr(0, beg.col());

  std::size_t kernellen = printUnescaped(os, codesec.code());
  SourcePoint newlim    = SourcePoint::eof();
  const bool  fullrange = lim == newlim;

  if (!fullrange)
  {
    assert(sourceLines.size() >= lim.line());
    auto const postbeg = sourceLines.begin()+lim.line();
    auto const postlim = sourceLines.end();

    std::for_each(postbeg, postlim, linePrinter);
    newlim = llmtools::SourcePoint{beg.line() + kernellen, 0};
  }

  return { beg, newlim };
}


/// prints the code from \p code to the stream \p os while unescaping
///   escaped characters.
/// \return the number of lines printed.
std::size_t
printUnescaped(std::ostream& os, const std::string& srccode)
{
  StringView  code = srccode;
  std::size_t linecnt = 1;
  char        last = ' ';
  bool        lastIsLineBreak = false;

  // print to os while handling escaped characters
  for (char ch : code)
  {
    lastIsLineBreak = false;

    if (last == '\\')
    {
      switch (ch)
      {
        case 'f':  /* form feed */
                   ++linecnt;
                   os << '\n';
                   [[fallthrough]];

        case 'n':  ++linecnt;
                   os << '\n';
                   lastIsLineBreak = true;
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
    else if (ch == '\n')
    {
      os << ch;
      ++linecnt;
      lastIsLineBreak = true;
    }
    else if (ch == '\\')
      last = ch;
    else
      os << ch;
  }

  if (!lastIsLineBreak) os << '\n';

  return linecnt;
}



}

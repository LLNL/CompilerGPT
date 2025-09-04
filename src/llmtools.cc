#include "llmtools.h"

#include <string>

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

/// loads the AI response from a JSON object
std::string loadAIResponseJson(const llmtools::Settings& settings)
{
  std::ifstream is{settings.responseFile};

  return jsonField(llmtools::readJsonStream(is), settings.responseField);
}

/// loads the AI response from a text file
std::string loadAIResponseTxt(const llmtools::Settings& settings, const boost::json::value& query)
{
  std::string res = readTxtFile(settings.responseFile);

  if (isLogFile(settings.responseFile))
  {
    std::string history = llmtools::lastEntry(query) + "\nmodel";

    res = std::string{extractResponseFromLog(res, history, "[end of text]")};
  }

  return res;
}

/// loads the AI response
std::string loadAIResponse(const llmtools::Settings& settings, const boost::json::value& query)
{
  if (isJsonFile(settings.responseFile))
    return loadAIResponseJson(settings);

  return loadAIResponseTxt(settings, query);
}


/// calls AI and returns result in response file
std::string invokeAI(const llmtools::Settings& settings, const boost::json::value& query)
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

  if (ec != 0) throw std::runtime_error{"AI invocation error."};

  return loadAIResponse(settings, query);
}

/// appends a response \p response to a conversation history \p val.
json::value
appendResponse(const llmtools::Settings& settings, json::value val, std::string response)
{
  json::array& res = val.as_array();
  json::object line;

  line["role"]    = settings.roleOfAI;
  line["content"] = response;

  res.emplace_back(std::move(line));
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


}


namespace llmtools
{


boost::json::value
queryResponse(const Settings& settings, boost::json::value query)
{
  std::string response = invokeAI(settings, query);

  query = appendResponse(settings, std::move(query), response);
  return query;
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


}

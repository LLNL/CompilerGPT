#include "llmtools.hpp"

#include <dlfcn.h>

#include <string>
#include <algorithm>
#include <numeric>
//~ #include <iostream> // for debugging only

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

//~ #include <boost/json/src.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/optional.hpp>

namespace json    = boost::json;
namespace adapt   = boost::adaptors;

using StringView  = boost::string_view;

template <class T>
using Optional = boost::optional<T>;

namespace
{

const std::string VAR_PREFIX = "${";
const std::string VAR_SUFFIX = "}";

std::string
getBaseDirectory(const std::string& path)
{
  std::size_t pos = path.find_last_of('/');
  if (pos == std::string::npos)
    return "";

  return path.substr(0, pos) + "/..";
}

}

std::string
llmtoolsLibraryBasePath()
{
  Dl_info info;

  if (dladdr((void*)&llmtoolsLibraryBasePath, &info))
    return getBaseDirectory(info.dli_fname);

  throw std::runtime_error{"Could not determine llmtools-library path"};
}


namespace /*anonymous*/
{
const std::string CC_MARKER_BEGIN = "```";
const std::string CC_MARKER_LIMIT = "```";

const char* const JK_KEY              = "key";
const char* const JK_EXEC             = "exec";
const char* const JK_EXEC_FLAGS       = "execFlags";
const char* const JK_RESPONSE_FILE    = "responseFile";
const char* const JK_RESPONSE_FIELD   = "responseField";
const char* const JK_SYSTEM_TEXT_FILE = "systemTextFile";
const char* const JK_ROLE_OF_AI       = "roleOfAI";
const char* const JK_HISTORY_FILE     = "historyFile";
const char* const JK_API_KEY_NAME     = "apiKeyName";
const char* const JK_PROMPT_FILE      = "promptFile";
const char* const JK_MODEL_NAME       = "modelName";
const char* const JK_ALT_NAMES        = "alternativeNames";

const char* const JK_PF_FILENAME      = "filename";
const char* const JK_PF_FORMAT        = "format";
const char* const JK_HIST_ROLE_KEY    = "role";
const char* const JK_HIST_CONTENT_KEY = "content";

StringView
toStringView(const boost::json::value& val)
{
  const boost::json::string& s = val.as_string();

  return StringView(s.begin(), s.size());
}

StringView
fieldToStringView(const boost::json::object& val, const char* fieldName)
{
  return toStringView(val.at(fieldName));
}

Optional<StringView>
fieldToStringViewOpt(const boost::json::object& val, const char* fieldName)
{
  auto pos = val.find(fieldName);

  if (pos != val.end())
    return toStringView(pos->value());

  return {};
}

struct LLMSetup
{
    LLMSetup(StringView key, const boost::json::value& cnf)
    : val(cnf.as_object())
    {
      val[JK_KEY] = std::string(key);
    }

    StringView canonicalName()  const { return fieldToStringView(val, JK_KEY); }
    StringView modelName()      const { return fieldToStringView(val, JK_MODEL_NAME); }
    StringView responseFile()   const { return fieldToStringView(val, JK_RESPONSE_FILE); }
    StringView responseField()  const { return fieldToStringView(val, JK_RESPONSE_FIELD); }
    StringView roleOfAI()       const { return fieldToStringView(val, JK_ROLE_OF_AI); }
    StringView historyFile()    const { return fieldToStringView(val, JK_HISTORY_FILE); }
    StringView exec()           const { return fieldToStringView(val, JK_EXEC); }

    Optional<StringView>
    execFlags()                 const { return fieldToStringViewOpt(val, JK_EXEC_FLAGS); }

    Optional<StringView>
    systemTextFile() const { return fieldToStringViewOpt(val, JK_SYSTEM_TEXT_FILE); }

    Optional<StringView>
    API_key_name()   const { return fieldToStringViewOpt(val, JK_API_KEY_NAME); }

    boost::json::value
    promptFile()   const { return llmtools::loadField(val, JK_PROMPT_FILE, boost::json::value{}); }

  private:
    boost::json::object val;
};

bool
fileExists(const std::string& fullPath)
{
  std::ifstream f{fullPath.c_str()};

  return f.good();
}

bool
absolutePath(const std::string& path)
{
  return path.size() && (path.front() == '/');
}

bool
endsWith(StringView str, StringView suffix)
{
  return (  str.size() >= suffix.size()
         && std::equal(suffix.rbegin(), suffix.rend(), str.rbegin())
         );
}

bool
isJsonFile(StringView filename)
{
  return endsWith(filename, ".json");
}

bool
isTxtFile(StringView filename)
{
  return endsWith(filename, ".txt");
}

bool
isLogFile(StringView filename)
{
  return endsWith(filename, ".log");
}

std::string
checkInvocation(StringView filename)
{
  std::string LLMTOOLS_HOME = "${LLMTOOLS:HOME}";
  std::string execFile{filename};

  if (execFile.rfind(LLMTOOLS_HOME,0) == 0)
    execFile = llmtoolsLibraryBasePath() + execFile.substr(LLMTOOLS_HOME.size());

  if (!absolutePath(execFile))
  {
    std::string fullPath = boostprocess::search_path(execFile).string();

    if (!fullPath.empty())
      return fullPath;
  }
  else if (fileExists(execFile))
    return execFile;

  std::stringstream err;

  err << "Error: default executable '" << filename << "' not found\n"
      << "  looking for '" << execFile << "'\n"
      << std::endl;

  throw std::runtime_error{err.str()};
}



/// Separates a string \p s at whitespaces and appends them as individual
///   command line arguments to a vector \p args. Strings within " quotes
///   are not split and written out to args as single string while removing
///   quotes at the beginning and end.
/// \param  s a sequence of command line arguments
/// \param  args a vector to which the parsed arguments will be appended
/// \throws std::runtime_error if a quoted string is not properly closed
/// \details
///   If the string s contains a quoted string, such as "-H x" the whitespaces
///     inside the quoted string are ignored.
///   Escaped quotes (\") remain unchanged.
void
splitArgs(const std::string& s, std::vector<std::string>& args)
{
  std::string current;
  bool        inQuotes = false;
  bool        escaped  = false;

  for (char c : s)
  {
    if (escaped)
    {
      current += '\\';
      current += c;

      escaped = false;
    }
    else if (c == '\\')
    {
      escaped = true;
    }
    else if (c == '"')
    {
      // Toggle quote mode
      inQuotes = !inQuotes;
    }
    else if (std::isspace(static_cast<unsigned char>(c)) && !inQuotes)
    {
      // If we hit whitespace outside of quotes and have collected characters,
      // store the current argument and reset
      if (!current.empty())
      {
        std::string arg;

        std::swap(arg, current);
        args.push_back(std::move(arg));
      }
    }
    else
    {
      // Add character to the current argument
      current += c;
    }
  }

  // Don't forget the last argument if there is one
  if (!current.empty())
    args.push_back(std::move(current));

  // Check if we have unclosed quotes
  if (inQuotes)
    throw std::runtime_error("Unclosed quotes in input string '" + s + "'");

  if (escaped)
    throw std::runtime_error("Unprocessed escaped character at end of input string '" + s + "'");
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
const json::value&
jsonElement(const json::value& val, StringView accessPath)
{
  if (accessPath.empty())
    return val;

  if (accessPath[0] == '.')
    return jsonElement(val, accessPath.substr(1));

  if (accessPath[0] == '[')
  {
    // must be an array index
    const std::size_t   lim = accessPath.find_first_of("]");
    assert((lim > 0) && (lim != StringView::npos));

    StringView          idx = accessPath.substr(1, lim-1);
    int                 num = boost::lexical_cast<int>(idx);
    const json::array&  arr = val.as_array();

#if CXX_20
    //~ auto                [ptr, ec] = std::from_chars(idx.data(), idx.data() + idx.size(), num);

    //~ trace(std::cerr, "i'", idx, " ", lim, '\n');

    //~ if (ec != std::errc{})
      //~ throw std::runtime_error{"Not a valid json array index (int expected)"};
#endif /*CXX_20*/

    return jsonElement(arr.at(num), accessPath.substr(lim+1));
  }

  const std::size_t pos = accessPath.find_first_of(".[");

  if (pos == StringView::npos)
    return val.as_object().at(accessPath);

  assert(pos != 0);
  const json::object& obj = val.as_object();
  StringView          lhs = accessPath.substr(0, pos);

  return jsonElement(obj.at(lhs), accessPath.substr(pos));
}

std::string
jsonString(const json::value& val, StringView accessPath)
{
  return std::string(jsonElement(val, accessPath).as_string());
}

StringView
extractResponseFromLog(StringView text, StringView input)
{
  std::size_t const startOfInput = text.find(input);
  assert(startOfInput != StringView::npos);

  return text.substr(startOfInput + input.size());
}


StringView
extractResponseFromLog(StringView text, StringView input, StringView delimiter)
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

  res[JK_HIST_ROLE_KEY]    = settings.roleOfAI();
  res[JK_HIST_CONTENT_KEY] = response;
  return res;
}


/// loads the AI response from a JSON object
json::object
loadAIResponseJson(const llmtools::Settings& settings)
{
  json::value   output = llmtools::readJsonFile(settings.responseFile());
  json::object  res = createResponse(settings, jsonString(output, settings.responseField()));

#if 0
  try
  {
    std::string stopReason = jsonString(output, "stop_reason");

    res["stop_reason"] = stopReason;
  }
  catch (...) {}
#endif

  return res;
}

/// loads the AI response from a text file
json::object
loadAIResponseTxt(const llmtools::Settings& settings, const boost::json::value& query)
{
  std::string txt = readTxtFile(settings.responseFile());

  if (isLogFile(settings.responseFile()))
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
  if (isJsonFile(settings.responseFile()))
    return loadAIResponseJson(settings);

  return loadAIResponseTxt(settings, query);
}

std::string
environmentVariable(const std::string& varname, std::string alt = {})
{
  if (varname.size())
    if (const char* var = std::getenv(varname.c_str()))
      return var;

  return alt;
}

void
writePromptFile( const std::string& promptFileName,
                 boost::json::value structure,
                 const llmtools::VariableMap& vars,
                 const boost::json::value& query
               )
{
  if (!isJsonFile(promptFileName))
    return;

  boost::json::object* obj = structure.if_object();
  if (obj == nullptr) return;

  boost::json::value&  format = obj->at(JK_PF_FORMAT);
  boost::json::object& fmtobj = format.as_object();

  for (auto& [key, value] : fmtobj)
  {
    boost::json::string* str = value.if_string();
    if (str == nullptr) continue;

    // \todo consider integrating json objects into text expansion if possible..
    if (*str == "${LLMTOOLS:HISTORY}")
      value = query;
    else
      *str = llmtools::expandText(std::string(*str), vars);
  }

  std::ofstream promptFile{promptFileName};

  promptFile << format
             << std::endl;
}

std::string
fileNameOpt(const boost::json::value& structure, const std::string& alt)
{
  if (const boost::json::string* str = structure.if_string())
    return std::string(*str);

  if (const boost::json::object* obj = structure.if_object())
    return llmtools::loadField(*obj, JK_PF_FILENAME, alt);

  return alt;
}


/// calls AI and returns result in response file
json::object
invokeAI(const llmtools::Settings& settings, const boost::json::value& query)
{
  storeQuery(settings, query);

  std::string              promptFileName = fileNameOpt(settings.promptFile(), "");
  llmtools::VariableMap    vars =
      { {"LLMTOOLS:MODEL",            settings.modelName() },
        {"LLMTOOLS:API_KEY",          environmentVariable(settings.apiKeyName()) },
        {"LLMTOOLS:PROMPT_FILE",      promptFileName },
        {"LLMTOOLS:HISTORY",          settings.historyFile() },
        {"LLMTOOLS:RESPONSE_FILE",    settings.responseFile() },
        {"LLMTOOLS:SYSTEM_TEXT_FILE", settings.systemTextFile() }
      };

  writePromptFile(promptFileName, settings.promptFile(), vars, query);

  std::string              execFlags = llmtools::expandText(settings.execFlags(), vars);
  std::vector<std::string> args;

  splitArgs(execFlags, args);

  std::string const        redirectStdOut = ">";
  std::string              stdlog;

  if (args.size() && (args.back().rfind(redirectStdOut,0) == 0))
  {
    stdlog = args.back().substr(redirectStdOut.size());
    args.pop_back();
  }

  boost::asio::io_context  ios;
  std::future<std::string> outstr;
  std::future<std::string> errstr;
  std::future<int>         exitCode;
  boostprocess::child      ai( settings.exec(),
                               boostprocess::args(args),
                               boostprocess::std_in.close(),
                               boostprocess::std_out > outstr,
                               boostprocess::std_err > errstr,
                               boostprocess::on_exit = exitCode,
                               ios
                             );

  ios.run();

  const int ec = exitCode.get();

  if (ec != 0)
    throw std::runtime_error{"AI invocation error " + std::to_string(ec) + " " + errstr.get()};

  if (stdlog.size())
  {
    // assert stdlog == settings.responseFile() ?
    std::ofstream log{stdlog};

    log << outstr.get() << std::endl;
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

std::ostream&
operator<<(std::ostream& os, CommandR comr)
{
  const json::array& arr = comr.value.as_array();

  for (const json::value& msg : arr)
  {
    const json::object& obj = msg.as_object();

    // Extract role and content
    std::string role    = jsonString(obj, JK_HIST_ROLE_KEY);
    std::string content = jsonString(obj, JK_HIST_CONTENT_KEY);

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
  if (std::size_t pos = prompt.find(VAR_PREFIX); pos != std::string_view::npos)
  {
    std::size_t postPrefix = pos + VAR_PREFIX.size();

    if (std::size_t lim = prompt.find(VAR_SUFFIX, postPrefix); lim != std::string_view::npos)
    {
      std::string_view cand = prompt.substr(postPrefix, lim-(postPrefix));

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

/// returns true if line is empty or only contains whitespace characters
bool isEmptyLine(std::string_view line)
{
  return std::all_of( line.begin(), line.end(),
                      [](unsigned char c) { return std::isspace(c); }
                    );
}

/// merges the elements in the json objects \p lhs and \p rhs into a combined object.
/// \param  lhs a json object
/// \param  rhs a json object
/// \result the merged json objects
/// \throw  a std::invalid_argument exception if any of the input values are not of type
//          boost::json::object.
/// \throw  a runtime exception of \p lhs and \p rhs contain elements with the same key.
boost::json::value
mergeJson(boost::json::value lhs, boost::json::value rhs)
{
  // Validate that both inputs are objects
  if (!lhs.is_object())
    throw std::invalid_argument("Left-hand side input is not a JSON object");

  if (!rhs.is_object())
    throw std::invalid_argument("Right-hand side input is not a JSON object");

  boost::json::object& lhsobj = lhs.as_object();

  // Merge in the elements from rhs, checking for duplicates
  for (auto& [key, value] : rhs.as_object())
  {
    // Check for duplicate keys
    auto const pos = lhsobj.find(key);

    if (pos != lhsobj.end())
    {
      std::stringstream err;

      err << "Duplicate key found during JSON merge: " << key;
      throw std::runtime_error(err.str());
    }

    // Add the key-value pair from rhs to the result
    lhsobj.emplace(std::move(key), std::move(value));
  }

  return lhs;
}


/// returns the json object on \p config with a \key provider.
/// \param config a json object
/// \param provider the identifier of a subobject in \p config
/// \return the subobject in \p config with key or alternative key matching \p provider.
/// \throws std::invalid_argument if config is not a JSON object.
/// \throws std::runtime_error no matching subobject can be identified.
/// \details
///   returns the subobject in \p config that has key with the same name as provider.
///   if no such subobject exists, findProvider performs a case-insensitive match
///   against a field 'alternative_names'. The field 'alternative_names'
///   is an optional field in each subobject in \p config, and may be a string
///   or an array of string.
LLMSetup
findProvider(const boost::json::object& config, const std::string& provider)
{
  // First, check for exact match with provider name
  auto const pos = config.find(provider);

  if (pos != config.end())
    return LLMSetup(provider, pos->value());

  // Check each subobject for 'alternative_names' field
  for (const auto& [key, value] : config)
  {
    // skip non-object values
    const boost::json::object* subObj = value.if_object();
    if (subObj == nullptr)
      continue;

    auto const altPos = subObj->find(JK_ALT_NAMES);
    if (altPos == subObj->end())
      continue;

    const boost::json::value& altNames = altPos->value();

    if (const boost::json::string* str = altNames.if_string())
    {
      // If alternative_names is a string, check for case-insensitive match
      if (boost::iequals(*str, provider))
        return LLMSetup(key, value);
    }
    else if (const boost::json::array* arr = altNames.if_array())
    {
      // If alternative_names is an array, check each element
      for (const auto& altName : *arr)
      {
        if (const boost::json::string* str = altName.if_string())
          if (boost::iequals(*str, provider))
            return LLMSetup(key, value);
      }
    }
  }

  // If we reach here, no provider was found
  throw std::runtime_error("No matching provider found: " + provider);
}

LLMSetup
findProvider(const llmtools::Configurations& config, const std::string& provider)
{
  return findProvider(config.as_object(), provider);
}


std::string
toString(Optional<StringView> opt)
{
  if (opt) return std::string(*opt);

  return {};
}


} // anonymous namespace

namespace llmtools
{

const char* const LLMnone = "<none>";
const char* const LLMerror = "<error!>";

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
  std::size_t                cnt = 0;

  while (std::getline(is, line))
  {
    ++cnt;
    p.write(line, ec);

    if (ec)
      throw std::runtime_error("error on line line " + std::to_string(cnt) + "\n    line: " + line);
  }

  p.finish(ec);

  if (ec)
    throw std::runtime_error("unable to finish parsing JSON file");

  return p.release();
}

boost::json::value
readJsonFile(const std::string& fileName)
{
  if (fileName.empty())
    throw std::runtime_error{"invalid empty filename"};

  std::ifstream ifs(fileName);

  if (!ifs.good())
    throw std::runtime_error{"File " + fileName + " is not accessible"};

  try
  {
    return readJsonStream(ifs);
  }
  catch (const std::runtime_error& e)
  {
    throw std::runtime_error(std::string(e.what()) + "\n    file: " + fileName);
  }
}

std::ostream&
operator<<(std::ostream& os, SettingsJsonFieldWriter wr)
{
  std::string prefix(wr.indent, ' ');

  prefix.insert(0, 1, '\n');

  return os << prefix << "\"" << JK_EXEC << "\":"             << boost::json::string(wr.settings.exec()) << ","
            << prefix << "\"" << JK_EXEC_FLAGS << "\":"       << boost::json::string(wr.settings.execFlags()) << ","
            << prefix << "\"" << JK_RESPONSE_FILE << "\":"    << boost::json::string(wr.settings.responseFile()) << ","
            << prefix << "\"" << JK_RESPONSE_FIELD << "\":"   << boost::json::string(wr.settings.responseField()) << ","
            << prefix << "\"" << JK_SYSTEM_TEXT_FILE << "\":" << boost::json::string(wr.settings.systemTextFile()) << ","
            << prefix << "\"" << JK_ROLE_OF_AI << "\":"       << boost::json::string(wr.settings.roleOfAI()) << ","
            << prefix << "\"" << JK_HISTORY_FILE << "\":"     << boost::json::string(wr.settings.historyFile()) << ","
            << prefix << "\"" << JK_API_KEY_NAME << "\":"     << boost::json::string(wr.settings.apiKeyName()) << ","
            << prefix << "\"" << JK_MODEL_NAME << "\":"       << boost::json::string(wr.settings.modelName()) << ","
            << prefix << "\"" << JK_PROMPT_FILE << "\":"      << wr.settings.promptFile()
            ;
}


Settings
settings(const json::value& cnf, const Settings& oldSettings)
{
  Settings res;

  res.exec()           = loadField(cnf, JK_EXEC,             oldSettings.exec());
  res.execFlags()      = loadField(cnf, JK_EXEC_FLAGS,       oldSettings.execFlags());
  res.responseFile()   = loadField(cnf, JK_RESPONSE_FILE,    oldSettings.responseFile());
  res.responseField()  = loadField(cnf, JK_RESPONSE_FIELD,   oldSettings.responseField());
  res.systemTextFile() = loadField(cnf, JK_SYSTEM_TEXT_FILE, oldSettings.systemTextFile());
  res.roleOfAI()       = loadField(cnf, JK_ROLE_OF_AI,       oldSettings.roleOfAI());
  res.historyFile()    = loadField(cnf, JK_HISTORY_FILE,     oldSettings.historyFile());
  res.apiKeyName()     = loadField(cnf, JK_API_KEY_NAME,     oldSettings.apiKeyName());
  res.modelName()      = loadField(cnf, JK_MODEL_NAME,       oldSettings.modelName());
  res.promptFile()     = loadField(cnf, JK_PROMPT_FILE,      oldSettings.promptFile());

  return res;
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
  storeQuery(settings.historyFile(), history);
}


std::string defaultModel(const Configurations& configs, const LLMProvider& provider)
{
  return std::string(findProvider(configs, provider).modelName());
}

Configurations
initializeWithDefault(Configurations current)
{
  return mergeJson(std::move(current), readJsonFile(llmtoolsLibraryBasePath() + "/etc/llmtools/llmtools-default.json"));
}

Configurations
initializeWithConfigFile(const std::string& configFileName, Configurations current)
{
  return mergeJson(std::move(current), readJsonFile(configFileName));
}

Settings
configure(const Configurations& configs, const std::string& provider, const std::string& llmmodel)
{
  const LLMSetup& setup    = findProvider(configs, provider);
  std::string     exeName  = checkInvocation(setup.exec());
  std::string     model    = llmmodel;

  if (model.empty())
    model = std::string(setup.modelName());

  return { exeName,
           toString(setup.execFlags()),
           std::string(setup.responseFile()),
           std::string(setup.responseField()),
           std::string(setup.roleOfAI()),
           toString(setup.systemTextFile()),
           std::string(setup.historyFile()),
           toString(setup.API_key_name()),
           model,
           setup.promptFile()
         };
}


/// gets the last response from the \p conversationHistory
std::string
lastEntry(const boost::json::value& conversationHistory)
{
  const json::array&  arr = conversationHistory.as_array();
  const json::value&  last = arr.back();
  const json::object& obj = last.as_object();
  const json::string  str = obj.at(JK_HIST_CONTENT_KEY).as_string();

  return std::string(str.data(), str.size());
}


boost::json::value
createConversationHistory(const Settings& settings, const std::string& systemText)
{
  json::array res;

  if (settings.systemTextFile().empty())
  {
    json::object line;

    line[JK_HIST_ROLE_KEY]    = "system";
    line[JK_HIST_CONTENT_KEY] = systemText;

    res.emplace_back(std::move(line));
  }
  else
  {
    std::ofstream ofs{settings.systemTextFile()};

    ofs << systemText;
  }

  return res;
}


boost::json::value
appendPrompt(boost::json::value conversationHistory, const std::string& prompt)
{
  json::array& convHistory = conversationHistory.as_array();
  json::object promptValue;

  promptValue[JK_HIST_ROLE_KEY]    = "user";
  promptValue[JK_HIST_CONTENT_KEY] = prompt;

  convHistory.emplace_back(std::move(promptValue));
  return conversationHistory;
}

LLMProvider
provider(const Configurations& configs, const std::string& providerName)
{
  LLMSetup setup = findProvider(configs, providerName);

  if (setup.API_key_name())
  {
    // check if the API key is defined
    std::string keyName = std::string(*setup.API_key_name());

    if (std::getenv(keyName.c_str()) == nullptr)
      throw std::runtime_error{"API KEY '" + keyName + "' is undefined in environment."};
  }

  return LLMProvider(setup.canonicalName());
}

/// replaces known placeholders with their text
std::string
expandText(const std::string& txt, const VariableMap& m)
{
  std::stringstream  res;
  std::string_view   prompt   = txt;
  PromptVariable     var      = nextVariable(prompt, m);
  const std::uint8_t varExtra = VAR_PREFIX.size() + VAR_SUFFIX.size();

  while (var.token().size() != 0)
  {
    const std::size_t varLen = var.offsetInString() + var.token().size() + varExtra;

    res << prompt.substr(0, var.offsetInString())
        << m.at(std::string(var.token()));

    prompt.remove_prefix(varLen);
    var = nextVariable(prompt, m);
  }

  res << prompt;
  return res.str();
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

// static
SourceRange
SourceRange::all()
{
  return { SourcePoint::origin(), SourcePoint::eof() };
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
  std::size_t       numNonEmpty = 0; // source code starts at line 1

  // skip beginning lines
  while ((linectr < begLine) && std::getline(src, line)) ++linectr;

  // copy code segment
  while ((linectr < limLine) && std::getline(src, line))
  {
    numNonEmpty += int(isEmptyLine(line) == false);
    ++linectr;
    txt << line << "\n";
  }

  if (numNonEmpty == 0)
    return {};

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

  // print to os while handling (some) escaped characters
  //   some escaped characters are commented are not handled in order not
  //   to distort doxygen marks.
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

        //~ case 't':  os << "  ";
                   //~ break;

        //~ case 'a':  /* bell */
        //~ case 'v':  /* vertical tab */
        //~ case 'r':  /* carriage return */
                   //~ break;

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

/// queries a string field from a JSON object.
std::string
loadField(const json::value& obj, const std::string& path, const std::string& alt)
{
  try
  {
    return jsonString(obj, path);
  }
  catch (...) {}

  return alt;
}


/// queries a boolean field from a JSON object.
bool
loadField(const json::value& obj, const std::string& path, bool alt)
{
  try
  {
    const json::value& val = jsonElement(obj, path);

    if (const bool* bp = val.if_bool())
      return *bp;

    if (const std::int64_t* ip = val.if_int64())
      return *ip;

    if (const std::uint64_t* up = val.if_uint64())
      return *up;
  }
  catch (...) {}

  return alt;
}

/// queries an int64_t field from a JSON object.
std::int64_t
loadField(const json::value& obj, const std::string& path, std::int64_t alt)
{
  try
  {
    const json::value& val = jsonElement(obj, path);

    if (const std::int64_t* ip = val.if_int64())
      return *ip;

    if (const std::uint64_t* up = val.if_uint64())
    {
      if (*up < std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        throw std::runtime_error("uint64_t value exceeds int64_t range.");

      return *up;
    }

    if (const bool* bp = val.if_bool())
      return *bp;
  }
  catch (...) {}

  return alt;
}

boost::json::value
loadField(const json::value& obj, const std::string& path, boost::json::value alt)
{
  try
  {
    return jsonElement(obj, path);
  }
  catch (...) {}

  return alt;
}

}

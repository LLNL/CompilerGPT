// LLMTools library
//   - common API for interacting with large language models
//   - tools for prompt generation, such as embedding and extracting
//     source code.
//
// Copyright (c) 2025, Lawrence Livermore National Security, LLC.
// All rights reserved.  LLNL-CODE-2001821
//
// License: SPDX BSD 3-Clause "New" or "Revised" License
//          see LICENSE file for details
//
// Authors: pirkelbauer2,liao6 (at) llnl.gov

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

#include <boost/json.hpp>

namespace llmtools
{
  using LLMProvider = std::string;

  extern const char* const LLMnone;
  extern const char* const LLMerror;

  /// LLM settings
  using SettingsBase = std::tuple< std::string, std::string, std::string, std::string
                                 , std::string, std::string, std::string, std::string
                                 , std::string, boost::json::value
                                 >;
  struct Settings : SettingsBase
  {
    using base = SettingsBase;
    using base::base;

    Settings( std::string pexec             = "/path/to/ai/script",
              std::string pexecFlags        = "${LLMTOOLS:MODEL}",
              std::string presponseFile     = "response.txt",
              std::string presponseField    = {},
              std::string proleOfAI         = "assistant",
              std::string psystemTextFile   = {},
              std::string phistoryFile      = "query.json",
              std::string papiKeyName       = {},
              std::string pmodelName        = {},
              boost::json::value promptFile = nullptr
            )
    : base( std::move(pexec), std::move(pexecFlags), std::move(presponseFile),
            std::move(presponseField), std::move(proleOfAI), std::move(psystemTextFile),
            std::move(phistoryFile), std::move(papiKeyName), std::move(pmodelName),
            std::move(promptFile)
          )
    {}

    std::string const& exec()       const        { return std::get<0>(*this); }
    std::string&       exec()                    { return std::get<0>(*this); }
    std::string const& execFlags() const         { return std::get<1>(*this); }
    std::string&       execFlags()               { return std::get<1>(*this); }
    std::string const& responseFile() const      { return std::get<2>(*this); }
    std::string&       responseFile()            { return std::get<2>(*this); }
    std::string const& responseField() const     { return std::get<3>(*this); }
    std::string&       responseField()           { return std::get<3>(*this); }
    std::string const& roleOfAI() const          { return std::get<4>(*this); }
    std::string&       roleOfAI()                { return std::get<4>(*this); }
    std::string const& systemTextFile() const    { return std::get<5>(*this); }
    std::string&       systemTextFile()          { return std::get<5>(*this); }
    std::string const& historyFile() const       { return std::get<6>(*this); }
    std::string&       historyFile()             { return std::get<6>(*this); }
    std::string const& apiKeyName() const        { return std::get<7>(*this); }
    std::string&       apiKeyName()              { return std::get<7>(*this); }
    std::string const& modelName() const         { return std::get<8>(*this); }
    std::string&       modelName()               { return std::get<8>(*this); }
    boost::json::value const& promptFile() const { return std::get<9>(*this); }
    boost::json::value&       promptFile()       { return std::get<9>(*this); }
  };

  using Configurations = boost::json::value;

  /// loads Settings from a JSON object \p config. If a key is not present, the
  ///   the value from \p oldSettings will be used.
  Settings
  settings(const boost::json::value& config, const Settings& oldSettings);

  /// Returns default configurations appended to \p cnf
  /// \throws std::runtime_error if a provider is defined multiple times in the configuration files
  Configurations
  initializeWithDefault(Configurations cnf = boost::json::object{});

  /// Load user-defined configurations from \p configFileName and append to \p cnf.
  /// \throws std::runtime_error if a provider is defined multiple times in the configuration files
  Configurations
  initializeWithConfigFile(const std::string& configFileName, Configurations cnf = boost::json::object{});

  /// returns the canonical key for a provider available in the configurations.
  /// \param  providerName a common name that identifies the provider
  /// \result the canonical name of the provider within the available configurations
  /// \throws std::runtime_error if the provider cannot be identified in the configurations
  /// \throws std::runtime_error if the provider requires an API key and the key is undefined
  /// \note
  ///    calling provider is optional, but ensures that subsequent calls do not fail due to
  ///    a provider being missing.
  LLMProvider
  provider(const Configurations& cnf, const std::string& providerName);

  /// stores the \p conversationHistory into the \p historyfile
  void
  storeQuery(const std::string& historyfile, const boost::json::value& conversationHistory);

  /// stores the \p conversationHistory into the historyfile as specified in \p settings
  void
  storeQuery(const Settings& settings, const boost::json::value& conversationHistory);

  /// reads a JSON stream and returns the corresponding JSON object
  boost::json::value
  readJsonStream(std::istream& is);

  /// reads a JSON file and returns the corresponding JSON object
  boost::json::value
  readJsonFile(const std::string& fileName);

  /// Writes out settings to JSON format;
  struct SettingsJsonFieldWriter
  {
    const Settings& settings;
    std::size_t     indent = 0; /// number of blanks to indent each key,value pair
  };

  /// writes out the settings as JSON key,value pairs in a JSON object.
  /// \param os output stream
  /// \param wr the writer
  std::ostream&
  operator<<(std::ostream& os, SettingsJsonFieldWriter wr);


  /// returns a new configuration for known LLM providers.
  /// \param provider the LLM provider
  /// \param llmmodel the specific LLM model to be used. If empty, the default model is used.
  /// \throws std::runtime_error if the provider cannot be identified in the configurations
  Settings
  configure(const Configurations& configs, const std::string& provider = "openai", const std::string& llmmodel = {});

  /// returns the default model for a \p provider.
  std::string
  defaultModel(const Configurations& configs, const LLMProvider& provider);

  //
  // manipulate conversation history

  /// creates a new conversation history
  /// \note CURRENTLY THIS IS LIMITED TO ONE CONCURRENT CONVERSATION HISTORY
  boost::json::value
  createConversationHistory(const Settings& settings, const std::string& systemText);

  /// append \p prompt to conversation history \p conversationHistory and return new conversation history
  boost::json::value
  appendPrompt(boost::json::value conversationHistory, const std::string& prompt);

  /// invokes the AI with the complete history in \p conversationHistory
  boost::json::value
  queryResponse(const Settings& settings, boost::json::value conversationHistory);

  /// gets the last response from the \p conversationHistory
  std::string
  lastEntry(const boost::json::value& conversationHistory);

  //
  // support text manipulation for prompt.

  /// a map from variable-names to text
  using VariableMap = std::unordered_map<std::string, std::string>;

  /// replaces variables in \p txt with values defined in \p vars.
  /// \details
  ///   Can be used to expand placeholders in prompts with defined
  ///   variables.
  std::string
  expandText(const std::string& txt, const VariableMap& vars);

  /// a source code location
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

  std::ostream&
  operator<<(std::ostream& os, SourcePoint p);

  /// a source code range
  using SourceRangeBase = std::tuple<SourcePoint, SourcePoint>;
  struct SourceRange : SourceRangeBase
  {
    using base = SourceRangeBase;
    using base::base;

    SourcePoint beg() const { return std::get<0>(*this); }
    SourcePoint lim() const { return std::get<1>(*this); }

    bool entireFile() const;

    /// returns a source range for the entire file
    static
    SourceRange all();
  };

  std::ostream&
  operator<<(std::ostream& os, SourceRange p);

  using CodeSectionBase = std::tuple<std::string, std::string>;

  struct CodeSection : CodeSectionBase
  {
    using base = CodeSectionBase;
    using base::base;

    /// the language marker; returns an empty string if it was not present
    const std::string& languageMarker() const { return std::get<0>(*this); }

    /// returns the code.
    const std::string& code()           const { return std::get<1>(*this); }
  };

  /// loads a file from an input stream \p is and replaces the section described by the source range \p sourceRange
  ///   with a the code in the code section object \p codesec and writes the result to stream \p os.
  ///   The function returns the source range of codesec in the output.
  /// \param os the output stream receiving the new code
  /// \param is the input stream containing the original code
  /// \param sourceRange the source range in the original code that needs to be replaced
  /// \param codesec the new code for the described code section.
  /// \result returns the range of \p codesec in the generated code.
  SourceRange
  replaceSourceSection(std::ostream& os, std::istream& is, SourceRange sourceRange, const CodeSection& codesec);

  /// returns a list of code sections found in \p markdownText
  /// \param  markdownText a string in markdown format that may contain code sections
  /// \result a list of code sections
  /// \details
  ///    in a markdown text, code sections appear between ``` and ```.
  ///    optionally, the begin marker specifies the language in which the
  ///    code section is in. e.g., ```cpp for C++, ```ada for Ada, or ```bash for bash scripts.
  ///    This function reads through a markdown text and generates a CodeSection object
  ///    for each section found.
  std::vector<CodeSection>
  extractCodeSections(const std::string& markdownText);

  /// reads file denoted by \p srcfile and extract the range described by \p rng into a markdown
  ///   codeblock. The codeblock uses \p langmarker as the language marker.
  std::string
  fileToMarkdown(const std::string& langmarker, const std::string& srcfile, SourceRange rng);

  /// reads stream \p srcstream and extract the range described by \p rng into a markdown
  ///   codeblock. The codeblock uses \p langmarker as the language marker.
  std::string
  fileToMarkdown(const std::string& langmarker, std::istream& srcstream, SourceRange rng);

  /// prints the code from \p code to the stream \p os while unescaping
  ///   escaped characters.
  /// \return the number of lines printed.
  std::size_t
  printUnescaped(std::ostream& os, const std::string& srccode);

  //
  // JSON tools

  /// queries a field of a certain type from a JSON object \p obj using \p path as access path.
  /// if the path cannot be found in \p obj then the alternative value \p alt is returned.
  /// \{
  std::string
  loadField(const boost::json::value& obj, const std::string& path, const std::string& alt);

  bool
  loadField(const boost::json::value& obj, const std::string& path, bool alt);

  std::int64_t
  loadField(const boost::json::value& obj, const std::string& path, std::int64_t alt);

  boost::json::value
  loadField(const boost::json::value& obj, const std::string& path, boost::json::value alt);
  /// \}
}

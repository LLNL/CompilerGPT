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

#include <boost/json.hpp>

namespace llmtools
{
  /// a list of supported LLM providers
  enum LLMProvider
  {
    openai=0,        // openai web interface
    claude=1,        // anthropic web interface
    ollama=2,        // ollama localhost web interface
    openrouter=3,    // openrouter web interface
    llamaCli=4,      // local llama-cli interface
    LLMnone=5,       // user defines settings
    LLMerror=6       // error flag
  };

  /// LLM settings
  struct Settings
  {
    std::string  invokeai       = "/path/to/ai/invocation";
    std::string  invokeparams   = "";
    std::string  responseFile   = "response.txt";
    std::string  responseField  = "";
    std::string  roleOfAI       = "assistant";  // can be overriden after configure
    std::string  systemTextFile = "";
    std::string  historyFile    = "query.json"; // can be overriden after configure
  };

  LLMProvider provider(const std::string& providerName);

  /// stores the \p conversationHistory into the \p historyfile
  void storeQuery(const std::string& historyfile, const boost::json::value& conversationHistory);

  /// stores the \p conversationHistory into the historyfile as specified in \p settings
  void storeQuery(const Settings& settings, const boost::json::value& conversationHistory);

  /// reads a JSON stream and returns the corresponding JSON object
  boost::json::value
  readJsonStream(std::istream& is);

  /// reads a JSON file and returns the corresponding JSON object
  boost::json::value
  readJsonFile(const std::string& fileName);

  /// returns a new configuration for known LLM providers.
  /// \param pathToLLMToolsDir the base path of the library installation
  ///        the bash scripts for calling curl are expected to be found at:
  ///        pathToLLMToolsDir/scripts (e.g., basePath/scripts/claude/exec-claude.sh)
  /// \param provider the LLM provider
  /// \param llmmodel the specific LLM model to be used. If empty, the default model is used.
  Settings configure(const std::string& pathToLLMToolsDir, LLMProvider provider, const std::string& llmmodel);

  /// returns the default model for a \p provider.
  std::string defaultModel(LLMProvider provider);

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

  /// replaces variables in \p rawprompt with values defined in \p vars.
  std::string
  expandPrompt(const std::string& rawprompt, const VariableMap& vars);

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
}

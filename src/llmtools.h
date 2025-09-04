#include <string>
#include <fstream>
#include <boost/json.hpp>

namespace llmtools
{
  /// a list of supported LLM providers
  enum LLMProvider
  {
    openai=0,     // openai web interface
    claude=1,     // anthropic web interface
    ollama=2,     // ollama localhost web interface
    openrouter=3, // openrouter web interface
    llamaCli=4,   // local llama-cli interface
    LLMnone=5,
    LLMerror=6
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

  /// stores the \p conversationHistory into the historyfile
  void storeQuery(const std::string& historyfile, const boost::json::value& conversationHistory);

  /// stores the \p conversationHistory into the historyfile as specified in \p settings
  void storeQuery(const Settings& settings, const boost::json::value& conversationHistory);

  /// reads a JSON file and returns the corresponding JSON object
  boost::json::value
  readJsonStream(std::istream& is);

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
}

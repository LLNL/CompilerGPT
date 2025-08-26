

#include <string>
#include <fstream>
#include <filesystem>
#include <boost/json.hpp>

namespace llmtools
{
  /// a list of supported LLM providers
  enum LLMProvider { openai=0, claude=1, ollama=2, openrouter=3, LLMnone=4, LLMerror=5 };
  
  /// LLM settings
  struct Settings
  {
    std::string  invokeai       = "/path/to/ai/invocation";
    std::string  responseFile   = "response.txt";
    std::string  responseField  = "";
    std::string  roleOfAI       = "assistant";  // can be overriden after configure
    std::string  systemTextFile = "";
    std::string  historyFile    = "query.json"; // can be overriden after configure
  };
  
  
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
  /// \note CURRENTLY THIS IS LIIMTED TO ONE CONCURRENT CONVERSATION HISTORY
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

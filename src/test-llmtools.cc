
#include <iostream>
#include <boost/json.hpp>

#include "llmtools.h"


int main()
{
  // \TODO SET THE PATH TO THE CORRECT PATH of the LLMTOOLS location.
  std::string LLMTOOLS_PATH = "/home/userid/llmtools/";
  
  std::cout << "The default model for OpenAI is " 
            << llmtools::defaultModel(llmtools::openai)
            << std::endl;
  
  llmtools::Settings config = llmtools::configure(LLMTOOLS_PATH, llmtools::openai, {} /*default model*/);
  boost::json::value conversationHistory = llmtools::createConversationHistory(config, "You are an expert programmer and skilled in C++ program optimization");
  
  std::string        prompt = "Optimize the following code snippet: ```cpp x = x + x + x + x;''' where x is of type int.";
  
  conversationHistory = llmtools::queryResponse(config, conversationHistory);
  
  std::string        fullResponse = llmtools::lastEntry(conversationHistory);
  
  std::cout << "\nAI response:\n" 
            << fullResponse 
            << std::endl;
  return 0;
}





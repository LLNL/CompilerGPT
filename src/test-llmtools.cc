
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <boost/json.hpp>
#include <boost/program_options.hpp>

#include "llmtools.h"

namespace po = boost::program_options;

// Global log file stream
std::ofstream logFile;
bool loggingEnabled = false;

// Function to log messages with timestamp
void log(const std::string& message) {
    if (!loggingEnabled) return;
    
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    logFile << "[" << std::ctime(&now_time) << "] " << message << std::endl;
    logFile.flush();
}

int main(int argc, char* argv[])
{
    // Parse command line options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("log", po::value<std::string>(), "enable logging to specified file")
        ("model", po::value<std::string>(), "specify the model to use (default: gpt-4o)");
    
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    
    // Setup logging if enabled
    if (vm.count("log")) {
        std::string logFilePath = vm["log"].as<std::string>();
        logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Error: Could not open log file: " << logFilePath << std::endl;
            return 1;
        }
        loggingEnabled = true;
        log("Logging started");
    }
    
    // Get the path from the LLMTOOLS_PATH environment variable
    const char* env_path = std::getenv("LLMTOOLS_PATH");
    if (!env_path) {
        std::cerr << "Error: LLMTOOLS_PATH environment variable not set.\n"
                << "Please set it to the directory containing the CompilerGPT repository.\n"
                << "Example: export LLMTOOLS_PATH=/path/to/CompilerGPT/\n";
        return 1;
    }
    std::string LLMTOOLS_PATH = env_path;
    log("LLMTOOLS_PATH: " + LLMTOOLS_PATH);
    
    // Get the model to use
    std::string model = "";
    if (vm.count("model")) {
        model = vm["model"].as<std::string>();
        log("Using specified model: " + model);
    }
    
    try {
        std::cout << "The default model for OpenAI is " 
                << llmtools::defaultModel(llmtools::openai)
                << std::endl;
        log("Default model for OpenAI: " + llmtools::defaultModel(llmtools::openai));
        
        log("Configuring llmtools with path: " + LLMTOOLS_PATH);
        llmtools::Settings config = llmtools::configure(LLMTOOLS_PATH, llmtools::openai, model);
        log("Configuration complete");
        
        std::string systemPrompt = "You are an expert programmer and skilled in C++ program optimization";
        log("System prompt: " + systemPrompt);
        
        log("Creating conversation history");
        boost::json::value conversationHistory = llmtools::createConversationHistory(config, systemPrompt);
        log("Conversation history created");
        
        std::string prompt = "Optimize the following code snippet: ```cpp x = x + x + x + x;''' where x is of type int.";
        log("User prompt: " + prompt);
        
        log("Appending prompt to conversation history");
        conversationHistory = llmtools::appendPrompt(conversationHistory, prompt);
        log("Prompt appended");
        
        log("Querying LLM for response");
        conversationHistory = llmtools::queryResponse(config, conversationHistory);
        log("Response received");
        
        std::string fullResponse = llmtools::lastEntry(conversationHistory);
        log("AI response: " + fullResponse);
        
        std::cout << "\nAI response:\n" 
                << fullResponse 
                << std::endl;
                
        if (loggingEnabled) {
            log("Execution completed successfully");
            logFile.close();
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (loggingEnabled) {
            log("Exception caught: " + std::string(e.what()));
            logFile.close();
        }
        return 1;
    }
}


#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <boost/json.hpp>
#include <boost/program_options.hpp>

#include "llmtools.hpp"

namespace po = boost::program_options;

// Global log file stream
std::ostream* logFile = nullptr;

// Function to log messages with timestamp
void log(const std::string& message) {
    if (logFile == nullptr) return;

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // In your logging function:
    //~ auto now = std::chrono::system_clock::now();
    //~ std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm time_info = *std::localtime(&now_time);

    (*logFile) << "[" << std::put_time(&time_info, "%c") << "] " << message
               << std::endl;
}

int main(int argc, char* argv[])
{
    llmtools::Configurations cnf = llmtools::initializeWithDefault();

    // Parse command line options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("log", po::value<std::string>(), "enable logging to specified file")
        ("provider", po::value<std::string>(), "specify the provider to use (default: openai)")
        ("model", po::value<std::string>(), "specify the model to use (default: gpt-4o)")
        ("config", po::value<std::string>(), "specifies a user-defined config file to replace default configurations (default: none)")
        ;

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

    std::unique_ptr<std::ofstream> logFileMgr = nullptr;

    // Setup logging if enabled
    if (vm.count("log")) {
        std::string logFilePath = vm["log"].as<std::string>();

        if (logFilePath == "stdout")
        {
          logFile = &std::cout;
        }
        else if (logFilePath == "stdout")
        {
          logFile = &std::cerr;
        }
        else
        {
          logFileMgr = std::make_unique<std::ofstream>(logFilePath, std::ios::out | std::ios::app);
          if (!logFileMgr->is_open()) {
              std::cerr << "Error: Could not open log file: " << logFilePath << std::endl;
              exit(1);
          }

          logFile = logFileMgr.get();
        }

        log("Logging started");
    }

    // Get the provider/model to use
    std::string provider = "openai";
    if (vm.count("provider")) {
        provider = vm["provider"].as<std::string>();
        log("Using specified provider: " + provider);
    }

    if (vm.count("config")) {
        std::string configFile = vm["config"].as<std::string>();
        log("Using user-defined config file: " + configFile);

        cnf = llmtools::initializeWithConfigFile(configFile);
    }

    llmtools::LLMProvider llmprovider = llmtools::provider(cnf, provider);
    if (llmprovider == llmtools::LLMerror || llmprovider == llmtools::LLMnone)
    {
      log("Unknown provider" + provider);
      exit(1);
    }

    log("Default model for " + provider + ": " + llmtools::defaultModel(cnf, llmprovider));

    std::string model = "";
    if (vm.count("model")) {
        model = vm["model"].as<std::string>();
        log("Using specified model: " + model);
    }

    try {
        log("Configuring llmtools");
        llmtools::Settings const config = llmtools::configure(cnf, llmprovider, model);
        log("Configuration complete");

        std::string systemPrompt = "You are an expert programmer and skilled in C++ program optimization";
        log("System prompt: " + systemPrompt);

        log("Creating conversation history");
        llmtools::ConversationHistory conversationHistory(config, systemPrompt);
        log("Conversation history created");

        std::string prompt = "Optimize the following code snippet: \n```cpp\n x = x + x + x + x;\n```\nwhere x is of type int.";
        log("User prompt: " + prompt);

        log("Appending prompt to conversation history");
        conversationHistory.appendPrompt(prompt);
        log("Prompt appended");

        log("Querying LLM for response");
        conversationHistory = llmtools::queryResponse(config, conversationHistory);
        log("Response received");

        std::string fullResponse = conversationHistory.lastEntry();
        log("AI response: " + fullResponse);

        std::cout << "\nAI response:\n"
                << fullResponse
                << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        log("Exception caught: " + std::string(e.what()));
        exit(1);
    }

    return 0;
}

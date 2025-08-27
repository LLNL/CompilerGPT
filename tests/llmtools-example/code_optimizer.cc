#include <iostream>
#include <string>
#include "llmtools.h"

int main() {
    // Set up environment
    const char* env_path = std::getenv("LLMTOOLS_PATH");
    if (!env_path) {
        std::cerr << "Error: LLMTOOLS_PATH environment variable not set.\n"
                  << "Please set it to the directory containing the CompilerGPT repository.\n"
                  << "Example: export LLMTOOLS_PATH=/path/to/CompilerGPT/\n";
        return 1;
    }
    std::string LLMTOOLS_PATH = env_path;
    
    // Configure for OpenAI
    llmtools::Settings config = llmtools::configure(LLMTOOLS_PATH, llmtools::openai, "gpt-4o");
    
    // Create conversation with system prompt
    boost::json::value conversation = llmtools::createConversationHistory(
        config,
        "You are an expert C++ programmer specializing in optimizing code for performance."
    );
    
    // Code to optimize
    std::string code = R"(
    void matrix_multiply(double* A, double* B, double* C, int n) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                C[i*n + j] = 0;
                for (int k = 0; k < n; k++) {
                    C[i*n + j] += A[i*n + k] * B[k*n + j];
                }
            }
        }
    }
    )";
    
    // Create prompt
    std::string prompt = "Please optimize this matrix multiplication function for better cache locality:\n\n```cpp\n" + code + "\n```";
    
    // Add prompt to conversation
    conversation = llmtools::appendPrompt(conversation, prompt);
    
    // Query LLM
    std::cout << "Querying LLM for optimization advice...\n";
    conversation = llmtools::queryResponse(config, conversation);
    
    // Get response
    std::string response = llmtools::lastEntry(conversation);
    
    // Display result
    std::cout << "\nOptimized code suggestion:\n" << response << std::endl;
    
    return 0;
}

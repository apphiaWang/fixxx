/* 
ls - print content of directory
*/
#include <filesystem>
#include <iostream>

using namespace std;

// Helper funtions
std::string decryptFilename(const std::string& filename, const std::string& user) {
    // Placeholder for actual decryption logic
    return filename;
}

std::string userOfPath(const std::string path) {
    // Placeholder for actual user extraction logic
    return "user";
}

auto currentPath = std::filesystem::current_path() / "filesystem/";
auto userRootPath = std::filesystem::current_path() / "filesystem/";

void ls()
{
    std::cout << "d -> ." << std::endl;
    
    // Print parent directory if not at user's root
    if (currentPath != userRootPath) {
    std::cout << "d -> .." << std::endl;
    }

    for (const auto& item : std::filesystem::directory_iterator(currentPath))
    {
        std::string file = item.path().filename();
        std::string decriptedFile = decryptFilename(file, userOfPath(currentPath));
        // if decrypted file is not empty
        if(decriptedFile.size() > 0)
            {
                if (item.is_directory()) {
                    std::cout << "d -> ";
                } else {
                    std::cout << "f -> ";
                }
                // Print the decrypted filename
                std::cout << decriptedFile << std::endl;
            }
        else 
        {
            if (item.is_directory()) {
                std::cout << "(unencrypted) d -> " << file << std::endl;
            } else {
               std::cout << "(unencrypted) f ->" << file << std::endl;
                
            }
        }

    }
}
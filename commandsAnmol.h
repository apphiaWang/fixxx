#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <exception>

// Splits a string by a delimiter and returns a vector of tokens
std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

// // Dummy implementation of encryption/decryption. Replace with actual logic.
// std::string encryptFilename(const std::string& filename, const std::string& key) {
//     // Dummy encryption logic
//     return "encrypted_" + filename;
// }

// std::string decryptFilename(const std::string& filename, const std::string& key) {
//     // Dummy decryption logic
//     return filename.substr(10); // Assuming 'encrypted_' prefix is removed
// }

void mkdir(const std::string& dirname, const std::string& currentPath, const std::string& adminName) {
    auto pathTokens = split(currentPath, "filesystem/");
    if (pathTokens.size() < 2) {
        std::cout << "Can't create dir here" << std::endl;
        return;
    }
    
    auto relpath = split(pathTokens[1], '/');
    if (relpath.size() < 2 || relpath[0].empty() || relpath[1].empty()) {
        std::cout << "Can't create dir here" << std::endl;
        return;
    }
    
    auto user = decryptFilename(relpath[0], adminName);
    auto share = decryptFilename(relpath[1], user);
    if (user.empty() || share.empty() || share == "shared") {
        std::cout << "Can't create dir here" << std::endl;
        return;
    }

    std::string dirname_enc;
    try {
        dirname_enc = encryptFilename(dirname, user);
    } catch (const std::exception& e) {
        std::cout << "mkdir failed. Exception in encrypt: " << e.what() << std::endl;
        return;
    }

    std::filesystem::path new_dir = std::filesystem::path(currentPath) / dirname_enc;
    if (std::filesystem::exists(new_dir)) {
        std::cout << "Name already exists" << std::endl;
    } else {
        try {
            std::filesystem::create_directory(new_dir);
        } catch (const std::exception& e) {
            std::cout << "mkdir failed. Exception: " << e.what() << std::endl;
        }
    }
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>



// // Stub for encryptFilename function
// std::string encryptFilename(const std::string& filename, const std::string& user) {
//     // Implement filename encryption logic based on the user
//     return filename + "_encrypted";
// }

// // Stub for decryptFilename function
// std::string decryptFilename(const std::string& filename, const std::string& user) {
//     // Implement filename decryption logic based on the user
//     return filename + "_decrypted";
// }

// // Stub for decryptByMetadataPrivateKey function
// std::string decryptByMetadataPrivateKey(const std::string& data, const std::string& keyName) {
//     // Implement data decryption logic based on the keyName
//     return data; // Assuming decryption is successful and returns the original data
// }




// Assuming the adminName is globally defined as:
const std::string adminName = "admin";

// Split function as previously described
std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = s.find(delimiter, start)) != std::string::npos) {
        tokens.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

std::string userOfPath(const std::string& path) {
    // Get the owner of the path
    auto pathTokens = split(path, "filesystem/");
    if (pathTokens.size() == 1) { // The path ends with 'filesystem', only admin can reach this folder
        return adminName;
    }
    auto pathTokensInFilesystem = split(pathTokens[1], "/");
    std::string userCipher = pathTokensInFilesystem[0];
    return decryptFilename(userCipher, adminName); // Assuming decryptFilename is defined to decrypt the cipher into a username
}


// Simplified cat function
void cat(const std::string& filename) {
    std::string user = userOfPath(currentPath);
    std::string encryptedFilename = encryptFilename(filename, user);
    std::string filePath = "filesystem/" + currentPath + "/" + encryptedFilename;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << filename << " doesn't exist" << std::endl;
        return;
    }

    std::string decryptKeyName = decryptFilename(filename, user); // Assuming decryptFilename now simply gets the decryption key name
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    try {
        std::cout << decryptByMetadataPrivateKey(buffer.str(), decryptKeyName) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "cat failed. Exception in decrypt: " << e.what() << std::endl;
    }
}

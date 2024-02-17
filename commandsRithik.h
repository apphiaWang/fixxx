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

void mkfile(const std::string& filename, std::string contents) {
    // Split current path to check if operation is allowed
    auto pathTokens = split(currentPath, "filesystem/");
    if (pathTokens.size() < 2 || split(pathTokens[1], '/').size() < 2) {
        std::cout << "Can't create file here" << std::endl;
        return;
    }

    // Extract user and folder name from path
    auto relPath = split(pathTokens[1], '/');
    auto user = decryptFilename(relPath[0], adminName);
    auto folderName = decryptFilename(relPath[1], user);
    
    // Validate user and folder name
    if (user.empty() || folderName.empty() || folderName == "shared") {
        std::cout << "Can't create file here" << std::endl;
        return;
    }

    // Encrypt filename and contents
    std::string filenameEnc;
    try {
        filenameEnc = encryptFilename(filename, user);
        contents = encrypt(contents, user);
    } catch (const std::exception& e) {
        std::cout << "mkfile failed. Exception in encrypt: " << e.what() << std::endl;
        return;
    }

    // Check if filename already exists
    if (std::filesystem::exists(currentPath / filenameEnc)) {
        std::cout << "Name already exists" << std::endl;
        return;
    }

    // Create and write to file
    std::ofstream file(currentPath / filenameEnc, std::ofstream::trunc);
    if (!file.is_open()) {
        std::cout << "Failed to create file" << std::endl;
        return;
    }
    
    file << contents;
    file.close();
    std::cout << "Successfully created file" << std::endl;

    // Share file with receivers
    auto receivers = isAdmin ? getReceivers(userOfPath(currentPath), getRelativePath(currentPath / filenameEnc, encryptFilename(userOfPath(currentPath), adminName).length() + currentPathPrefixLength + 1))
                             : getReceivers(userOfPath(currentPath), getRelativePath(currentPath / filenameEnc, currentPathPrefixLength + 1));
    for (const auto& receiver : receivers) {
        share(filename, receiver);
    }
}
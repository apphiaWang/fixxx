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

// constants
auto currentPath = std::filesystem::current_path() / "filesystem/";
auto userRootPath = std::filesystem::current_path() / "filesystem/";
const std::filesystem::path ROOT_PATH = std::filesystem::current_path();


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


void share(const std::string& filename, const std::string& username)
{
    // Check if the file can be shared in the current path
    auto pathTokens = split(currentPath, "filesystem/");
    auto relpath = split(pathTokens[1], '/');
    if (pathTokens.size() < 2 || relpath.size() < 2) {
        std::cout << "Can't share file here" << std::endl;
        return;
    }

    // Decrypt user and folder names
    auto user = decryptFilename(relpath[0], adminName);
    auto folderName = decryptFilename(relpath[1], user);
    if(user.empty()) {
        std::cout << "Unexpected path" << std::endl;
        return;
    }
    if (folderName.empty() || folderName == "shared") {
        std::cout << "Can't share file here" << std::endl;
        return;
    }

    // Validate if the source file exists
    auto full_source_path = currentPath / encryptFilename(filename, userOfPath(currentPath));                          
    if (!std::filesystem::exists(full_source_path))
    {
        std::cout << "File " << filename << " doesn't exist." << std::endl;
        return;
    }

    if(!std::filesystem::is_regular_file(full_source_path))
    {
        std::cout << "File " << filename << " isn't a regular file." << std::endl;
        return;
    }
    
    // Validate if the target user exists
    auto userDirectory = ROOT_PATH / "filesystem" / encryptFilename(username, adminName);
    if (!std::filesystem::exists(userDirectory))   
    {
        std::cout << "User " << username << " doesn't exist." << std::endl;
        return;
    }

    // Read the source file
    std::ifstream source_file(full_source_path.generic_string());
    if (source_file.fail()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
    }
    std::string content;
    char ch;
    while (source_file.get(ch)) {
        content += ch;
    }

    // Decrypt and re-encrypt the file content
    auto currentPathUser = userOfPath(currentPath);
    auto decryptedContent = decryptByMetadataPrivateKey(content, currentPathUser);
    auto encryptedContent = encrypt(decryptedContent, username);

    // Define the target path and write the encrypted content
    auto full_target_path = ROOT_PATH / "filesystem" / encryptFilename(username, adminName) / encryptFilename("shared", username) / encryptFilename(currentPathUser + "_" + filename, username);
    std::ofstream ofs(full_target_path.generic_string(), std::ios::trunc);
    ofs << encryptedContent;
    if (!ofs) {
        std::cerr << "Failed to write to file: " << full_target_path << std::endl;
        return;
    }

    // Update fileShareMapping
    addFileShareMapping(userOfPath(currentPath), getRelativePath(full_source_path, currentPathPrefixLength + 1), username);
}

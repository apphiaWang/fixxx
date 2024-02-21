#ifndef COMMANDS_H
#define COMMANDS_H

#include "stringUtils.h"
#include "fileUtils.h"
#include "authentication.h"


/* session info*/
std::string currentUser;
bool isAdmin = false;


/*** constants ***/
#define FILENAME_MAX_LEN 20
// Serve for stripping / display purpose
const std::filesystem::path SYSTEM_ROOT_PATH = std::filesystem::current_path();
const std::string FILE_SYSTEM_ROOT_PATH_STR = (std::filesystem::current_path() / "filesystem/").u8string();
// current path
auto currentPath = std::filesystem::current_path() / "filesystem/";
// init() should update it to user root directory
auto userRootPath = std::filesystem::current_path() / "filesystem/";


/* 
pwd - print current path
*/
void pwd()
{
    std::cout << "/";
    if(currentPath != userRootPath) {
        // remove path before filesystem
        std::string path = isAdmin 
                            ? removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR) 
                            : removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR + encrypt_decrypt(currentUser));

        std::string userOfFolder = userOfPath(currentPath);

        // decrypt paths and print it
        auto pathToBePrintedTokens = split(path, '/');
        for (std::vector<std::string>::iterator it = pathToBePrintedTokens.begin() ; it != pathToBePrintedTokens.end(); ++it) {
            if(isAdmin && it ==  pathToBePrintedTokens.begin()){
                std::cout << encrypt_decrypt(*it) + "/";  
            }else{
                std::cout << encrypt_decrypt(*it) + "/";
            }
        }
    }
    std::cout << std::endl;
}

/*
cd - go to target directory
*/
void cd(const std::string& targetDir)
{
    std::string workPath = targetDir;
    std::filesystem::path tmpPath;

    if (workPath[0] == '/' ){
        workPath = workPath.substr(1, workPath.length() -1);
        tmpPath = userRootPath;
    }
    else
    {
        tmpPath = currentPath;
    }

    auto tokens = split(workPath, '/');
    for (std::vector<std::string>::iterator it = tokens.begin() ; it != tokens.end(); ++it) {

        std::string token = *it;
        std::string dirName;
        std::string username = userOfPath(tmpPath);

        if (token == "")
        {  // multiple '/', e.g. "///", will be considered as single '/'
            continue;
        } 
        else if(token != ".." && token != ".")
        {
            dirName = encrypt_decrypt(token);            
        }
        else
        {
            dirName = token;
        }

        std::filesystem::path newPath;

        try{
            newPath = std::filesystem::canonical(tmpPath / dirName);
        }
        catch(const std::exception& ex){
            std::cout << "Invalid file path " << workPath << std::endl;
            return;
        }

        if(!checkPathBoundary(userRootPath, newPath)) {
            std::cout << "Directory " << targetDir << " is overbound" << std::endl;
            return;
        }
        if(!std::filesystem::is_directory(newPath)) {
            std::cout << targetDir << " is not a directory" << std::endl;
            return;
        }

        tmpPath = newPath;
    }

    currentPath = tmpPath / ""; 
}

/*
mkdir - create a new directory
*/
void mkdir(const std::string& dirname)
{
    // input validation
    if (dirname.empty() || dirname.length() > FILENAME_MAX_LEN) 
    {
        std::cout << "Invaid path, please check user manual" << std::endl;
        return;
    }
    for (const char c : dirname) 
    {
        if (!isalnum(c) && c != '_' && c != '-' && c != '.' && c != '=') {
            std::cout << "Invaid path, please check user manual" << std::endl;
            return;
        }
    }
    
    // get the current location path and the target directory to create
    std::string locPath;
    auto canonicalDir = std::filesystem::weakly_canonical(currentPath / dirname);
    std::string targetDir;
    try {
        locPath = removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR);
        targetDir = removePrefix(canonicalDir.u8string(), FILE_SYSTEM_ROOT_PATH_STR);
    } catch (int error) {
        std::cout << "Can't create dir here" << std::endl;
        return;
    }
    
    // check if user is allowed to mkdir under current directory
    // user should only mkdir under filesystem/<username>/personal
    auto relpath = split(locPath,'/');
    auto userOfLoc = encrypt_decrypt(relpath[0]);
    if(userOfLoc.empty() || userOfLoc != currentUser || relpath.size() < 2) {
        std::cout << "Can't create dir here" << std::endl;
        return;
    }

    auto expectHomeDir = encrypt_decrypt(relpath[1]);
    if(expectHomeDir.empty() or expectHomeDir != "personal") {
        std::cout << "Can't create dir here" << std::endl;
        return;
    }    

    std::string dirname_enc, user_enc, share_enc;
    try {
        dirname_enc = encrypt_decrypt(dirname);
    }
    catch (const std::exception& e) {
        std::cout << "mkdir failed. Exception in encrypt: " << e.what() << std::endl;
        return;
    }

    if(std::filesystem::exists(currentPath / dirname_enc)) {
        std::cout << "Name already exists" << std::endl; 
    } else {
        try {
            auto new_dir = currentPath / dirname_enc;
            std::filesystem::create_directory(new_dir);
        }
        catch (const std::exception& e) {
            std::cout << "mkdir failed. Exception: " << e.what() << std::endl;
            return;
        }
    }
}

/*
ls - list directories and files under the current directory
*/
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
        std::string decryptedFile = encrypt_decrypt(file);
        // if decrypted file is not empty
        if(decryptedFile.size() > 0)
        {
            if (item.is_directory()) {
                std::cout << "d -> ";
            } else {
                std::cout << "f -> ";
            }
            std::cout << decryptedFile << std::endl;
        }
        // @TODO check encryption decryption with team and design the behavior
        // else if (isAdmin)
        // {   // only shows unencrypted dir/files to admin user
        //     if (item.is_directory()) {
        //         std::cout << "(unencrypted) d -> " << file << std::endl;
        //     } else {
        //        std::cout << "(unencrypted) f ->" << file << std::endl;
                
        //     }
        // }
    }
}

std::string encryptFileContent(std::string plaintext) {
    return plaintext;
}

std::string decryptFileContent(std::string cipher) {
    return cipher;
}

/*
cat - print the content of the file
*/
void cat(const std::string& filename) {
    std::string pathLoc = removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR);
    std::string finalPath = "filesystem/" + pathLoc + "/" + encrypt_decrypt(filename);

    std::ifstream file(finalPath);
    if (file.is_open())
    {
        try {
            std::stringstream buffer;
            file >> buffer.rdbuf();
            // TODO @Hugh implement file content ecryption
            std::cout << decryptFileContent(buffer.str()) << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "cat failed. Exception in decrypt: " << e.what() << std::endl;
        }
        file.close();
    }
    else
    {
        std::cout << filename << " doesn't exist" << std::endl;
    }
}


/*
mkfile - create a new text file
*/
void mkfile(const std::string& filename, std::string contents) {
    // input validation
    if (filename.empty() || filename.length() > FILENAME_MAX_LEN) 
    {
        std::cout << "Invaid file name, please check user manual" << std::endl;
        return;
    }
    for (const char c : filename) 
    {
        if (!isalnum(c) && c != '_' && c != '-' && c != '.' && c != '=') {
            std::cout << "Invaid file name, please check user manual" << std::endl;
            return;
        }
    }
    
    // get the current location path and the target file path to create
    std::string locPath;
    auto canonicalFile = std::filesystem::weakly_canonical(currentPath / filename);
    std::string filePath;
    try {
        locPath = removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR);
        filePath = removePrefix(canonicalFile.u8string(), FILE_SYSTEM_ROOT_PATH_STR);
    } catch (int error) {
        std::cout << "Can't create file here" << std::endl;
        return;
    }

    auto relPath = split(locPath, '/');
    auto user = encrypt_decrypt(relPath[0]);
    auto folderName = encrypt_decrypt(relPath[1]);
    std::cout << user << currentUser << std::endl;
    // Should only create files under filesystem/<user>/personal
    if (user.empty() || user !=  currentUser || folderName.empty() || folderName == "shared") {
        std::cout << "Can't create file here" << std::endl;
        return;
    }

    // Encrypt filename and contents
    std::string filenameEnc;
    try {
        filenameEnc = encrypt_decrypt(filename);
        contents = encryptFileContent(contents);
    } catch (const std::exception& e) {
        std::cout << "mkfile failed. Exception in encrypt: " << e.what() << std::endl;
        return;
    }

    // Check if filename already exists
    if (std::filesystem::exists(currentPath / filenameEnc)) {
        std::cout << "mkfile failed. File "<< filename << " already exists" << std::endl;
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
}

/*
adduser - create a new user
*/
void adduser(const std::string& username){

    if (!isAdmin) {
        std::cout << "Invaid command, please check user manual" << std::endl;
        return;
    }
    if (username.empty() || username.length() > FILENAME_MAX_LEN) 
    {
        std::cout << "Invaid username, please check user manual" << std::endl;
        return;
    }
    for (const char c : username) 
    {
        if (!isalnum(c) && c != '_' && c != '-' && c != '.' && c != '=') {
            std::cout << "Invaid username, please check user manual" << std::endl;
            return;
        }
    }
    generate_key_pair(username);

    // @TODO update metadata    
}

/*
share - share a file with another user
*/
void share(const std::string& filename, const std::string& username)
{
    // input validation
    if (username == currentUser) {
        std::cout << "You cannot share file with yourself" << std::endl;
        return;
    }
    if (filename.empty() || filename.length() > FILENAME_MAX_LEN) 
    {
        std::cout << "Invaid file name, please check user manual" << std::endl;
        return;
    }
    for (const char c : filename) 
    {
        if (!isalnum(c) && c != '_' && c != '-' && c != '.' && c != '=') {
            std::cout << "Invaid file name, please check user manual" << std::endl;
            return;
        }
    }

    std::string locPath;
    try {
        locPath = removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR);
    } catch (int error) {
        std::cout << "Can't share file here" << std::endl;
        return;
    }

    auto relPath = split(locPath, '/');
    auto userOfFile = encrypt_decrypt(relPath[0]);
    auto folderName = encrypt_decrypt(relPath[1]);
    // Should only create files under filesystem/<user>/personal
    if (userOfFile.empty() || userOfFile !=  currentUser || folderName.empty() || folderName == "shared") {
        std::cout << "Can't share file here" << std::endl;
        return;
    }

    auto fullFilePath = currentPath / encrypt_decrypt(filename);                          
    if (!std::filesystem::exists(fullFilePath))
    {
        std::cout << "File " << filename << " doesn't exist." << std::endl;
        return;
    }
    if(!std::filesystem::is_regular_file(fullFilePath))
    {
        std::cout << "File " << filename << " isn't a regular file." << std::endl;
        return;
    }
    
    // Check the target user 1. exists 2. is not the currentUser
    // @TODO check if the target user exists
    

    // Read the source file
    std::ifstream source_file(fullFilePath.generic_string());
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
    auto decryptedContent = decryptFileContent(content);
    auto encryptedContent = encryptFileContent(decryptedContent);

    // Define the target path and write the encrypted content
    auto full_target_path = std::filesystem::current_path() / "filesystem" / encrypt_decrypt(username) / encrypt_decrypt("shared") / encrypt_decrypt(currentPathUser + "_" + filename);
    std::ofstream ofs(full_target_path.generic_string(), std::ios::trunc);
    ofs << encryptedContent;
    if (!ofs) {
        std::cerr << "Share file failed. Failed to write to file: " << full_target_path << std::endl;
        return;
    }
    std::cout << "Successfully shared file: "<< filename << " with user: " << username << std::endl;
}

#endif // COMMANDS_H

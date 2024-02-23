#ifndef COMMANDS_H
#define COMMANDS_H

#include "stringUtils.h"
#include "fileUtils.h"
#include "authentication.h"


/* session info*/
std::string currentUser;
bool isAdmin = false;
auto currentPath = std::filesystem::current_path() / "filesystem/";
auto userRootPath = std::filesystem::current_path() / "filesystem/";


/*** constants ***/
#define FILENAME_MAX_LEN 20
#define FILECONTENT_MAX_LEN 4096
const std::filesystem::path SYSTEM_ROOT_PATH = std::filesystem::current_path();
const std::string FILE_SYSTEM_ROOT_PATH_STR = (std::filesystem::current_path() / "filesystem/").u8string();


/*
Utilility functions
*/
void enterUserHome(const std::string& username)
{
    currentPath = currentPath / encrypt_decrypt(username);
    userRootPath = userRootPath / encrypt_decrypt(username);
}

bool checkUserExist(const std::string &username) {
    return std::filesystem::exists(SYSTEM_ROOT_PATH / "filesystem" / encrypt_decrypt(username));
}

std::string userOfPath(const std::string path){
    std::string locPath = removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR);
    auto relPaths = split(locPath,'/');
    return encrypt_decrypt(relPaths[0]);
}


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
        std::cout << "mkdir failed, Invaid path, please check user manual" << std::endl;
        return;
    }
    for (const char c : dirname) 
    {
        if (!isalnum(c) && c != '_' && c != '-' && c != '.' && c != '=') {
            std::cout << "mkdir failed, Invaid path, please check user manual" << std::endl;
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
        std::cout << "mkdir failed, Can't create dir here" << std::endl;
        return;
    }
    
    // check if user is allowed to mkdir under current directory
    // user should only mkdir under filesystem/<username>/personal
    auto relpath = split(locPath,'/');
    auto userOfLoc = encrypt_decrypt(relpath[0]);
    if(userOfLoc.empty() || userOfLoc != currentUser || relpath.size() < 2) {
        std::cout << "mkdir failed, Can't create dir here" << std::endl;
        return;
    }

    auto expectHomeDir = encrypt_decrypt(relpath[1]);
    if(expectHomeDir.empty() or expectHomeDir != "personal") {
        std::cout << "mkdir failed, Can't create dir here" << std::endl;
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
        std::cout << "mkdir failed, " << dirname << " already exists" << std::endl; 
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
        // TODO hide private_keys and metadata
        if (file == "metadata" || file == "private_keys") {
            continue;
        }
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
    }
}

/*
cat - print the content of the file
*/
void cat(const std::string& filename) {
    std::string locPath = removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR);
    std::string finalPath = "filesystem/" + locPath + "/" + encrypt_decrypt(filename);

    auto relPaths = split(locPath, '/');
    if (relPaths.size() < 2) {
        std::cout << "cat failed." << std::endl;
        return;
    }
    std::string owner = encrypt_decrypt(relPaths[0]); 

    std::ifstream file(finalPath);
    if (file.is_open())
    {
        try {
            std::stringstream buffer;
            file >> buffer.rdbuf();
            std::cout << rsa_decrypt(buffer.str(), owner) << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "cat failed. Exception in decrypt: " << e.what() << std::endl;
        }
        file.close();
    }
    else
    {
        std::cout << "cat failed, " << filename << " doesn't exist" << std::endl;
    }
}

/*
adduser - create a new user
*/
void adduser(const std::string& username){

    if (!isAdmin) {
        std::cout << "adduser failed, Invaid command, please check user manual" << std::endl;
        return;
    }
    if (username.empty() || username.length() > FILENAME_MAX_LEN) 
    {
        std::cout << "adduser failed, Invaid username, please check user manual" << std::endl;
        return;
    }
    for (const char c : username) 
    {
        if (!isalnum(c) && c != '_' && c != '-' && c != '.' && c != '=') {
            std::cout << "adduser failed, Invaid username, please check user manual" << std::endl;
            return;
        }
    }
    if (checkUserExist(username)) {
        std::cout << "adduser failed, user already exists." << std::endl;
        return;
    }
    generate_key_pair(username);
}

/*
share - share a file with another user
*/
void share(const std::string& filename, const std::string& username)
{
    // input validation
    if (username == currentUser) {
        std::cout << "share failed, cannot share file with the owner" << std::endl;
        return;
    }
    if (filename.empty() || filename.length() > FILENAME_MAX_LEN) 
    {
        std::cout << "share failed, invaid file name, please check user manual" << std::endl;
        return;
    }
    for (const char c : filename) 
    {
        if (!isalnum(c) && c != '_' && c != '-' && c != '.' && c != '=') {
            std::cout << "share failed, invaid file name, please check user manual" << std::endl;
            return;
        }
    }
    std::string locPath;
    try {
        locPath = removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR);
    } catch (int error) {
        std::cout << "share failed, can't share file here" << std::endl;
        return;
    }
    std::cout << "1" << std::endl;
    auto relPath = split(locPath, '/');
    auto userOfFile = encrypt_decrypt(relPath[0]);
    auto folderName = encrypt_decrypt(relPath[1]);
    // Should only create files under filesystem/<user>/personal
    if (userOfFile.empty() || userOfFile !=  currentUser || folderName.empty() || folderName == "shared") {
        std::cout << "share failed, can't share file here" << std::endl;
        return;
    }
    std::cout << "2" << std::endl;
    
    auto fullFilePath = currentPath / encrypt_decrypt(filename);                          
    if (!std::filesystem::exists(fullFilePath))
    {
        std::cout << "share failed, file " << filename << " doesn't exist." << std::endl;
        return;
    }
    if(!std::filesystem::is_regular_file(fullFilePath))
    {
        std::cout << "share failed, file " << filename << " isn't a regular file." << std::endl;
        return;
    }
    std::cout << "3" << std::endl;
    
    // Check the target user 1. exists 2. is not the currentUser
    if (!checkUserExist(username)) {
        std::cout << "share failed. User "<< username <<" doesn't exist." << std::endl;
        return;
    }

    // Read the source file
    std::ifstream source_file(fullFilePath.generic_string());
    if (source_file.fail()) {
        std::cerr << "share failed, Failed to open file: " << filename << std::endl;
        return;
    }
    std::string content;
    char ch;
    while (source_file.get(ch)) {
        content += ch;
    }
    std::cout << "4" << std::endl;
    
    // Decrypt and re-encrypt the file content
    auto decryptedContent = rsa_decrypt(content, currentUser);
    auto encryptedContent = rsa_encrypt(decryptedContent, username);

    // Define the target path and write the encrypted content
    std::string encFileName = encrypt_decrypt(currentUser + "_" + filename);
    auto full_target_path = std::filesystem::current_path() / "filesystem" / encrypt_decrypt(username) / encrypt_decrypt("shared") / encFileName;
    std::ofstream ofs(full_target_path.generic_string(), std::ios::trunc);
    ofs << encryptedContent;
    if (!ofs) {
        std::cerr << "share failed, failed to write to file: " << full_target_path << std::endl;
        return;
    }

    addFileShareMapping(encrypt_decrypt(currentUser), encFileName, encrypt_decrypt(username));
}

/*
mkfile - create a new text file
*/
void mkfile(const std::string& filename, std::string content) {
    // input validation
    if (content.empty()) {
        std::cout << "mkfile failed, file content cannot be empty" << std::endl;
        return;
    }
    if (filename.empty() || filename.length() > FILENAME_MAX_LEN) 
    {
        std::cout << "mkfile failed, Invaid file name, please check user manual" << std::endl;
        return;
    }
    for (const char c : filename) 
    {
        if (!isalnum(c) && c != '_' && c != '-' && c != '.') {
            std::cout << "mkfile failed, Invaid file name, please check user manual" << std::endl;
            return;
        }
    }
    if (content.length() > FILECONTENT_MAX_LEN) {
        std::cout << "mkfile failed, content too long, file content should not exceed " << FILECONTENT_MAX_LEN << " bytes." << std::endl;
        return;
    }
    
    // get the current location path and the target file path to create
    std::string locPath;
    auto canonicalFile = std::filesystem::weakly_canonical(currentPath / filename);
    std::string filePath;
    try {
        locPath = removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR);
        filePath = removePrefix(canonicalFile.u8string(), FILE_SYSTEM_ROOT_PATH_STR);
    } catch (int error) {
        std::cout << "mkfile failed, can't create file here" << std::endl;
        return;
    }

    // Should only create files under filesystem/<user>/personal
    auto relPath = split(locPath, '/');
    if (relPath.size() < 2) {
        std::cout << "mkfile failed, can't create file here" << std::endl;
        return;
    }
    auto user = encrypt_decrypt(relPath[0]);
    auto folderName = encrypt_decrypt(relPath[1]);
    if (user.empty() || user !=  currentUser || folderName.empty() || folderName == "shared") {
        std::cout << "mkfile failed, can't create file here" << std::endl;
        return;
    }

    std::cout << "1" << std::endl;
    // Encrypt filename and content
    std::string filenameEnc;
    std::string cypher;
    try {
        filenameEnc = encrypt_decrypt(filename);
        cypher = rsa_encrypt(content, currentUser);
    } catch (const std::exception& e) {
        std::cout << "mkfile failed, Exception in encrypt: " << e.what() << std::endl;
        return;
    }
    
    // Create and write to file
    std::ofstream file(currentPath / filenameEnc, std::ofstream::trunc);
    if (!file.is_open()) {
        std::cout << "mkfile failed, failed to create file" << std::endl;
        return;
    }
    file << cypher;
    file.close();

    // reshare file
    std::vector<std::string> receivers;
    receivers = getReceivers(encrypt_decrypt(currentUser), encrypt_decrypt(currentUser + "_" + filename));
    for (auto receiver : receivers) {
        share(filename, encrypt_decrypt(receiver));
    }
}

#endif // COMMANDS_H

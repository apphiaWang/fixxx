#ifndef COMMANDS_H
#define COMMANDS_H

#include "stringUtils.h"
#include "fileUtils.h"
#include "cryptoUtills.h"

/* session info*/
std::string currentUser;
std::string adminName = "admin";
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
                            : removePrefix(currentPath, FILE_SYSTEM_ROOT_PATH_STR + encryptFilename(currentUser, adminName));

        std::string userOfFolder = userOfPath(currentPath);

        // decrypt paths and print it
        auto pathToBePrintedTokens = split(path, '/');
        for (std::vector<std::string>::iterator it = pathToBePrintedTokens.begin() ; it != pathToBePrintedTokens.end(); ++it) {
            if(isAdmin && it ==  pathToBePrintedTokens.begin()){
                std::cout << decryptFilename(*it, adminName) + "/";  
            }else{
                std::cout << decryptFilename(*it, userOfFolder) + "/";
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
            dirName = encryptFilename(token, username);            
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
    auto userOfLoc = decryptFilename(relpath[0], adminName);
    if(userOfLoc.empty()) {
        std::cout << "An error occurs, please contact admin" << std::endl;
        return;
    }
    if(relpath.size() < 2) {
        std::cout << "Can't create dir here" << std::endl;
        return;
    }
    auto expectHomeDir = decryptFilename(relpath[1], userOfLoc);
    if(expectHomeDir.empty() or expectHomeDir != "personal") {
        std::cout << "Can't create dir here" << std::endl;
        return;
    }    

    std::string dirname_enc, user_enc, share_enc;
    try {
        dirname_enc = encryptFilename(dirname, userOfLoc);
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
        std::string decryptedFile = decryptFilename(file, userOfPath(currentPath));
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
        else if (isAdmin)
        {   // only shows unencrypted dir/files to admin user
            if (item.is_directory()) {
                std::cout << "(unencrypted) d -> " << file << std::endl;
            } else {
               std::cout << "(unencrypted) f ->" << file << std::endl;
                
            }
        }
    }
}


#endif // COMMANDS_H
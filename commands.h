#ifndef COMMANDS_H
#define COMMANDS_H

#include "stringUtils.h"
#include "fileUtils.h"

/* session info*/
std::string currentUser;
std::string adminName = "admin";
bool isAdmin = false;


/*** constants ***/
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


#endif // COMMANDS_H
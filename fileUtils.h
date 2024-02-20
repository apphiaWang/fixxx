#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <vector>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm> 
#include <filesystem>

/*
Check if filesystem already initialized
*/
bool fileSystemExist() {
    // TODO @team
    return true;
}

/*
Init root folder filesystem for the encrypted file system
*/
void initSystemRoot() {
    // TODO @team store the metadata
    // TODO @team error handle
    std::filesystem::create_directory("filesystem");
}

/*
Find the owner of the current path for decryption
*/
std::string userOfPath(const std::string path){
    // @TODO @team
    return "admin";
}

/*
Encrypt file name
*/
std::string encryptFilename(const std::string &filename, const std::string &username) {
    // TODO @team encryption
    return filename;
}

/*
Decrypt file name
*/
std::string decryptFilename(const std::string &cipher, const std::string &username) {
    // TODO @team decrypt file name
    return cipher;
}

/*
Check if user has access of the target directory
*/
bool checkPathBoundary(const std::filesystem::path &root, const std::filesystem::path &child)
{
    auto const canonicalRootPath = std::filesystem::canonical(root);
    auto const canonicalChildPath = std::filesystem::canonical(child);
    auto itr = std::search(canonicalChildPath.begin(), canonicalChildPath.end(), 
                           canonicalRootPath.begin(), canonicalRootPath.end());
    
    return itr == canonicalChildPath.begin();
}

#endif // FILEUTILS_H
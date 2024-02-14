#ifndef SYSTEMUTILS_H
#define SYSTEMUTILS_H

#include <vector>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

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

#endif // SYSTEMUTILS_H
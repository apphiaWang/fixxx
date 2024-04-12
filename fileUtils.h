#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm> 
#include <filesystem>

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

/*
Get suffix number of the share file
*/
int getFileSuffixNumber(const std::string &sender,
                         const std::string &filename,
                         const std::string &filepath,
                         const std::string &receiver)
{
    std::ifstream ifile("filesystem/.metadata/fileShareMapping.txt");
    if (!ifile.is_open())
    {
        std::cerr << "Error: could not open fileShareMapping" << std::endl;
    }

    std::string line;
    int max_existing_suffix = -1;

    while (std::getline(ifile, line))
    {
        std::istringstream iss(line);
        std::string senderCol, filenameCol, filePathCol, receiverCol, suffixCol;
        if (std::getline(iss, senderCol, ',') && std::getline(iss, filenameCol, ',') 
            &&  std::getline(iss, filePathCol, ',') && std::getline(iss, receiverCol, ',')
            && std::getline(iss, suffixCol, ','))
        {
            if (sender == senderCol && filename == filenameCol 
                && filepath == filePathCol && receiver == receiverCol)
            {
                return  std::stoi(suffixCol);
            }

            if (sender == senderCol && filename == filenameCol && receiver == receiverCol)
            {
                int suffix = std::stoi(suffixCol);
                if (suffix > max_existing_suffix) {
                    max_existing_suffix = suffix;
                }
            }
        }
    }
    ifile.close();

    return max_existing_suffix + 1;
}

/*
Update file share mapping in metadata folder
*/
void addFileShareMapping(const std::string &sender,
                         const std::string &filename,
                         const std::string &filepath,
                         const std::string &receiver,
                         const int &suffix)
{
    // TODO refactor
    std::ifstream ifile("filesystem/.metadata/fileShareMapping.txt");
    if (!ifile.is_open())
    {
        std::cerr << "Error: could not open fileShareMapping" << std::endl;
    }

    // check if the pairing has been added
    std::string line;
    while (std::getline(ifile, line))
    {
        std::istringstream iss(line);
        std::string senderCol, filenameCol, filePathCol, receiverCol, suffixCol;
        if (std::getline(iss, senderCol, ',') && std::getline(iss, filenameCol, ',') 
            &&  std::getline(iss, filePathCol, ',') && std::getline(iss, receiverCol, ',')
            && std::getline(iss, suffixCol, ','))
        {
            if (sender == senderCol && filename == filenameCol 
                && filepath == filePathCol && receiver == receiverCol)
            {
                return;
            }
        }
    }
    ifile.close();

    std::ofstream ofile;
    ofile.open("filesystem/.metadata/fileShareMapping.txt", std::ios_base::app);
    if (!ofile.is_open())
    {
        std::cerr << "Error: could not open fileShareMapping" << std::endl;
        return;
    }

    ofile << sender << "," << filename << "," << filepath << "," << receiver << "," << suffix << std::endl;
    ofile.close();
}

/*
Get shared receivers of a file
*/
std::vector<std::string> getReceivers(const std::string &sender,
                                      const std::string &filename,
                                      const std::string &filepath)
{
    std::vector<std::string> receivers;

    std::ifstream ifile("filesystem/.metadata/fileShareMapping.txt");
    if (!ifile.is_open())
    {
        std::cerr << "Error: could not open fileShareMapping" << std::endl;
        return receivers;
    }

    std::string line;
    while (std::getline(ifile, line))
    {
        std::istringstream iss(line);
        std::string senderCol, filenameCol, filepathCol, receiverCol;
        if (std::getline(iss, senderCol, ',') && std::getline(iss, filenameCol, ',')
            && std::getline(iss, filepathCol, ',') && std::getline(iss, receiverCol, ','))
        {
            if (sender == senderCol && filename == filenameCol && filepath == filepathCol)
            {
                receivers.push_back(receiverCol);
            }
        }
    }

    return receivers;
}

#endif // FILEUTILS_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include "commands.h"
#include "stringUtils.h"
#include "systemUtils.h"
#include "userUtils.h"

#define USERNAME_MAX_LEN 20

/* 
Display current user name and current directory. Wait for user to enter command.
*/
void prompt()
{
    std::string cmd;
    int numargs;
    while(1)
    {
        //TODO @team add your commands here
        std::cout << "Enter your command:> ";
        std::getline(std::cin, cmd);
        cmd = strip(cmd);
        if(cmd == "")
        {
            continue;
        } 
        else if (cmd == "pwd")
        {
            pwd();
        }
        else 
        {
            std::cout << "Invalid command." << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Invalid argument. Check user manual." << std::endl;
        exit(1);
    }
    
    // Init filesystem root folder if not exist
    if (fileSystemExist()) {
        login(argv[1]);
    } else {
        initSystemRoot();
        addUser(argv[1], true);
    }
    
    std::string info = "Encrypted Filsystem:\n\nAvailable Commands:\ncd <dir>\nls\npwd\nmkfile <file> <contents> \
    \nmkdir <dir>\ncat <file>\nshare <file> <user>\nexit\n\nAdmin can also use:\nadduser <user>\n\nFilename/username constraints: \
    \nMax 20 characters. Can only contain 'A-Z','a-z','0-9','-','_','.','='.\nFile contents max length: 470 bytes.\n\n";
    std::cout << info << std::endl;
    prompt();

    return 0;
}

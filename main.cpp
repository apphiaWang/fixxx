#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <sstream>
#include <sys/stat.h>

#include "commands.h"
#include "fileUtils.h"
#include "stringUtils.h"
#include "userUtils.h"
#include "authentication.h"

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
        std::cout << "CMPT785BIBIFI> ";
        std::getline(std::cin, cmd);
        cmd = strip(cmd);
        if(cmd == "")
        {
            continue;
        }
        else if(cmd == "exit") 
        {
            exit(1);
        }
        else if (cmd == "pwd")
        {
            pwd();
        }
        else if (cmd == "ls")
        {
            ls();
        }
        else 
        { // commands with arguments
            std::vector<std::string> args = split(cmd, ' ');

            if (args[0] == "cd") 
            {
                if (args.size() != 2) 
                {
                    std::cout << "invalid argument, check user manual" << std::endl;
                } else {
                    cd(args[1]);
                }
            }
            else if (args[0] == "mkdir") 
            {
                if (args.size() != 2) 
                {
                    std::cout << "invalid argument, check user manual" << std::endl;
                } else {
                    mkdir(args[1]);
                }                
            }
            else {
                std::cout << "Invalid command" << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    const char *seed = "some random fake encryption template....";
    
    if(argc != 2)
    {
        std::cout << "Invalid argument. Check user manual." << std::endl;
        exit(1);
    }
    
    // Init filesystem root folder if not exist
    struct stat st;
    const char *folder_name = "filesystem";
    const char *public_folder_name = "public_keys";
    const char *private_folder_name = "filesystem/private_keys";
    	
    if (stat(folder_name, &st) == -1) {
        if (errno == ENOENT) {
            std::cout << "Initializing CMPT785 encrypted file system..." << std::endl;
            mkdir(folder_name, S_IRWXU);
            mkdir(public_folder_name, S_IRWXU);
            mkdir(private_folder_name, S_IRWXU);
            std::cout << "Generating key pair for " << argv[1] << "..." << std::endl;
            std::string name_prefix(argv[1]); 
            generate_key_pair(name_prefix);
            std::cout << "Key pair generated." << std::endl;
        } else {
            // @TODO Some other error occurred.
            return 0;
        }
    } else {
        if (S_ISDIR(st.st_mode)) {
            std::cout << "Verifying...." << std::endl;
            std::string name_prefix(argv[1]); 
            bool is_valid = validate_login(name_prefix, seed);
            if (is_valid) {
                std::cout << "Login succeeded." << std::endl;                
                // @TODO check if user is admin and track status, now assume user is always admin
                isAdmin = true;
            } else {
                std::cout << "Login failed." << std::endl;
                return 0;
            }
        } else {
            return 0;
        }
    }

    std::string info = "CMPT785 Encrypted Filsystem:\n\nAvailable Commands:\ncd <dir>\nls\npwd\nmkfile <file> <contents> \
    \nmkdir <dir>\ncat <file>\nshare <file> <user>\nexit\n\nAdmin can also use:\nadduser <user>\n\nFilename/username constraints: \
    \nMax 20 characters. Can only contain 'A-Z','a-z','0-9','-','_','.','='.\nFile contents max length: 470 bytes.\n\n";
    std::cout << info << std::endl;
    prompt();  

    return 0;
}

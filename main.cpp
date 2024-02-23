#include <filesystem>
#include <fstream>
#include <cstring>
#include <map>
#include <vector>
#include <sstream>
#include "commands.h"
#include "fileUtils.h"
#include "stringUtils.h"

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
            else if(args[0] == "cat")
            {
                if(args.size() != 2)
                    std::cout << "invalid argument, check user manual" << std::endl;
                else
                    cat(args[1]);
            }
            else if(args[0] == "adduser")
            {
                if(args.size() != 2)
                    std::cout << "invalid argument, check user manual" << std::endl;
                else
                    adduser(args[1]);
            }
            else if(args[0] == "share")
            {
                if(args.size() != 3)
                    std::cout << "invalid argument, check user manual" << std::endl;
                else
                    share(args[1], args[2]);
            }
            else if(args[0] == "mkfile")
            {
                // re-split the command
                size_t contentStart = cmd.find_first_of('"');
                size_t contentEnd = cmd.find_last_of('"');
                if (contentStart == std::string::npos || contentStart == contentEnd || contentEnd != cmd.size()-1) {
                    std::cout << "invalid argument, check user manual" << std::endl;
                    continue;
                }
                std::string new_arg = strip(cmd.substr(0, contentStart-1));
                auto new_args = split(new_arg, ' ');
                std::string file_content = cmd.substr(contentStart + 1, contentEnd - contentStart-1);
                if(new_args.size() != 2)
                    std::cout << "invalid argument, check user manual" << std::endl;
                else
                    mkfile(new_args[1], file_content);
            }
            else {
                std::cout << "Invalid command" << std::endl;
            }
        }
    }
}

void init_filesystem(const std::string& name){
    const char *folder_name = "filesystem";
    const char *metadata = "filesystem/.metadata";
    const char *public_folder_name = "public_keys";
    const char *private_folder_name = "filesystem/.private_keys";
    std::cout << "Initializing CMPT785 encrypted file system..." << std::endl;
    mkdir(folder_name, S_IRWXU);
    mkdir(public_folder_name, S_IRWXU);
    mkdir(private_folder_name, S_IRWXU);
    mkdir(metadata, S_IRWXU);
    std::cout << "Generating key pair for " << name << "..." << std::endl;
    std::string name_prefix(name); 
    generate_key_pair(name_prefix);
    std::cout << "Key pair generated." << std::endl;
    
    std::ofstream userfile("filesystem/.metadata/users.txt");
    std::string username_cypher = encrypt_decrypt(name);
    if (userfile.is_open()) {
        // Write the text to the file
        userfile << username_cypher;
        // Close the file
        userfile.close();
    }
    std::ofstream fileShareMappingFile("filesystem/.metadata/fileShareMapping.txt");
    fileShareMappingFile.close();
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
    	
    if (stat(folder_name, &st) == -1) {
        if (errno == ENOENT) {
            init_filesystem(argv[1]);
            isAdmin = true;
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
                isAdmin = checkRole(name_prefix);
                if (!isAdmin) {
                    enterUserHome(name_prefix);
                }
            } else {
                std::cout << "Login failed." << std::endl;
                return 0;
            }
        } else {
            return 0;
        }
    }
    currentUser = argv[1];

    std::string userInfo = "CMPT785 Encrypted Filesystem:\n\nAvailable Commands:\ncd <dir>\nls\npwd\nmkfile <file> <content> \
    \n \t*quote your file content with double quote, i.e. \" \
    \nmkdir <dir>\ncat <file>\nshare <file> <user>\nexit\n";

    std::string nameConstraint = "Filename constraints: \
    \nMax 20 characters. Can only contain 'A-Z','a-z','0-9','-','_','.'.\nFile contents max length: 4096 bytes.\n";
    std::cout << userInfo << std::endl;
    if (isAdmin){
        std::cout << "Admin-only commands:\nadduser <user>\n" << std::endl;
        std::cout << "Username/" << nameConstraint << std::endl;
    } else {
        std::cout << nameConstraint << std::endl;
    }
    prompt();  

    return 0;
}

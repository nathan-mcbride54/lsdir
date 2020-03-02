/*
|   NAME: lsdir
|   AUTHOR: Nathan McBride (0615415)
|   PURPOSE: List out the contents of a directory
|
|   USAGE:  ./lsdir (current directory)
|           ./lsdir -s 10 -b 5 /home/COIS/3380/lab3
|
|   PARAMETERS: 
|       -s (specify by oldest files, followed by maximum age in days)   
|       -b (specify by newest files, followed by minimum age in days)    
*/


#include <dirent.h> // Directory structures
#include <limits.h> // Used for PATH_MAX
#include <stdio.h> // Basic I/O functions
#include <stdlib.h> // Standard libraries
#include <string.h> // String functions
#include <sys/stat.h> // File status functions
#include <time.h> // Time functions
#include <unistd.h> // Used to detect current directory


char* getFileType(struct stat file); // Returns type of file depending on bitmask

void main(int argc, char *argv[]){
    
    int bValue = 0; // User supplied "since" value
    int sValue = 0; // User supplied "before" value
    int userArg; // Stores the argument returned by getopt
    char* dirPath; // Pointer to directory path
    char cwd[PATH_MAX]; // Array to store path of current directory
    DIR *openDirectory; // Pointer to the directory to be listed
    struct dirent *directoryEntry; // Pointer to struct containing information about the specified directory
    struct stat file_status; // Struct to hold the status of the current file

    // If user supplied no arguments, assume current directory
    if(argv[1] == NULL)
    {   
        // Set current working directory if not NULL
        if(getcwd(cwd, sizeof(cwd)) != NULL) 
        {
            dirPath = cwd;
        }
        else
        {
            printf("Error determing current directory, exiting...");
            exit(1);
        }
    }
    else
    {
        // Loop while until the end of argv array is reached
        while(optind < argc)
        {
            // Use getopt to check if -s or -b flags have been supplied
            if ((userArg = getopt (argc, argv, "b:s:")) != -1)
            {
                switch(userArg)
                {
                    case 'b':
                        bValue = atoi(optarg);
                    break;
                    
                    case 's':
                        sValue = atoi(optarg);
                    break;

                    case '?':
                        printf("You supplied an invalid argument, exiting...\n");
                        exit(22);
                    break;
                }
            }

            // Once getopt has read the arguments, set directory path and exit the loop
            else
            {
                dirPath = argv[optind];
                argc = 0;
            }
        }
    }

    // Append "/" to the end of the supplied path to avoid errors
    if(dirPath[strlen(dirPath)-1] != '/')
    {
        strcat(dirPath, "/");
    }

    printf("%s\n", dirPath);
    // Validate that the directory exists by attempting to open it
    if((openDirectory = opendir(dirPath)) == NULL)
    {
        printf("Directory: \"%s\" not found, exiting...\n", dirPath);
        exit(1);
    }

    // Display column names for the file information
    printf("%-10s%5s%6s%6s%10s%40s%27s\n","inode","Type","UID","GID","SIZE","Filename","Last Modified");

    // Reference: https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
    while ((directoryEntry = readdir(openDirectory)) != NULL) // 
    {
        char file[4096]; // Array to store the full path of the directory, and file
        strcpy(file, dirPath); // Save pathname to array
        strcat(file, directoryEntry->d_name); // Save filename to array

        // Read the current file, and store it's information using stat. 
        if (stat(file, &file_status) == 0)
        {
            if (sValue != 0 || bValue != 0) // If either B or S flag was used
            {
                time_t curTime = time(0); // Set current time

                // Calculate the difference between the current date and the files creation date
                int difTimeDays = (difftime(curTime, file_status.st_mtime) / 24) / 3600; 
                
                if(sValue != 0)
                {
                    if(difTimeDays > sValue) { continue; } // If the file is too recent, skip
                }

                if(bValue != 0)
                {
                    if(difTimeDays < bValue) { continue; } // If the file is too old, skip
                }
            }

            printf("%-10ld", (long)file_status.st_ino); // Print inode number
            printf("%5s", getFileType(file_status)); // Print file type
            printf("%6ld", (long)file_status.st_uid); // Print owner UID
            printf("%6ld", (long)file_status.st_gid); // Print owner GID
            printf("%10lld", (long long)file_status.st_size); // Print file size
            printf("%40s", directoryEntry->d_name); // Print name of file
            printf("%27s", ctime(&file_status.st_mtime)); // Print date last modified
        }
    }
}

// Function: getFileType
// Parameters: Structure containing the file information
// Usage: Returns the type of file depending on bitmask
char* getFileType(struct stat file) 
{  
    char* fileType;

    // Check file bitmask and set fileType according to the result
    switch(file.st_mode & __S_IFMT) 
    {
        case __S_IFREG: fileType = "REG "; break;
        case __S_IFDIR: fileType = "DIR "; break;
        case __S_IFIFO: fileType = "FIFO"; break;
        case __S_IFLNK: fileType = "LINK"; break;
        case __S_IFSOCK: fileType = "SOCK"; break;
        case __S_IFCHR: fileType = "C_DEV"; break;
        case __S_IFBLK: fileType = "B_DEV"; break;
        default: fileType = "----"; break;
    }
    return fileType;
}
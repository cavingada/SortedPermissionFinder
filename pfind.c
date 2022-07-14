#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

// author @ Cavin Gada

int verifyPermissionString(const char* s) {
    // compute length of string
    int len = (int) strlen(s); 

    // check if string length is too small/large. (a valid persmission string has only 9 characters)
    if (len>9 || len <9) {
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", s);
        return 1;
    }

    for (int i = 0; i < 9; i+=1) {
        /*
        if this is the first position of every set of three positions in the string, 
        the character is invalid if it is not 'r' or '-'
        */
        if (i%3 == 0 && !(s[i] == 'r' || s[i] == '-')) {  
            fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", s);
            return 1;
        }
        /*
        if this is the second position of every set of three positions in the string, 
        the character is invalid if it is not 'w' or '-'
        */
        else if (i%3 == 1 && !(s[i] == 'w' || s[i] == '-')) {
            fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", s);
            return 1;
        }
        /*
        if this is the third position of every set of three positions in the string, 
        the character is invalid if it is not 'x' or '-'
        */
        else if (i%3 == 2 && !(s[i] == 'x' || s[i] == '-')) {
            fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", s);
            return 1;
        }
    }
    return 0;
}

// credit to: https://c-for-dummies.com/blog/?p=4101 
// which was useful when figuring how to check if permission true (& operator)

char* statModeToString(mode_t mode) {

    int permission_AND_Mode;

    char *permString = malloc(9);

    for (int i = 0; i < 9; i++) {

        /*
        if we are checking the first three letters, then compare with owner perms
        */
        if (i<3) {
            if (i%3 == 0) {
                permission_AND_Mode = mode & S_IRUSR;
                permString[i] = permission_AND_Mode ? 'r' : '-';
            }
            else if (i%3 == 1) {
                permission_AND_Mode = mode & S_IWUSR;
                permString[i] = permission_AND_Mode ? 'w' : '-';
            }
            else if (i%3 == 2) {
                permission_AND_Mode = mode & S_IXUSR;
                permString[i] = permission_AND_Mode ? 'x' : '-';
            }
        }
        /*
        if we are checking the second three letters, then compare with group perms
        */
        else if (i<6) {
            if (i%3 == 0) {
                permission_AND_Mode = mode & S_IRGRP;
                permString[i] = permission_AND_Mode ? 'r' : '-';
            }
            else if (i%3 == 1) {
                permission_AND_Mode = mode & S_IWGRP;
                permString[i] = permission_AND_Mode ? 'w' : '-';
            }
            else if (i%3 == 2) {
                permission_AND_Mode = mode & S_IXGRP;
                permString[i] = permission_AND_Mode ? 'x' : '-';
            }
        }
        /*
        if we are checking the last three letters, then compare with other perms
        */
        else {
            if (i%3 == 0) {
                permission_AND_Mode = mode & S_IROTH;
                permString[i] = permission_AND_Mode ? 'r' : '-';
            }
            else if (i%3 == 1) {
                permission_AND_Mode = mode & S_IWOTH;
                permString[i] = permission_AND_Mode ? 'w' : '-';
            }
            else if (i%3 == 2) {
                permission_AND_Mode = mode & S_IXOTH;
                permString[i] = permission_AND_Mode ? 'x' : '-';
            }
        }
    }
    return permString;
}


void traverseDirectory(char *path, const char* permissions) {

    DIR *dp;
    struct dirent *dirp;

    dp = opendir(path);

    while ((dirp = readdir(dp)) != NULL) {
        
        // keep file name handy
        char* itemName = dirp->d_name;

        // if the file name is . or .. just skip over it
        if (strcmp(itemName, ".") == 0 || strcmp(itemName, "..")==0) {
            continue;
        }
        // create the new path to begin traversing through
        char newPath[strlen(path) + strlen(itemName) + 1]; 
        strcpy(newPath, path);
        strcat(newPath, "/");
        strcat(newPath,itemName);

        struct stat fileInfo;
        int status = stat(newPath, &fileInfo);
        char* test = statModeToString(fileInfo.st_mode); // convert file permissions into char*

        // if we are dealing with a directory...
        if (dirp->d_type == DT_DIR) {
    
            //printf("%c", test[0]);
            if (test[0] == 'r') {
                free(test);
                traverseDirectory(newPath, permissions);
                continue;
            }
            else {
                free(test);
                fprintf(stderr, "Cannot open directory '%s'. Permission denied.\n", newPath);
                continue;
            }
        }

        if (strcmp(test, permissions) == 0) { // compare if command line perms = file perms
            printf("%s\n", newPath);         // print the file name if they match!
        }

        free(test);                            // free allocated memory 
    }
    closedir(dp);                            
}

int main(int argc, char *argv[]) {

    char buf[PATH_MAX]; // use max path amount so we can accomodate any directory input
    char* dir = realpath(argv[1], buf); // real path should give us the total path, not just the relative directory
    const char* permissionString = argv[2];

    int exit = verifyPermissionString(permissionString);

    if (exit == 1) {
        return exit;
    }

    traverseDirectory(dir, permissionString);

    return 0;
}
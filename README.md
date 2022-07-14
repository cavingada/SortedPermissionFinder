# SortedPermissionFinder
This project finds files with a specified set of
permissions. This program will recursively search for files whose permissions match the
permissions string starting in the specified directory.
Permissions strings are formatted similarly to how the command ls formats them.
In UNIX systems, the leftmost character specifies the type of file (d for directory, l for
symlink, etc). The permission string passed as command-line argument will only contain
the right-most 9 characters, such as r-xrw-r-x.

Then, this project uses pfind.c and and sorting its
output (.o file). To utilize this tool, I use a combination of fork(), exec*(), and pipe().

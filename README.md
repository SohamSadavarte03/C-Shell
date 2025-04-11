# C Shell

This project is a custom shell implementation in C. The shell supports basic command execution, directory traversal, process management, and more.


## Features

- *Custom Commands*: Implemented custom commands like hop, reveal, log, seek, proclore.
- *Directory Traversal*: Easily navigate directories with custom commands.
- *Process Management*: Manage processes with custom and built-in commands.
- *Directories and files*: Can list files and directories in a directory.


## Files and Directories

- main.c: The main entry point for the shell program.
- input.c: Did string splitting and background and foreground processes handled.
- log.c: Handles logging and history management.
- hop.c: Implements the hop command for directory traversal.
- reveal.c: Implements the reveal command for listing files and directories.
- proclore.c: Implements process-related commands.
- seek.c: Implements the seek command for file and directory search.




### Compilation

To compile the project, run the following command in the root directory:

make

After compilation, you can run the shell using
make run

Alternatively, you can directly run the executable
./a.out

To remove the compiled files and the executable, run
make clean

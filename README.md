
# C Shell

This project is a custom shell implementation in C. The shell supports basic command execution, directory traversal, process management, and more. The source files are organized in the codes directory.

## Features

- *Custom Commands*: Implemented custom commands like hop, reveal, log, seek, proclore, fg, bg, activities, neonate, iMan and can add aliases too.
- *Directory Traversal*: Easily navigate directories with custom commands.
- *Process Management*: Manage processes with custom and built-in commands.
- *Signal Handling*: Handles signals to manage processes and cleanup resources.
- *Colored Output*: Outputs file and directory names in colored text for easier identification.

## Files and Directories

- main.c: The main entry point for the shell program.
- input.c: Utility functions for string splitting and command parsing.
- log.c: Handles logging and history management.
- hop.c: Implements the hop command for directory traversal.
- reveal.c: Implements the reveal command for listing files and directories.
- proclore.c: Implements process-related commands.
- seek.c: Implements the seek command for file and directory search.
- iman.c: Implements the iman function for printing man pages from internet using sockets.

## Limitations and Assumptions

The shell implementation makes certain assumptions and has specific limitations based on defined constants:

- Commands with '&' and '|' consecutively will not work.
- Tokenising is done first with '|' and then with '&'.
- setpgid(0,0) is used. So vim does not work.
- Custom functions other than mk_hop() and hop_seek() dont work.
- If 'sleep 100' entered and sent to background and stopped. And if fg is called for it(using its pid) then it will run for less than 100 secs.
- Double quotes function will not work in execvp.
- If there are two | one after another with no spaces then the command till '||' will run and command in right of '||' will be ignored. But if two | have space between them then it is invalid command.
- If theres | in the start or end it is invalid command.
- If command is 'wc < a.txt > b.txt > new.txt' then wc will read from a.txt and print it in b.txt. Basically wc will take input or give output to file after first >(>> also) or <.




### Compilation

To compile the project, run the following command in the root directory:


make

After compilation, you can run the shell using
make run

Alternatively, you can directly run the executable
./a.out

To remove the compiled files and the executable, run
make clean


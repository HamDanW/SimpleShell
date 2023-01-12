# ECS150: Project #1 - Simple Shell
## Introduction
In this project we created a simple shell program that functions as a command-line interpreter. It will take user inputs from the command line and execute them as well as displaying a completion message for user feedback. This project helps improve our understanding of the UNIX system as well as a deeper understanding of how command lines work. It also introduces students to a challenge in coding with C creating stronger software developing skills.
## Specifications
The program can run simple commands such as (ls, ps, cat, echo) as well as built in commands (exit, cd, pwd, sls). When the user inputs the character ">" as well as a file name after the a command it will prompt the program to redirect the output of the command into a newly created file named by the user. The program also has implementations of piping, which is indicated by the character "|", which will connect the stdout of the previous command to the stdin of the next command until the last command.
## Design
The first part of the program's design was creating an execution function that will parse the cmd line into commands and arguments as well as recognize the characters ">" and "|" for file redirection and piping. The execution function will delete all the spaces from the user input and place each string into an array. It will also append the arguments together into one string that will then be used as the full input for the argument. The execution function also holds the forking of the parent and child inside its contents along with file redirection. The last section of execution will check if the piping exsist for either two or three different sections of commands. If they exist then it will either go into pipeline2 or pipeline3. The main function will hold all the build in commands as well as running a loop that includes the execution function and the sls function in extra features. The parsePipe function is used to parse the different commands and arguments before each of the "|" characters. The parseArgv is used to remove all the spaces in the entire cmd line.
## Solved Issues
The first issues we had was memory not being cleared, which causes an issue where a letter "n" would appear before every single argument. The next issue we faced was the cmd line containing a newline character "\n" that we didn't recognize until later. Another issue we faced was implementing sls, which had issues with going through the directory entry. The biggest issue was implementing piping multiple commands. We tried making it pipe multiple commands through a while loop, but it got too complicated. That was when we decided to make a max piping limit of 3 times. The last issue was during error management where we had to tweak our previous code in order to satisfy the criteria for all the error management issues.
## Sources
https://www.tutorialspoint.com/c_standard_library/c_function_system.htm <br/>
https://www.geeksforgeeks.org/how-to-append-a-character-to-a-string-in-c/ <br/>
https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm <br/>
https://en.cppreference.com/w/c/experimental/dynamic/strdup <br/>
https://stackoverflow.com/questions/8107826/proper-way-to-empty-a-c-string <br/>
https://www.youtube.com/watch?v=uHH7nHkgZ4w


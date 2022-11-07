# Custom-Shell
A Custom-Shell (like linux terminal) in C language.

# How to Run
Just compile `CustomShell.c` file in the repo using `gcc` and run the executable.



# Functionalities of the Terminal (CustomShell)
The CustomShell supports the following:

1.  Built-in commands: `cd`, `pwd`, `exit` (closes CustomShell itself).

2.  Accept commands (names of executables that are existing on the file
    system), and execute them in both the foreground mode (shell waits for
    the execution of the command to complete), and the background mode
    (where shell does not wait for the command to complete). The suffix `&`
    is used to execute a command in the background mode.

3.  Support I/O redirection using `<<<` , and `>>>` operators.

4.  Support command pipelines using `|` (custom) operator.

5.  Control the execution of the commands by interrupting an execution or
    terminating the execution of a command (support `SIGINT`, `SIGSTP`).

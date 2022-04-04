# TDT 4186 - Practical Exercise 3

## How it works

A struct is made for the command input in the prompt, as well as
a struct for a linked_process, which is used to keep track of
background processes. Linked processes are inserted and deleted 
in the linked list as background processes are executed and terminated.

The `parse_command` writes to the command struct by iterating over the
input. It also checks if the command should be a background process by
checking if the last character in the input is equal to '&'.

The `execute_command` exectues the command using `execvp`. The command
name and command args are read from the command_t struct. Before it executes,
it forks and the args are iterated over to check if it contains input redirection, and sets
flags (in and out) based on the results. It then (based on the flags) opens 
the input/output, duplicates the file descriptor into STDIN/STDOUT (and opens),
and lastly closes the original file.
When this is done the command is executed.

The main method initializes some variables used. Flush is run in a simple while-loop
where it continuously checks the input, overwrites the command_t and executes the command.
Inside the loop it's also checked for internal commands (cd and jobs) by checking if the
command name is equal to either 'cd' or 'jobs', and executes accordingly. Lastly, memory used
in the command_t is freed.

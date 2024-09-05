# Seashell: A Unix Shell
Seashell is a basic custom shell built in C that mimics some core features of
popular Unix shells. It supports built-in commands, Unix command execution, and
input/output redirection. This project was a fun way to utilize some system-level
programming concepts from my computer systems course.

## Features
- **Unix Command Execution**: Supports executing standard Unix commands like
  `ls`, `echo`, and more.
- **Built-in Commands**:
  - `cd`: Change the current working directory.
  - `help`: Displays a list of built-in commands and usage information.
  - `exit`: Exit the shell session.
  - `seashell`: Displays a humorous seashell-themed joke.
- **Input Redirection (`<`)**: Redirects input from a file.
- **Output Redirection**:
  - `>`: Redirects output to a file (truncates the file).
  - `>>`: Redirects output to a file (appends to the file).
- **Prompt Customization**: Displays the current working directory.

## Usage
To test out the shell, follow the instructions below.

### Compilation
Ensure you have GCC installed, then use the provided `Makefile`:
```bash
make
```
This will generate an executable called `seashell`.

### Running the Shell
You can run the shell by executing:
```bash
./seashell
```

### Example Commands
Here are some example commands to try:
```bash
ls
cd /path/to/directory
echo "Hello, Seashell!" > output.txt
cat < input.txt
cat < input.txt >> output.txt
```

### Cleanup
To clean up the compiled files:
```bash
make clean
```



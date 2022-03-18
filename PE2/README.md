
# TDT4186 - Practical Exercise 2

## 2.1 a)-d)
Solutions are implemented in `mtwwwd.c`, `sem.c` and `bbuffer.c`. The contents in `www` exist to provide a demo of the web server.

To run the web server with the demo files, use
```./mtwwwd www 4242 8 16```
This will start the web server with 8 threads and a buffer size of 16, on port 4242. You can then open the index page in your browser with
```localhost:4242```

## 2.1 e)

### Directory Traversal attack
Directory traversal is vulnerability that allows an attacker to read arbitrary files on the server. The following terminal output is an example.
```sh
computerman@Computouir / % curl localhost:4242/../secret.txt --path-as-is
You should not be able to access this!
computerman@Computouir / % 
```
The server is supposed to host the contents of `www`. `secret.txt` is located in a parent directory, but can still be accessed. 

Suggested solutions:
1. Validate the requested path (e.g. remove `/..`).
2. Set file permissions of the process so that it is unable to access files not in the `/{directory}` directory.

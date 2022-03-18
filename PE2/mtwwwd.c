#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "bbuffer.h"

#define MAXREQ (4096*1024)

char *www_path;
int port;
int threads;
int bufferslots;

int sockfd, newsockfd;
socklen_t clilen;
struct sockaddr_in6 serv_addr, cli_addr;

BNDBUF* bbuffer;


void error(const char *msg) {
    perror(msg);
    exit(1);
}


typedef struct {
    char type[8]; // GET, PUT, POST etc.
    char path[100]; // The requested path
} http_request;


http_request read_request(char* request) {
    http_request req;
    // Set strings to all zeros. If not done, undefined behavior occurs.
    memset(req.type, '\0', sizeof(req.type));
    memset(req.path, '\0', sizeof(req.path));
    
    int read_cursor = 0;
    int write_cursor = 0;
    int space_count = 0;
    char* attr;
    
    // Iterate through request and find information separated by space or newline
    while (space_count < 2) {
        char letter = request[read_cursor];
        switch (space_count) {
            case 0: attr = &req.type[0]; break;
            case 1: attr = &req.path[0]; break;
        }
        if (letter == ' ' || letter == '\n') {
            attr[write_cursor] = '\0';
            write_cursor = 0;
            space_count++;
        } else {
            attr[write_cursor] = letter;
            write_cursor++;
        }
        read_cursor++;
    };

    return req;
}


void get_content_type(char* path, char* result) {
    char const s[2] = ".";
    // These pointers could be allocated per thread and passed into the function
    // to improve application efficiency
    // However, this would cause the application to use a bit more memory
    // as inactive threads would keep memory allocated.
    char *token = malloc(100);
    char *token_prev = malloc(100);

    token = strtok(path, s);
    
    while(1) {
        token = strtok(NULL, s);
        if (token == NULL) {
            result = "text/plain";
            if (strlen(token_prev) < 3) {
                break;
            }
            if (!strcmp(token_prev, "html")) result = "text/html";
            if (!strcmp(token_prev, "css"))  result = "text/css";
            if (!strcmp(token_prev, "ico"))  result = "image/x-icon";
            if (!strcmp(token_prev, "png"))  result = "image/png";
            if (!strcmp(token_prev, "jpg"))  result = "image/jpeg";
            if (!strcmp(token_prev, "gif"))  result = "image/gif";
            if (!strcmp(token_prev, "txt"))  result = "text/plain";
            break;
        } else {
            *token_prev = *token;
        }
    }
    free(token);
    free(token_prev);
    return;
}

void read_file(char *path, long *length, char result[MAXREQ]) {
    FILE *file;
    char abs_path[200] = {'\0'};
    strcat(abs_path, www_path);
    
    if (!strcmp(path, "/")) {
        strcat(abs_path, "/index.html");
    } else {
        strcat(abs_path, path);
    }
    
    file = fopen(abs_path, "rb");
    if (file == NULL) {
        memset(abs_path, '\0', sizeof(abs_path));
        strcat(abs_path, www_path);
        strcat(abs_path, "/404.html");
        file = fopen(abs_path, "rb");
    }

    fread(&result[0], MAXREQ, 1, file);
    fseek(file, 0, SEEK_END); // seek to end of file
    *length = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET); // seek back to beginning of file
}


void* serve_request() {
    long body_size = 0;
    char* content_type = malloc(16);
    int threadsockfd;
    ssize_t n;
    char* body;
    char* buffer;
    char* msg;
    
    while (1) {
        
        // Get newsockfd from bbuffer
        threadsockfd = bb_get(bbuffer);
        body = malloc(MAXREQ);
        buffer = malloc(MAXREQ);
        msg = malloc(MAXREQ);
        
        // Reset buffer
        bzero(buffer, MAXREQ);
        
        // Read the HTTP request into buffer
        n = read(threadsockfd, buffer, MAXREQ-1); 
        if (n < 0) error("ERROR reading from socket");
        
        http_request received_request = read_request(buffer);
        printf("\n%s to %s\n", received_request.type, received_request.path);
        memset(body, '\0', MAXREQ);
        read_file(received_request.path, &body_size, body);
        
        // Generate response
        get_content_type(received_request.path, content_type);
        
        snprintf(msg, MAXREQ,
            "HTTP/1.1 200 OK\n"
            "Content-Type: %s\n"
            "Content-Length: %lu\n\n", content_type, body_size);
        
        // Places the body after the header. It is done this way to allow arbitrary binary files
        // to be sent, including images. These may include the \0 string terminator.
        memcpy(&msg[strlen(msg)], body, body_size);

        // Send the response
        n = write(threadsockfd, msg, strlen(msg) + body_size); 
        if (n < 0) error("ERROR writing to socket");
        
        // Close the connection
        close (threadsockfd);
        free(buffer);
        free(msg);
        free(body);
    }
    free(content_type);
    printf("\nGoodbye.\n");
    return 0;
}

int main(int argc, char *argv[]) {

    // Handle command line arguments
    if (argc == 5) {
        www_path = argv[1];
        port = atoi(argv[2]);
        threads = atoi(argv[3]);
        bufferslots = atoi(argv[4]);
    } else {
        error("Invoke this command using mtwwwd [www-path] [port] [#threads] [#bufferslots]");
    }
    printf("Setting up a server for %s on port %i\n\n", www_path, port);
    
    // Create the socket
    sockfd = socket(PF_INET6, SOCK_STREAM, 0);
    int flag = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    // Binds the socket to an address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd, 5);

    // Create bbuffer
    bbuffer = bb_init(bufferslots);
    
    // Create threads
    int thread;
    pthread_t server_threads[threads];
    for (int i = 0; i < threads; i++) {
        thread = pthread_create(&server_threads[i], NULL, serve_request, NULL);
    }
    
    while(1) {
        // Set size of client address
        clilen = sizeof(cli_addr);
        
        // Accept a new connection
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        
        // Pass file descriptor to bbuffer
        bb_add(bbuffer, newsockfd);
    }
    bb_del(bbuffer);
    return 0;
}

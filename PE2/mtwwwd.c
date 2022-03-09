#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAXREQ (4096*1024)

char buffer[MAXREQ], msg[MAXREQ];
char body[65535] = {'\0'};
char content_type[16] = "";
char *www_path;
int port;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    char type[8];
    char path[100];
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

void get_content_type(char* path, char result[16]) {
    const char s[2] = ".";
    char *token;
    char *token_prev;

    token = strtok(path, s);
    
    while(1) {
        token = strtok(NULL, s);
        if (token == NULL) {
            result = "text/plain";
            if (strlen(token_prev) < 3) {
                return;
            }
            if (!strcmp(token_prev, "html")) result = "text/html";
            if (!strcmp(token_prev, "css"))  result = "text/css";
            if (!strcmp(token_prev, "ico"))  result = "image/x-icon";
            if (!strcmp(token_prev, "png"))  result = "image/png";
            return;
        } else {
            token_prev = token;
        }
    }
}

void read_file(char *path, char result[65535]) {
    FILE *file;
    char row[65535];
    char abs_path[200] ={'\0'};
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
    //fgets(result, 65535, file);
    fread(&result[0], 65535, 1, file);
    
    //while (fgets(row, sizeof row, file) != NULL) {
    //   strcat(result, row);
    //}
}

int main(int argc, char *argv[]) {

    if (argc == 3) {
        www_path = argv[1];
        port = atoi(argv[2]);
    }
    printf("Setting up a server for %s on port %i\n\n", www_path, port);


    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    
    // Creates the socket
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    
    // Binds the socket to an address
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd, 5);

    while (1) {   
        
        // Set size of client address
        clilen = sizeof(cli_addr);
        
        // Accept a new connection
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
            &clilen); 
        if (newsockfd < 0) error("ERROR on accept");
        
        // Reset buffer
        bzero(buffer, sizeof(buffer));

        // Read the HTTP request into buffer
        n = read(newsockfd,buffer, sizeof(buffer)-1); 
        if (n < 0) error("ERROR reading from socket");

        http_request received_request = read_request(buffer);
        printf("\nType: %s\nPath: %s\n", 
                received_request.type, received_request.path);
        memset(body, '\0', sizeof(body));
        read_file(received_request.path, body);
        printf("%s", body);

        // Make body of response
        get_content_type(received_request.path, content_type);
        // Generate response

        snprintf(msg, sizeof (msg),
            "HTTP/1.1 200 OK\n"
            "Content-Type: %s\n"
            "Content-Length: %lu\n\n%s", content_type, strlen(body), body);

        // Send the response
        n = write(newsockfd, msg, strlen(msg)); 
        if (n < 0) error("ERROR writing to socket");

        // Close the connection
        close (newsockfd);
    }
}


#include "headers.h"
#define PORT 5000

int main(int argc, char *argv[])
{
    struct sockaddr_in server_address, client_address;
    socklen_t sin_len = sizeof(client_address); //socket length = size of the client address
    int server, client;                         //file descriptor
    char buffer[2048];                          //save the content sent by browser
    int tempFIle;                              //holding the file descriptions of files that is open
    int on = 1;

    FILE *filePointer;
    int bufferLength = 10000;
    char bufferHome[bufferLength];
    char bufferAbout[bufferLength];
    char bufferContact[bufferLength];

    filePointer = fopen("index.html", "r");

    while (fgets(bufferHome, bufferLength, filePointer))
    {
        printf("%s\n", bufferHome);
    }

    fclose(filePointer);

    filePointer = fopen("about.html", "r");

    while (fgets(bufferAbout, bufferLength, filePointer))
    {
        printf("%s\n", bufferAbout);
    }

    fclose(filePointer);

    filePointer = fopen("contact.html", "r");

    while (fgets(bufferContact, bufferLength, filePointer))
    {
        printf("%s\n", bufferContact);
    }

    fclose(filePointer);

    char http_header_home[2048] = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type:text/html; charset:UTF-8\r\n\r\n";
    char http_header_about[2048] = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type:text/html; charset:UTF-8\r\n\r\n";
    char http_header_contact[2048] = "HTTP/1.1 200 OK\r\n"
                                     "Content-Type:text/html; charset:UTF-8\r\n\r\n";
    strcat(http_header_home, bufferHome);
    strcat(http_header_contact, bufferContact);
    strcat(http_header_about, bufferAbout);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    { //something wrong when starting server
        perror("socket");
        exit(1);
    }

    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)); //setup socket properly

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) //Bind the socket to network
    {
        perror("bind");
        close(server);
        exit(1);
    }

    if (listen(server, 10) == -1) //Queue Size
    {
        perror("listen");
        close(server);
        exit(1);
    }

    while (1)
    {
        client = accept(server, (struct sockaddr *)&client_address, &sin_len);
        if (client == -1)
        {
            perror("Connection Failed....\n");
            continue;
        }
        printf("Client Connected....\n");

        if (!fork())
        {
            /*child process*/
            close(server);
            memset(buffer, 0, 2048);
            read(client, buffer, 2047); //reading from the port
            printf("%s\n", buffer);

            char request_type[8];                  // GET or POST
            char request_path[1024];               // /info etc.
            char request_protocol[128];            // HTTP/1.1
            const int request_buffer_size = 65536; // 64K
            char request[request_buffer_size];

            sscanf(buffer, "%s %s %s", request_type, request_path, request_protocol);

            char get_check[] = "GET";
            char about_check[] = "/about";
            char home_check[] = "/";
            char contact_check[] = "/contact";

            if (!strncmp(buffer, "GET /favicon.ico", 16))
            {
                tempFIle = open("favicon.ico", O_RDONLY);
                sendfile(client, tempFIle, NULL, 20000);
                close(tempFIle);
            }

            if (!strncmp(buffer, "GET /test.jpg", 16))
            {
                tempFIle = open("test.jpg", O_RDONLY);
                sendfile(client, tempFIle, NULL, 10000);
                close(tempFIle);
            }

            if (!strncmp(buffer, "GET /test.txt", 16))
            {
                tempFIle = open("test.txt", O_RDONLY);
                sendfile(client, tempFIle, NULL, 2000);
                close(tempFIle);
            }

            if ((strcmp(request_type, get_check) == 0) && (strcmp(request_path, about_check) == 0))
            { //about
                write(client, http_header_about, sizeof(http_header_about) - 1);
            }
            else if ((strcmp(request_type, get_check) == 0) && (strcmp(request_path, home_check) == 0))
            { //home
                write(client, http_header_home, sizeof(http_header_home) - 1);
            }
            else if ((strcmp(request_type, get_check) == 0) && (strcmp(request_path, contact_check) == 0))
            { //contact
                write(client, http_header_contact, sizeof(http_header_contact) - 1);
            }else{
                send(client,"HTTP/1.0 404 Not Found\n\n",20,0);
            }

            close(client);
            printf("Closing Connection...\n");
            exit(0);
        }
        /* parent process */
        close(client);
    }

    return 0;
}

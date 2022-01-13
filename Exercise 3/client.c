/**
 * @file client.c
 * @author Filip Markovic (e12024750@student.tuwien.ac.at)
 * @brief This program takes input and extracts information from the input to try to connect to a http
 * site on the internet.
 * @details First of all, the arguments get parsed, to know to which URL we connect (also on which port),
 * and to which output file we write. After that we get the addrinfo and connect to the server through 
 * sockets. 
 * @date 2022-01-12
 * 
 */
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

/**
 * @brief prints the correct way to run the program
 */
void usage(){
    fprintf(stderr, "[client] USAGE: client [-p PORT] [-o FILE | -d DIR] URL\n");
}

/**
 * @brief prepares the input URL and parses it into separate Strings for later use. 
 * 
 * @details The input string has to be a http:// website, otherwise it terminates. The standard file on 
 * the website we open (if not given) is /input.html.
 * 
 * @param host char pointer, to which the host name shall be saved (e.g. www.nonhttps.com)
 * @param filepath char pointer, to which the file path shall be saved (e.g. /index.html)
 * @param url char pointer, input URL, has to meet certain conditions
 * @return int 0 on succes, -1 on failure
 */
int parseUrl(char *host, char *filepath, char *url){
    if (strncmp(url, "http://", 7) != 0){
        fprintf(stderr, "ERROR: input String doesn't start with 'http://'\n");
        usage();
        return -1;
    }
    host = strcpy(host, &url[7]);
    char *after = host;

    host = strsep(&after, ";/?:@=&");
    if (*host == '\0'){
        fprintf(stderr, "ERROR: no host given in input\n");
        usage();
        return -1;
    }
    if (after != NULL && *after != '\0'){
        strcpy(filepath, "/");
        strcat(filepath, after);
    } else {
        strcpy(filepath, "/index.html");
    }

    return 0;
}

/**
 * @brief opens the file where the output (of the website) shall be saved
 * 
 * @details takes in parameters, and returns the file pointer to either stdout (if no optional arg given),
 * to directory if -d argument given, or to a specific file if -o file given. (Can't save to both dir and
 * output, only to one). 
 * 
 * @param output output file path to which the output shall be saved
 * @param directory output directory path to which the file shall be saved
 * @param filepath filepath which we open on the website, also the standard filename to which we save, if
 * only a directory is given.
 * @return FILE* FILE pointer to which we write our output, returns NULL on failure
 */
FILE *openOutput(char *output, char *directory, char *filepath){
    if (output == NULL && directory == NULL){
        return stdout;
    }

    FILE *file;
    if (output != NULL){
        char *tmp = malloc(sizeof(char) * strlen(output) + 2*sizeof(char));
        if (strncmp(output, "./", 2) != 0){
            tmp = strcpy(tmp, "./");
        }
        tmp = strcat(tmp, output);
        file = fopen(tmp, "w");
        free(tmp);
        if (file == NULL){
            return NULL;
        }
        return file;
    } else if (directory != NULL) {
        char *tmp = malloc(sizeof(char) * (strlen(directory) * strlen(filepath)) + sizeof(char));
        if (strncmp(directory, "./", 2) != 0){
            tmp = strcpy(tmp, "./");
        }
        tmp = strcat(tmp, directory);
        tmp = strcat(tmp, filepath);
        
        file = fopen(tmp, "w");
        if (file == NULL){
            free(tmp);
            return NULL;
        }
        free(tmp);
        return file;
    }

    return stdout;
}

/**
 * @brief Opens a socket to access the webpage and writes the output from the site. Here the main logic of
 * the program happens.
 * 
 * @details First we parse the input arguments with getopt(), if no port is given the standard port is 
 * 80. The standard output if nothing is given is stdout. Then we connect to the socket in a few steps
 * (like was given in the lecture). If everything worked, we check if the server response is valid and
 * we can read the response, which er write to our (in the last function given) output.
 * 
 * @param argc amount of input arguments
 * @param argv array of strings of input
 * @return int returns 0 on success, != 0 on failure
 */
int main(int argc, char *argv[])
{
    int c, opt_p = 0, opt_o = 0, opt_d = 0;
    char *port = "80", *output = NULL, *directory = NULL;

    while ((c = getopt(argc, argv, "p:o:d:")) != -1){
        switch (c)
        {
        case 'p':
            if (opt_p != 0){
                usage();
                fprintf(stderr, "ERROR: reading optional arguments\n");
                exit(EXIT_FAILURE);
            }
            opt_p = 1;
            port = optarg;
            break;
        case 'o':
            if (opt_o != 0 || opt_d != 0){
                usage();
                fprintf(stderr, "ERROR: reading optional arguments\n");
                exit(EXIT_FAILURE);
            }
            opt_o = 1;
            output = optarg;
            break;
        case 'd':
            if (opt_o != 0 || opt_d != 0){
                usage();
                fprintf(stderr, "ERROR: reading optional arguments\n");
                exit(EXIT_FAILURE);
            }
            opt_d = 1;
            directory = optarg;
            break;
        case '?':
            usage();
            fprintf(stderr, "ERROR: wrong optional arguments\n");
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    if (optind != argc - 1){
        fprintf(stderr, "ERROR: reading positional arguments\n");
        exit(EXIT_FAILURE);
    }
    char *url = argv[optind];
    char *host = malloc(strlen(url) * sizeof(*url));
    char *filepath = malloc(strlen(url) * sizeof(*url));

    if (parseUrl(host, filepath, url) == -1){
        free(host);
        free(filepath);
        fprintf(stderr, "ERROR: parsing input URL\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int res = getaddrinfo(host, port, &hints, &ai);

    if (res != 0){
        freeaddrinfo(ai);
        free(host);
        free(filepath);
        fprintf(stderr, "ERROR: running getaddrinfo()\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);


    if (sockfd < 0){
        freeaddrinfo(ai);
        free(host);
        free(filepath);
        fprintf(stderr, "ERROR: running socket()\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0){
        freeaddrinfo(ai);
        free(host);
        free(filepath);
        fprintf(stderr, "ERROR: running connect()\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(ai);

    FILE *sockFile = fdopen(sockfd, "r+");
    if (sockFile == NULL){
        free(host);
        free(filepath);
        fprintf(stderr, "ERROR: opening socket file\n");
        exit(EXIT_FAILURE);
    }
    FILE *outputFile = openOutput(output, directory, filepath);
    if (outputFile == NULL){
        free(host);
        free(filepath);
        fclose(sockFile);
        fprintf(stderr, "ERROR: opening output file\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fprintf(sockFile, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", filepath, host);
    fflush(sockFile);

    char *line = NULL;
    size_t n;

    getline(&line, &n, sockFile);
    if (line == NULL || strncmp(line, "HTTP/1.1", strlen("HTTP/1.1")) != 0){
        free(host);
        free(filepath);
        fclose(outputFile);
        fclose(sockFile);
        free(line);
        fprintf(stderr, "Protocol Error!\n");
        exit(2);
    }
    if (strncmp(&line[9], "200", strlen("200")) != 0){
        free(host);
        free(filepath);
        fclose(outputFile);
        fclose(sockFile);
        free(line);
        fprintf(stderr, "%sProtocol Error!\n", &line[9]);
        exit(3);
    }

    while (getline(&line, &n, sockFile)){
        if (strcmp(line, "\r\n") == 0){
            break;
        }
    }
    while (getline(&line, &n, sockFile) != -1){
        fputs(line, outputFile);
    }

    free(host);
    free(filepath);
    fclose(outputFile);
    fclose(sockFile);
    free(line);
    exit(EXIT_SUCCESS);
}
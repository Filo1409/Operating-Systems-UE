#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

void usage(){
    printf("Usage: mydiff [-i] [-o outfile] file1 file2\n");
    fprintf(stderr, "ERROR\n");
    exit(1);
}

int getDifferences(char *string1, char *string2, int n, int caseSens){
    int diff = 0;

    if (caseSens != 0){
        for (int i = 0; i < n; i++){
            string1[i] = tolower(string1[i]);
            string2[i] = tolower(string2[i]);
        }
    }

    for (int i = 0; i < n; i++){
        if (string1[i] != string2[i]){
            diff++;
        }
    }
    return diff;
}

int computeSolution(int caseSens, char *output, char *input1, char *input2){
    size_t n1 = 0;
    char *line1 = NULL;
    size_t n2 = 0;
    char *line2 = NULL;
    int n = 0;

    FILE *inputf1 = fopen(input1, "r");
    if (inputf1 == NULL){
        return -1;
    }
    FILE *inputf2 = fopen(input2, "r");
    if (inputf2 == NULL){
        return -1;
    }
    FILE *outputf;
    if (output == NULL){
        outputf = stdout;
    } else {
        outputf = fopen(output, "w");
    }
    if (outputf == NULL){
        return -1;
    }
    while (getline(&line1, &n1, inputf1) != -1 && getline(&line2, &n2, inputf2) != -1){
        n = strlen(line1) - 1 < strlen(line2) - 1 ? strlen(line1) - 1 : strlen(line2) - 1;
        printf("%ld = strlen1, %ld = strlen2\n", strlen(line1), strlen(line2));
        if (caseSens == 0){
            if (strncmp(line1, line2, n) != 0){
                printf("%s", line1);
                printf("%s", line2);
                // fprintf(outputf, "The lines aren't the same\ndifferences = %d\n", getDifferences(line1, line2, n, caseSens));
                char *diff = malloc(1024);
                sprintf(diff, "The lines aren't the same\ndifferences = %d\n", getDifferences(line1, line2, n, caseSens));
                fputs(diff, outputf);
            }
        } else {
            if (strncasecmp(line1, line2, n) != 0){
                printf("%s", line1);
                printf("%s", line2);
                // fprintf(outputf, "The lines aren't the same\ndifferences = %d\n", getDifferences(line1, line2, n, caseSens));
                char *diff = malloc(1024);
                sprintf(diff, "The lines aren't the same\ndifferences = %d\n", getDifferences(line1, line2, n, caseSens));
                fputs(diff, outputf);
            }
        }
    }
    if (fclose(inputf1) != 0){
        return -1;
    }
    if (fclose(inputf2) != 0){
        return -1;
    }
    if (outputf != stdout){
        if (fclose(outputf) != 0){
            return -1;
        }
    }
    return 0;
}

int main (int argc, char *argv[]){
    int c;
    int opt_i = 0;
    int opt_o = 0;
    char *output = NULL;
    char *input1;
    char *input2;
    
    while ((c = getopt(argc, argv, "io:")) != -1){
        switch(c){
            case 'i':
                if (opt_i != 0){
                    usage();
                }
                opt_i = 1;
                break;
            case 'o':
                if (opt_o != 0){
                    usage();
                }
                opt_o = 1;
                output = optarg;
                break;
            case '?':
                usage();
                break;
        }
    }

    if (argc - optind != 2){
        usage();
    }
    input1 = argv[optind];
    input2 = argv[optind + 1];

    if (computeSolution(opt_i, output, input1, input2) == -1){
        usage();
    }

    exit(0);
}


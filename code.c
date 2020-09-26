#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "StringSort.h"

enum FILE_TYPE
{
    UTF_8,
    UTF_16,
};

typedef enum PARAM_TYPE
{
    PARAM_HELP,
    PARAM_OPEN,
    PARAM_NOT_EXIST
}PARAM_TYPE;

char *str_to_lower(char *str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }
    return str;
}

PARAM_TYPE get_param_type(const char *param)
{
    assert(param);
    bool long_param = param[1] == '-';
    param = (long_param) ? param + 2 : param + 1;
    int len = strlen(param);
    if (!long_param) {
        if (len > 1) { return PARAM_NOT_EXIST; }
        switch (param[0]) {
        case 'H':
        case 'h': 
            return PARAM_HELP;
        case 'O':
        case 'o':
            return PARAM_OPEN;
        }
    } else {
        if (!strcmp(param, "help")) { return PARAM_HELP; }
        if (!strcmp(param, "open")) { return PARAM_OPEN; }
    }
    return PARAM_NOT_EXIST;
}

unsigned char *map_file_in_memory(FILE *file, size_t *mem_size)
{
    int ok = 0;
    long size = 0;
    unsigned char *mem = NULL;
    ok = fseek(file, 0, SEEK_END);
    if (ok) { return NULL; }
    size = ftell(file);
    if (size < 0) { return NULL; }
    rewind(file);
    
    #if (LONG_MAX >= SIZE_MAX)
    if (size >= SIZE_MAX) {
        return NULL;
    }
    #endif
    *mem_size = size + 1;
    mem = (unsigned char*)calloc(mem_size, sizeof(*mem));
    if (!mem) { return NULL; }
    
    fread(mem, sizeof(*mem), size, file);
    if (ferror(file)) {
        free(mem);
        return NULL;
    }

    mem[size] = '\n';//
    return mem;
}

void print_help()
{
    printf("[-o | --open] %%FileName%%  :  open file for sort string\n");
    printf("[-h | --help]  :  print that\n");
}

int main(int argc, char *argv[]) 
{
    FILE *file = NULL;
    unsigned char *mem = NULL;
    size_t mem_size = 0;

    if (argc < 2) {
        print_help();
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (!argv[i]) { break; }
        if (argv[i][0] != '-') { continue; }
        switch (get_param_type(str_to_lower(argv[i]))) {
        case PARAM_OPEN:
            file = fopen(argv[i + 1], "rb");
            if (!file) {
                printf("File not exist or can't be opened now\n");
            }
            break;
        case PARAM_HELP:
            print_help();
            break;
        case PARAM_NOT_EXIST:
            printf("Param not exist: %s\nFor help start program with: -h\n", argv[i]);
            break;
        }
    }

    if (!file) {
        printf("File not set\nFor set file strat program with: -o %%FileName%%\n");
        return 0;
    }

    mem = map_file_in_memory(file, &mem_size);
    fclose(file);
    if (!mem) {
        printf("an error occurred while mapping file to memory\n");
        return 0;
    }
    
    int b = TODO_name(mem, mem_size);
    printf("hmm... is %d\n", b);

    return 0;
}
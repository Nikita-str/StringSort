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
    PARAM_INVERT,
    PARAM_FILE_OUT,
    PARAM_TYPE_OF_FILE,
    PARAM_NOT_EXIST
}PARAM_TYPE;

typedef enum FILE_FOR_TYPE
{
    FILE_FOR_START = 0b1,
    FILE_FOR_END = 0b10,
    FILE_FOR_PRIMARY = 0b100,
    NOT_FILE_FOR = 0,
}FILE_FOR_TYPE;

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
        if (len > 2) { return PARAM_NOT_EXIST; }
        switch (param[0]) {
        case 'h': 
            return PARAM_HELP;
        case 'o':
            return PARAM_OPEN;
        case 'i':
            return PARAM_INVERT;
        case 'f':
            if (param[1] == 'o') return PARAM_FILE_OUT;
        case 't':
            return PARAM_TYPE_OF_FILE;
        }
    } else {
        if (!strcmp(param, "help")) { return PARAM_HELP; }
        if (!strcmp(param, "open")) { return PARAM_OPEN; }
        if (!strcmp(param, "invert")) { return PARAM_INVERT; }
        if (!strcmp(param, "file_out")) { return PARAM_FILE_OUT; }
        if (!strcmp(param, "type_of_file")) { return PARAM_TYPE_OF_FILE; }
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
    printf("[-i | --invert]  :  invert line output order\n");
    printf("[-h | --help]  :  print that\n");
    //TODO:
    printf("[-fo | --file_out] [start | end | primary] %%FileOutName%%  :  file for appropriate output\n"
           "     by default: -file_out start out_by_start.txt\n"
           "                 -file_out end out_by_end.txt\n"
           "                 -file_out primary out_primordial.txt\n");
    //? : printf("[-c | --encoding] [utf8 | utf32] :  what encoding of in file\n");
    printf("[-t | --type_of_out] [s &| e &| p] :  what files to output\n"
           "    s - for output with sorting by {s}tart line order\n"
           "    e - for output with sorting by {e}nd line order\n"
           "    p - for output with sorting by {p}rimordial line order\n"
           "    by default: -t sep\n");
}

bool check_file_name(const char *file_now)
{
    assert(file_now);
    for (int i = 0; file_now[i] != '\0'; i++) {
        char c = file_now[i];
        if (!(('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || 
              ('A' <= c && c <= 'Z') || c == '.' || c == '_' || c == '#' || c == '\\' || c == '/' || c == ':')) { return false; }
    }
    return true;
}

bool check_file_busy(const char *file_now, const char *files[])
{
    assert(file_now);
    assert(files);
    for (int i = 0; files[i] != NULL; i++) {
        if (!strcmp(file_now, files[i])) { return false; }
    }
    return true;
}

FILE_FOR_TYPE get_file_for_type(const char *param)
{
    if (!strcmp(param, "start")) { return FILE_FOR_START; }
    if (!strcmp(param, "end")) { return FILE_FOR_END; }
    if (!strcmp(param, "primary")) { return FILE_FOR_PRIMARY; }
    return NOT_FILE_FOR;
}

uint32_t get_file_out_flags(const char *param)
{
    uint32_t ret = 0;
    for (int i = 0; param[i] != '\0'; i++) {
        char c = param[i];
        if (c == 's') ret = ret | FILE_FOR_START;
        if (c == 'e' || c == 'u') ret = ret | FILE_FOR_END;
        if (c == 'p') ret = ret | FILE_FOR_PRIMARY;
    }
    return ret;
}

int main(int argc, char *argv[])
{
    FILE *file = NULL;
    unsigned char *mem = NULL;
    size_t mem_size = 0;
    bool invert_order = false;
    COMPARE_BY cmp_by = COMPARE_BY_START_OF_LINE;
    uint32_t file_out_flags = FILE_FOR_START | FILE_FOR_END | FILE_FOR_PRIMARY;

    if (argc < 2) {
        print_help();
        return 0;
    }

    const char *file_open_name = NULL;
    const char *file_out_sort_by_start = "out_by_start.txt";
    const char *file_out_sort_by_end = "out_by_end.txt";
    const char *file_out_primary = "out_primordial.txt";
    const char *files[] = {file_open_name, file_out_sort_by_start, file_out_sort_by_end, file_out_primary, NULL};

    for (int i = 1; i < argc; i++) {
        if (!argv[i]) { break; }
        if (argv[i][0] != '-') { continue; }
        switch (get_param_type(str_to_lower(argv[i]))) {
        case PARAM_OPEN:
            if (file) {
                fclose(file);
            }
            if (argc - i < 2) {
                printf("not enough params for %s\n", argv[i]);
                break;
            }
            const char *f_name = argv[i + 1];
            if (!check_file_name(f_name)) {
                printf("Bad file name: %s\n", f_name);
                break;
            }
            if (!check_file_busy(f_name, files)) {
                printf("file will already be used: %s", f_name);
                break;
            }
            file_open_name = f_name;
            file = fopen(f_name, "rb");
            if (!file) {
                printf("File not exist or can't be opened now\n");
            }
            i += 1;
            break;
        case PARAM_HELP:
            print_help();
            break;
        case PARAM_INVERT:
            invert_order = true; // inv_ord = !inv_ord; ?
            break;
        case PARAM_FILE_OUT:
        {
            if (argc - i < 3) {
                printf("not enough params for %s\n", argv[i]);
                break;
            }
            FILE_FOR_TYPE type_ff = get_file_for_type(str_to_lower(argv[i + 1]));
            if (type_ff == NOT_FILE_FOR) {
                printf("first param for %s can be only 'start', 'end' or 'primary'\n", argv[i]);
                break;
            }

            const char *f_name = argv[i + 2];
            if (!check_file_name(f_name)) {
                printf("Bad file name: %s", f_name);
                break;
            }
            if (!check_file_busy(f_name, files)) {
                printf("file will already be used: %s", f_name);
                break;
            }

            switch (type_ff) {
            case FILE_FOR_START:
                file_out_sort_by_start = f_name;
                break;
            case FILE_FOR_END:
                file_out_sort_by_end = f_name;
                break;
            case FILE_FOR_PRIMARY:
                file_out_primary = f_name;
                break;
            default: assert(0);  break;
            }
            break;

            i += 2;
        }

        case PARAM_TYPE_OF_FILE:
        {
            if (argc - i < 2) {
                printf("not enough params for %s\n", argv[i]);
                break;
            }
            file_out_flags = get_file_out_flags(argv[i + 1]);
            break;
        }

        case PARAM_NOT_EXIST:
            printf("Param not exist: %s\nFor help start program with: -h\n", argv[i]);
            break;
        default:
            printf("parameter was not processed\n");
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
    
    int b = TODO_name(mem, mem_size, invert_order, files + 1, file_out_flags);
    if (!b) {
        printf("something went wrong  ://  \n");
    } else {
        printf("program ended successfully\n");
    }

    return 0;
}
#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

typedef struct line_ptr
{
    uint8_t *ptr;//<ptr on line
    size_t len; //<size of line 
}line_ptr;


enum GET_CHAR_BAD_RET
{
    GET_CHAR_LAST_VALID_VALUE = 0xF00000 - 1,
    GET_CHAR_INVALID_STRUCT = 0xF00000,
    GET_CHAR_INVALID_VALUE = 0xF00001,
    GET_CHAR_TOO_LONG_VALUE = 0xF00002,
    GET_CHAR_UNEXPECTED_ERROR = 0xF0000F,
    GET_CHAR_OTHER_ERROR = 0xF000FF,
};

enum { UTF_8_MAX_BYTE = 4 };
enum { UTF_8_BYTE_BIT = 8 }; // CHAR_BIT  |  but for sure  |  >_<
enum { UTF_8_SHIFT_DOP_BLOCK = 1<<6 };//10_xxxxxx

typedef enum COMPARE_BY
{
    COMPARE_BY_START_OF_LINE = 0,
    COMPARE_BY_END_OF_LINE = 1,
}COMPARE_BY;

extern uint32_t get_char_UTF_8(const uint8_t *now_char, size_t *byte_readed, int max_byte_read, COMPARE_BY cmp_by);

typedef enum LINE_CMP_RES
{
    LINE_CMP_LESS = -1,
    LINE_CMP_EQUAL = 0,
    LINE_CMP_MORE = 1,
    LINE_CMP_ERROR = 0xE,

    LINE_CMP_SPEC_VALUE_OK = 0x1F,
}LINE_CMP_RES;

//TODO:extern

extern bool line_sort(line_ptr *sort_arr, size_t size, bool invert_order, COMPARE_BY cmp_by);

extern bool TODO_name(const uint8_t *mem, size_t mem_size, bool invert_order, const char *files_sep[], uint32_t files_flag);//TODO : name


#include "StringSort.h"


//+++######################## STRUCT: arr_line_ptr
static struct arr_line_ptr
{
    line_ptr *arr;
    size_t size;
    size_t _max_size;
};
typedef struct arr_line_ptr arr_line_ptr;

static arr_line_ptr arr_line_ptr_new()
{
    arr_line_ptr alp = {0, 0, 0};
    alp._max_size = 4;
    alp.arr = (line_ptr *)calloc(alp._max_size, sizeof(line_ptr));
    return alp;
}

static void arr_line_ptr_add(arr_line_ptr *ptr, line_ptr line)
{
    if (ptr->size == ptr->_max_size) {
        ptr->_max_size *= 2;
        ptr->arr = (line_ptr *)realloc(ptr->arr, sizeof(line_ptr) * ptr->_max_size);
        if (!ptr->arr) { exit(1); }
    }
    ptr->arr[ptr->size++] = line;
}

static void arr_line_ptr_free(arr_line_ptr *ptr)
{
    free(ptr->arr);
}
//---######################## STRUCT: arr_line_ptr

static arr_line_ptr get_line_ptrs(const uint8_t *mem, size_t mem_size)//TODO:UTF-8 NOW!!!
{
    //!! we have guarantee that end line will be ended with '\n'
    arr_line_ptr alp = arr_line_ptr_new();
    line_ptr line = {NULL, 0};//{mem, 0};
    size_t space_combo = 0; //< cause we are not interesting in end spaces
    bool first_space = true;//<first spaces also not needed
    for (size_t i = 0; i < mem_size; i++) {
        uint8_t c = mem[i];
        if ((c == '\n') && (line.len != 0)) {
            arr_line_ptr_add(&alp, line);
            line.ptr = NULL;//mem + i + 1;
            line.len = 0;
            first_space = true;
            space_combo = 0;
        } else if (isspace(c)) {
            if (!first_space) { space_combo++; }
        } else {
            line.len += space_combo + 1;
            space_combo = 0;
            if (first_space) {
                line.ptr = mem + i;
                first_space = false;
            }
        }
    }
    return alp;
}

bool is_skip_symb_UTF_8(uint32_t c)
{
    return (c < 0x30) || (0x39 < c && c < 0x41) || 
           (0x5A < c && c < 0x61) || (0x7A < c && c < 0x8F);
}

uint32_t get_char_UTF_8(const uint8_t *now_char, size_t *byte_readed, int max_byte_read, COMPARE_BY cmp_by)//TODO:UTF_8 NOW!!!!
{
    const uint8_t MASK_2_BYTE = 0xC0; //110x_xxxx
    const uint8_t MASK_3_BYTE = 0xE0; //1110_xxxx
    const uint8_t MASK_4_BYTE = 0xF0; //1111_0xxx

    const uint8_t MASK_LAST_2_BIT = 0xC0; //1100_0000
    const uint8_t MASK_DOP_BIT = 0x80; //1000_0000

    const uint8_t MASK_LAST_3_BIT = 0xE0; //1110_0000
    const uint8_t MASK_LAST_4_BIT = 0xF0; //1111_0000
    const uint8_t MASK_LAST_5_BIT = 0xF8; //1111_1000

    const uint8_t MASK_FIRST_6_BIT = 0x3F; //0011_1111
    const uint8_t MASK_FIRST_5_BIT = 0x1F; //0001_1111
    const uint8_t MASK_FIRST_4_BIT = 0x0F; //0000_1111
    const uint8_t MASK_FIRST_3_BIT = 0x07; //0000_0111

    assert(now_char);
    assert(byte_readed);
    assert(0 < max_byte_read && max_byte_read <= UTF_8_MAX_BYTE);
    assert(cmp_by == COMPARE_BY_END_OF_LINE || cmp_by == COMPARE_BY_START_OF_LINE);

    *byte_readed = 0;
    int numb_of_byte = 0;
    uint32_t ret = 0;
    uint8_t byte = now_char[0];

    uint32_t now_shift = 1;//for sort_by == SORT_BY_END_OF_LINE

    if (cmp_by == COMPARE_BY_END_OF_LINE) {
        int i_back = 0;// !! i_back >= 0
        while ((byte & MASK_LAST_2_BIT) == MASK_DOP_BIT) {
            if ((max_byte_read - 1) <= i_back) { //if == => after will be != 10_xxxxxx => too long
                return GET_CHAR_TOO_LONG_VALUE;
            }

            ret = ret + (byte & MASK_FIRST_6_BIT) * now_shift;
            now_shift *= UTF_8_SHIFT_DOP_BLOCK;

            i_back++;
            if (i_back == UTF_8_MAX_BYTE - 1) { //if == => after will be != 10_xxxxxx => invalid structure (but may be not too long)
                return GET_CHAR_INVALID_STRUCT;
            }
            byte = now_char[-i_back]; // *(now_char - i_back)
        }
        numb_of_byte = i_back + 1;
    }

    //read first byte
    if (byte >> (UTF_8_BYTE_BIT - 1)) {
        if ((byte & MASK_LAST_3_BIT) == MASK_2_BYTE) {
            numb_of_byte = 2;
            byte = byte & MASK_FIRST_5_BIT;
        } else if ((byte & MASK_LAST_4_BIT) == MASK_3_BYTE) {
            numb_of_byte = 3;
            byte = byte & MASK_FIRST_4_BIT;
        } else if ((byte & MASK_LAST_5_BIT) == MASK_4_BYTE) {
            numb_of_byte = 4;
            byte = byte & MASK_FIRST_3_BIT;
        } else {
            return GET_CHAR_INVALID_STRUCT;
        }
    } else { // 0xxx_xxxx
        if (cmp_by == COMPARE_BY_END_OF_LINE && numb_of_byte != 1) {
            return GET_CHAR_INVALID_STRUCT;
        }
        *byte_readed = 1;
        return byte;
    }

    if (cmp_by == COMPARE_BY_START_OF_LINE) {
        ret = byte;
    } else if (cmp_by == COMPARE_BY_END_OF_LINE) {
        if (numb_of_byte != numb_of_byte) {
            return GET_CHAR_INVALID_STRUCT;
        }
        ret = ret + (byte & MASK_FIRST_6_BIT) * now_shift;
    }

    if (numb_of_byte > max_byte_read) {
        return GET_CHAR_TOO_LONG_VALUE;
    }

    if (cmp_by == COMPARE_BY_START_OF_LINE) {
        //read other bytes
        for (int i = 1; i < numb_of_byte; i++) {
            byte = now_char[i];
            if ((byte & MASK_LAST_2_BIT) != MASK_DOP_BIT) {
                return GET_CHAR_INVALID_STRUCT;
            }
            byte = byte & MASK_FIRST_6_BIT;
            ret = (ret << 6) + byte;
        }
    }

    //check diaposone:
    switch (numb_of_byte) {
    case 2:
        if (ret < 0x0080 || ret > 0x07FF) { return GET_CHAR_INVALID_VALUE; } 
        break;
    case 3:
        if (ret < 0x0800 || ret > 0xFFFF) { return GET_CHAR_INVALID_VALUE; }
        if (ret > 0xD7FF && ret < 0xE000) { return GET_CHAR_INVALID_VALUE; }
        break;
    case 4:
        if (ret < 0x10000 || ret > 0x10FFFF) { return GET_CHAR_INVALID_VALUE; }
        break;
    default:
        assert(0);
        return GET_CHAR_UNEXPECTED_ERROR;
    }

    *byte_readed = numb_of_byte;
    return ret;
}

static inline uint32_t line_cmp_UTF_8_get_next_symb(const line_ptr *line, const uint8_t *ptr, size_t *len_now, bool *end, COMPARE_BY cmp_by)
{
    size_t bytes_readed = 0;
    uint32_t c = 0;
    while (!*end) {
        int max_byte_read = line->len  - *len_now;
        max_byte_read = (max_byte_read >= UTF_8_MAX_BYTE) ? UTF_8_MAX_BYTE : max_byte_read;
        const uint8_t *ptr_char = (cmp_by == COMPARE_BY_START_OF_LINE) ? 
            ptr + (*len_now) :
            ptr + (line->len - *len_now - 1); //WARNING: if 3 item will added to cmp_by then it ERROR

        c = get_char_UTF_8(ptr_char, &bytes_readed, max_byte_read, cmp_by);
        *len_now += bytes_readed;
        if (!bytes_readed) { 
            assert(GET_CHAR_LAST_VALID_VALUE < c);  
            return c; 
        }
        if (*len_now > line->len) { return GET_CHAR_INVALID_STRUCT; }
        //if (cmp_by == COMPARE_BY_START_OF_LINE) { ptr += bytes_readed; }
        if (!is_skip_symb_UTF_8(c)) {
            break;
        }
        if (*len_now == line->len) { *end = true; }
    }
    return c;
}

LINE_CMP_RES line_cmp_UTF_8(const line_ptr *l1, const line_ptr *l2, bool invert_order, COMPARE_BY cmp_by)//TODO:UTF_8 NOW!!!!
{
    size_t len_now_1 = 0;
    size_t len_now_2 = 0;

    const uint8_t *ptr_1 = l1->ptr;
    const uint8_t *ptr_2 = l2->ptr;

    bool l1_end = len_now_1 == l1->len;
    bool l2_end = len_now_2 == l2->len;

    while (!l1_end && !l2_end) {
        uint32_t c1 = line_cmp_UTF_8_get_next_symb(l1, ptr_1, &len_now_1, &l1_end, cmp_by);
        uint32_t c2 = line_cmp_UTF_8_get_next_symb(l2, ptr_2, &len_now_2, &l2_end, cmp_by);
        if (GET_CHAR_LAST_VALID_VALUE < c1 || GET_CHAR_LAST_VALID_VALUE < c2) {
            return LINE_CMP_ERROR;
        }

        if (l1_end || l2_end) { break; }

        //it really need cause c1/c2 can be last char, but not skipped symbol
        l1_end = len_now_1 == l1->len; 
        l2_end = len_now_2 == l2->len;

        if (c1 != c2) {
            return ((c1 < c2) ^ invert_order) ? LINE_CMP_LESS : LINE_CMP_MORE;
        }
    }

    if (l1_end && l2_end) {
        return LINE_CMP_EQUAL;
    } else if (l1_end) {
        return invert_order ? LINE_CMP_MORE : LINE_CMP_LESS;
    } else if (l2_end) {
        return invert_order ? LINE_CMP_LESS : LINE_CMP_MORE; 
    }

    assert(0);
    return LINE_CMP_ERROR;
}

bool line_sort(line_ptr *sort_arr, size_t size, bool invert_order, COMPARE_BY cmp_by)
{
    LINE_CMP_RES res = LINE_CMP_EQUAL;

    switch (size) {
    case 0: 
    case 1:
        return true;
    case 2:
        res = line_cmp_UTF_8(sort_arr + 0, sort_arr + 1, invert_order, cmp_by);
        if (res == LINE_CMP_ERROR) {
            return false;
        } else if (res == LINE_CMP_MORE) {

            line_ptr line_save = sort_arr[0];
            sort_arr[0] = sort_arr[1];
            sort_arr[1] = line_save;
        }
        return true;
    }

    bool ok = true;

    size_t mid = size >> 1;
    size_t left = 0;
    size_t right = size - 1;

    line_ptr piv = sort_arr[mid];

    while (left <= right) {
        while ((res = line_cmp_UTF_8(sort_arr + left, &piv, invert_order, cmp_by)) == LINE_CMP_LESS) { left++; }
        if (res == LINE_CMP_ERROR) { return false; }

        static int JUST_FOR_GDB = 0;
        JUST_FOR_GDB++;
        while ((res = line_cmp_UTF_8(sort_arr + right, &piv, invert_order, cmp_by)) == LINE_CMP_MORE) { right--; }
        if (res == LINE_CMP_ERROR) { return false; }

        if (left >= right) { break; }

        line_ptr line_save = sort_arr[left];
        sort_arr[left++] = sort_arr[right];
        sort_arr[right--] = line_save;
    }

    ok = ok && line_sort(sort_arr, right + 1, invert_order, cmp_by);
    ok = ok && line_sort(sort_arr + right + 1, size - right - 1, invert_order, cmp_by);

    return ok;
}

bool print_line_in_file(FILE *file, line_ptr *lines, size_t line_count)
{
    for (size_t i = 0; i < line_count; i++) {
        fwrite(lines[i].ptr, sizeof(*lines[i].ptr), lines[i].len, file);
        fwrite("\n", 1, 1, file);
        //TODO:return false when error
    }
    return true;
}

bool TODO_name(const uint8_t *mem, size_t mem_size, bool invert_order, const char *files_sep[], uint32_t files_flag)
{
    int ok = true;
    FILE *file = NULL;

    arr_line_ptr alp = get_line_ptrs(mem, mem_size);

    for (int i = 0; i < 2; i++) {
        if ((files_flag & (1 << i)) == 0) continue;
        COMPARE_BY cmp_by = i;
        ok = line_sort(alp.arr, alp.size, invert_order, cmp_by);
        if (!ok) { goto BAD_EXIT; }

        file = fopen(files_sep[i], "wb");
        if (!file) { goto BAD_EXIT; }

        ok = print_line_in_file(file, alp.arr, alp.size);
        if (!ok) {
            fclose(file);
            goto BAD_EXIT;
        }
        ok = fclose(file);
        if (ok != 0) { goto BAD_EXIT; }
    }

    if (files_flag & 0b100) {
        file = fopen(files_sep[2], "wb");
        if (!file) { goto BAD_EXIT; }

        size_t size = fwrite(mem, sizeof(*mem), mem_size, file);
        if (size != mem_size) {
            fclose(file);
            goto BAD_EXIT;
        }
        ok = fclose(file);
        if (ok != 0) { goto BAD_EXIT; }
    }

    arr_line_ptr_free(&alp);
    return true;

    BAD_EXIT:
    arr_line_ptr_free(&alp);
    return false;
}
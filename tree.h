// tree.h - the header file for functions related to huffman-coding trees


/*
 * a struct to represent a the huffman code. The struct is necessary
 * because 001 and 0001 (for example) have the same integer representation
 */
typedef struct {
    int value;
    int bits;
} *CODE, code;

TREE init_tree(int type, char zero_value, char one_value, TREE zero, TREE one);

void merge(PTABLE pt);

TREE huffman(PTABLE pt);

void tree_to_table(TREE tr, CODE ctable);

void print_tree(TREE tr);

int *string_to_code(char *str);

void print_bin_int(int in);

int *encode(int* dest, char* str, CODE ctable);
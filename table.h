// table.h - header file for character frequency table

#include <stdlib.h>


typedef struct tree {
    char zero_value;
    char one_value;
    int type;
    /*
     * type = 0: leaf leaf
     * type = 1: node leaf
     * type = 2: leaf node
     * type = 3: node node
    */
    struct tree *zero;
    struct tree *one;
} *TREE, tree;

/*
 * an entry in ptable. Includes the chracter, the probability
 * associated with that character, and a tree pointer to store
 * the code tree built up so far.
 */
typedef struct {
    char key;
    double prob;
    TREE tr;
} *ENTRY, entry;


typedef struct {
    ENTRY entries;
    int size;
} *PTABLE, ptable;

ENTRY init_entry(char key, double prob, TREE tr);

void destroy_tree(TREE tr);

void destroy_entry(ENTRY et);

void print_ptable(PTABLE pt);

PTABLE str_to_ptable(char *str);

void ptable_pop(PTABLE pt);

void ptable_add(PTABLE pt, ENTRY et);

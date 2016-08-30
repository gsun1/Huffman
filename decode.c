#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "table.h"
#include "tree.h"
#include "decode.h"

#define TSIZE 128

/*
 * entry_helper(source,co,ch) - Takes a tree source, a code co, and a character ch
 * and uses it to build up just enough of the tree so that the path to the character
 * ch can be added to the tree.
 */
static void entry_helper(TREE source, code co, char ch) {
    TREE rover = source;
    assert(source != NULL);
    int value = co.value;
    int bits = co.bits;
    //printf("%d\n",value);
    int bit;
    for (int i = bits; i > 1; --i) {
        bit = (value >> (i-1)) & 1;
        //printf("%d\n",bit);
        if (bit == 0) {
            if (rover->zero == NULL) {
                //printf("Rover zero null\n");
                TREE new = (TREE)malloc(sizeof(tree));
                new->one = NULL;
                new->zero = NULL;
                if (rover->one == NULL) {
                    rover->type = 1;
                }
                else {
                    rover->type = 3;
                }
                rover->zero = new;
            }
            rover = rover->zero;
        }
        else {
            if (rover->one == NULL) {
                //printf("Rover one null\n");
                TREE new = (TREE)malloc(sizeof(tree));
                new->one = NULL;
                new->zero = NULL;
                if (rover->zero == NULL) {
                    rover->type = 2;
                }
                else {
                    rover->type = 3;
                }
                rover->one = new;
            }
            rover = rover->one;
        }
    }
    bit = value&1;
    if (bit == 0) {
        rover->zero_value = ch;
        if (rover->one == NULL) {
            rover->type = 0;
        }
        else {
            rover->type = 2;
        }
    }
    else {
        rover->one_value = ch;
        if (rover->zero == NULL) {
            rover->type = 0;
        }
        else {
            rover->type = 1;
        }
    }


}

/*
 * table_to_tree(ctable) - takes a ctable (which is the right data structure
 * for encoding and is more compact) and returns the corresponding tree
 * (which is the right data structure for decoding).
 */
TREE table_to_tree(CODE ctable) {
    TREE result = init_tree(-1,'\0','\0',NULL,NULL);
    for (char c = 0; c < TSIZE-1; c++) {
        code co = ctable[(int)c];
        //printf("%c: %d,%d\n",c,co.value,co.bits);
        if (co.bits != 0) {
            //printf("%c\n",c);
            entry_helper(result,co,c);
        }
    }
    return result;
}

/*
 * push_char(c,str,size) - pushes a new character c into
 * string str of size size
 */
static char *push_char(char c, char *str, int *size) {
    //printf("Pushing %c\n",c);
    str = (char*)realloc(str,((*size)+1) * sizeof(char));
    str[*size] = c;
    return str;
}

/*
 * decode_helper(str,top,current,word,size) - takes a string str of sizes size and adds as
 * many characters as possible based on the current location in the tree and the next int
 * to read, word. A pointer top points to the top of the tree, and current is used to keep
 * track of where in the tree is currently being traversed.
 */
static char *decode_helper(char* str, TREE top, TREE *current, int word, int* size) {
    //printf("Entering helper\n");
    for(int i = 31; i >= 0; i--) {
        //printf("entering loop\n");
        if ((*size) != 0 && str[(*size) - 1] == '\0') {
            //printf("Finished\n");
            return str;
        }
        int bit = (word >> i) & 1;
        //printf("%d\n",bit);
        if(bit == 0) {
            //printf("bit zero\n");
            if ((*current)->zero != NULL) {
                *current = (*current)->zero;
            }
            else {
                char c = (*current)->zero_value;
                str = push_char(c,str,size);
                int tmp = *size + 1;
                *size = tmp;
                //printf("%d\n",tmp);
                *current = top;
            }
        }
        else {
            //printf("bit one\n");
            if ((*current)->one != NULL) {
                *current = (*current)->one;
            }
            else {
                char c = (*current)->one_value;
                str = push_char(c,str,size);
                int tmp = *size + 1;
                *size = tmp;
                //printf("%d\n",tmp);
                *current = top;
            }
        }
    }
    return str;
}

/*
 * decode(key,code) - uses a tree to decode the int array pointed to by
 * code and return the corresponding string.
 */
char *decode(TREE key, int *code) {
    //printf("Here?");
    char *result = (char*)malloc(sizeof(char));
    //printf("Entering\n");
    TREE tmp = key;
    TREE *current = &tmp;
    //printf("Passed\n");
    int *size = (int*)malloc(sizeof(int));
    *size = 0;
    for(int i = 0;;i++) {
        //printf("%d\n",code[i]);
        result = decode_helper(result,key,current,code[i],size);
        
        if (result[(*size)-1] == '\0') {
            return result;
        }
    }
    return NULL;
}

/*int main() {
    char *wood = "We conjure the spirits of the computer with our spells";
    printf("String to encode:\n%s\n",wood);
    PTABLE pt = str_to_ptable(wood);
    TREE tr = huffman(pt);
    CODE ctable = (CODE)calloc(TSIZE, sizeof(code));
    tree_to_table(tr,0,ctable,1);
    for(char c = 0; c < 127; ++c) {
        printf("%c: %d,%d\n",c,ctable[(int)c].value,ctable[(int)c].bits);
    }
    //print_tree(tr,"",1);
    tr = table_to_tree(ctable);
    int* code = string_to_code(wood);
    printf("The code:\n");
    print_tree(tr,"",1);
    char *result = decode(tr,code);
    printf("Encoded string:\n");
    for(int i = 0; code[i] != 0; ++i) {
        print_bin_int(code[i]);
    }
    printf("\nDecoded:\n");
    printf("%s\n",result);
    exit(0);
}*/
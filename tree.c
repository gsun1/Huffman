#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "table.h"
#include "tree.h"

#define TSIZE 128
#define INT_BITS 32

/*
 * init_tree(type,zer_value,one_value,zero,one) - initialize a tree based
 * on the input values
 */
TREE init_tree(int type, char zero_value, char one_value, TREE zero, TREE one) {
    TREE tr = (TREE)malloc(sizeof(tree));
    tr->type = type;
    tr->zero_value = zero_value;
    tr->one_value = one_value;
    tr->zero = zero;
    tr->one = one;
    return tr;
}

/*
 * destroy_ptable(pt) - frees all pointers associated with a ptable
*/
void destroy_ptable(PTABLE pt) {
    destroy_entry(pt->entries);
    free(pt);
}

/*
 * merge_helper(pt,tr) - takes a ptable, pt, and a tree, tr and
 * performs a merge a la Huffman. The first two entries of ptable
 * have their probabilities added to determine the probability of
 * their merged entry. A new entry is created with a null character
 * as a placeholder as its character and added to the table
 */
static void merge_helper(PTABLE pt, TREE tr) {
    double prob = pt->entries[0].prob + pt->entries[1].prob;
    ENTRY et = init_entry('\0',prob,tr);
    ptable_pop(pt);
    ptable_pop(pt);
    ptable_add(pt,et);
}

/*
 * merge(pt) - takes a ptable, pt and performs a merge on
 * the first two elements. Gives an assert error if pt is
 * too small.
 */
void merge(PTABLE pt) {
    assert(pt->size > 1);
    if (pt->entries[0].tr == NULL && pt->entries[1].tr == NULL) {
        TREE tr = init_tree(0,pt->entries[0].key,pt->entries[1].key,NULL,NULL);
        merge_helper(pt,tr);
        
    }
    else if (pt->entries[0].tr != NULL && pt->entries[1].tr == NULL) {
        TREE tr = init_tree(1,'\0',pt->entries[1].key,pt->entries[0].tr,NULL);
        merge_helper(pt,tr);
    }
    else if (pt->entries[0].tr == NULL && pt->entries[1].tr != NULL) {
        TREE tr = init_tree(2,pt->entries[0].key,'\0',NULL,pt->entries[1].tr);
        merge_helper(pt,tr);
    }
    else {
        TREE tr = init_tree(3,'\0','\0',pt->entries[0].tr,pt->entries[1].tr); 
        merge_helper(pt,tr);  
    }
}

/*
 * huffman(pt) - continually merges the table until there is a single entry left
 * At this point, the tree associated with the first and only entry is the completed
 * huffman coding tree
 */
TREE huffman(PTABLE pt) {
    while (pt->size > 1) {
        merge(pt);
    }
    return pt->entries[0].tr;
}

/*
 * print_tree_helper(tr,path,size) - helper method for printing a tree takes a tree,
 * a string path to represent the ones and zeros traveled so far, and size, the number
 * of ones and zeroes nodes traveled so far and recursively prints the tree from there. 
 */
void print_tree_helper(TREE tr, char *path, int size) {
    char *cpy0 = (char*)calloc(size+1,sizeof(char));
    char *cpy1 = (char*)calloc(size+1,sizeof(char));
    cpy0 = strncpy(cpy0,path,size+1);
    cpy1 = strncpy(cpy1,path,size+1);
    cpy0 = strcat(cpy0,"0");
    cpy1 = strcat(cpy1,"1");

    if (tr->type == 0) {
        printf("%c: %s\n",tr->zero_value,cpy0);
        printf("%c: %s\n",tr->one_value,cpy1);
    }
    else if (tr->type == 2) {
        printf("%c: %s\n",tr->zero_value,cpy0);
        print_tree_helper(tr->one,cpy1,size+1);
    }
    else if (tr->type == 1) {
        print_tree_helper(tr->zero,cpy0,size+1);
        printf("%c: %s\n",tr->one_value,cpy1);
    }
    else {
        print_tree_helper(tr->zero,cpy0,size+1);
        print_tree_helper(tr->one,cpy1,size+1);
    }
}

/*
 * print_tree(tr) - wrapper method for print_tree_helper to recursively print
 * a tree from the beginning.
 */
void print_tree(TREE tr) {
    print_tree_helper(tr,"",1);
}

/*
 * ctable_insert(ctable,index,value,bits) - add a new code into the code table
 * at index index with value value and bits bits.
 */
static void ctable_insert(CODE ctable, int index, int value, int bits) {
    CODE entry = ctable + index;
    entry->value = value;
    entry->bits = bits;
}

/*
 * tree_to_table(tr,path,ctable,bits) - takes a tree tr, the integer so far,
 * a code table, and a number of bits, and recursively builds out the code
 * table, placing the code in the array pointed to by ctable
 */
void tree_to_table_helper(TREE tr, int path, CODE ctable, int bits) {
    int zero_path = path << 1;
    int one_path = (path << 1) + 1;

    if (tr->type == 0) {
        ctable_insert(ctable,(int)tr->zero_value,zero_path,bits);
        ctable_insert(ctable,(int)tr->one_value,one_path,bits);
    }
    else if (tr->type == 2) {
        ctable_insert(ctable,(int)tr->zero_value,zero_path,bits);
        tree_to_table_helper(tr->one,one_path,ctable,bits+1);
    }
    else if (tr->type == 1) {
        tree_to_table_helper(tr->zero,zero_path,ctable,bits+1);
        ctable_insert(ctable,(int)tr->one_value,one_path,bits);
    }
    else {
        tree_to_table_helper(tr->zero,zero_path,ctable,bits+1);
        tree_to_table_helper(tr->one,one_path,ctable,bits+1);
    }
}

/*
 * tree_to_table(tr,ctable) - wrapper method for tree_to_table_helper to
 * recursively build out a code table from a huffman tree
 */
void tree_to_table(TREE tr,CODE ctable) {
    tree_to_table_helper(tr,0,ctable,1);
}

/*
 * encode_helper(dest,covered,pos,str,ctable) - helper method that takes an int array, dest
 * an int covered, an int pos, a string str, and a code table ctable, and builds out an array
 * of ints to represnt bit by bit the huffman code described by ctable. Each character of the
 * string is read one at a time, and the relevant changes are made to the int array. A pointer
 * is returned that points to the resultant int array.
 */
static int *encode_helper(int* dest, int covered, int pos, char* str, CODE ctable) {
    int value = ctable[(int)str[0]].value;
    int bits = ctable[(int)str[0]].bits;
    int diff = INT_BITS - pos - bits;
    if (diff > 0) {
        *(dest + covered) += (value << diff);
        if (str[0] == '\0') {
            return dest;
        }
        else {
            return encode_helper(dest,covered, pos+bits,str+1,ctable);
        }
    }
    else if (diff == 0) {
        *(dest + covered) += value;
        dest = realloc(dest,(covered+2)*sizeof(int));
        covered += 1;
        *(dest + covered) = 0;
        if (str[0] == '\0') {
            return dest;
        }
        else {
            return encode_helper(dest,covered,0,str+1,ctable);
        }
    }
    else {
        *(dest + covered) += (value >> (-diff));
        int mask = 0;
        for(int i = 0; i < -diff; ++i) {
            mask += (1 << i);
        }
        int remaind = mask & value;
        dest = realloc(dest,(covered+2)*sizeof(int));
        covered += 1;
        *(dest + covered) = 0;
        *(dest + covered) += (remaind << (32 + diff));
        if (str[0] == '\0') {
            return dest;
        }
        else {
            return encode_helper(dest,covered,-diff,str+1,ctable);
        }
    }
}

/*
 * encode(dest,str,ctable) - takes a string str, a code table ctable,
 * and an int pointer dest and builds up an array of integers to represent
 * the huffman encoding. A pointer to the result is returned.
 */
int *encode(int* dest, char* str, CODE ctable) {
    return encode_helper(dest,0,0,str,ctable);
}

/*
 * Prints out an integer as its binary represntation. Used for debugging
 * purposes.
 */
void print_bin_int(int in) {
    //printf("%d",in&(1<<31)>>31);
    for (int i = 0; i < 32; i++) {
        printf("%d",(((in<<i)>>31)&1));
    }
}

/*
 * string_to_code(str) - takes an input string str and returns
 * the code to represnt it.
 */
int *string_to_code(char *str) {
    PTABLE pt = str_to_ptable(str);
    TREE tr = huffman(pt);
    CODE ctable = (CODE)calloc(TSIZE, sizeof(code));
    destroy_ptable(pt);
    tree_to_table(tr,ctable);
    //print_tree(tr,"",1);
    int *dest = (int*)calloc(1,sizeof(int));
    int *result = encode(dest,str,ctable);
    free(ctable);
    return result;
}



/*int main() {
    char *wood = "How much wood could a wood chuck chuck if a wood chuck could chuck wood";
    printf("%s\n",wood);
    int count = 0;
    for(int i = 0; wood[i] != '\0'; ++i) {
        ++count;
    }
    printf("Original size: %d bytes\n",count);
    //print_ptable(pt);

    
    printf("Huffman code:\n");
    
    //print_tree(tr,"",1);
    //printf("%p\n",dest);
    int *dest = string_to_code(wood);
    //printf("\n");
    count = 0;
    for(int i = 0; dest[i] != 0; ++i) {
        count++;
        //printf("%d\n",dest[i]);
        print_bin_int(dest[i]);
    }
    printf("\n");
    printf("Compressed size: %d bytes\n",count*4);
}*/
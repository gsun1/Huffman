// table.c - some functions to define an interface for
// manipulating character frequency table

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "table.h"

#define TSIZE 128 // number of entries needed to represent the ASCII characters

/*
 * init_table - creates an integer table to store the frequencies of all of the characters
 * encountered in the text to be compressed 
 */
static int *init_table() {
    int *result = (int*)calloc(TSIZE,sizeof(int));
    return result;
}

/*
 * inc_table(table,key) - adds one to the table entry for the key specified
 * behavior undefined for invalid keys
 */
static void inc_table(int* table, char key) {
    table[(int)key]++;
}

/*int get_value(int* table, char key) {
    return table[key];
}*/

/*int total_value(int *table, int t_size) {
    int result = 0;
    for(int i = 0; i < t_size; i++) {
        result += table[i];
    }
    return result;
}*/

/*
 * num_non_zero(table, t_size) - loops through a table of size
 * t_size and returns the number of entries that aren't zero
 */
static int num_non_zero(int *table, int t_size) {
    int result = 0;
    for(int i = 0; i < t_size; i++) {
        if (table[i] != 0) {
            ++result;
        }
    }
    return result;
}

/*
 * cmp_entry(a,b) - compares entries to a probability table by comparing
 * their probabilities. Returns -1 if a < b, 0 if a = b, 1 if a > b
 * This is for use in the qsort method
 */
static int cmp_entry(const void* a, const void* b) {
    ENTRY fst = (ENTRY)a;
    ENTRY snd = (ENTRY)b;
    double diff = fst->prob - snd->prob;
    if(diff < 0) {
        return -1;
    }
    else if (diff == 0) {
        return 0;
    }
    else {
        return 1;
    }
}

/*
 * init_entry(key,prob,tr) - initializes a table entry with the data
 * passed to the function
 */

ENTRY init_entry(char key, double prob, TREE tr) {
    ENTRY result = (ENTRY)malloc(sizeof(entry));
    result-> key = key;
    result->prob = prob;
    result->tr = tr;
    return result;
}

/*
 * destroy_tree(tr) - recursively free all of the nodes of a tree, tr
 */
void destroy_tree(TREE tr) {
    if (tr->zero != NULL) {
        destroy_tree(tr->zero);
    }

    if (tr->one != NULL) {
        destroy_tree(tr->one);
    }

    free(tr);
}

/*
 * destroy_entry - get rid of the tree associated 
 */
void destroy_entry(ENTRY et) {
    if(et->tr != NULL) {
        destroy_tree(et->tr);
    }
    free(et);
}

/*
 * prob_table(ftable,t_size,total) - takes an int array ftable of size t_size
 * that represents the frequencies of each character and returns a ptable
 */
static PTABLE prob_table(int *ftable, int t_size, int total) {
    PTABLE result = (PTABLE)malloc(sizeof(ptable));
    int non_zero = num_non_zero(ftable,t_size);
    ENTRY start = (ENTRY)calloc(non_zero,sizeof(entry));

    result->entries = start;
    result->size = non_zero;
    
    int added = 0;
    ENTRY rover = start;
    int i = 0;

    
    while(added < non_zero) {
        if(ftable[i] > 0) {
            rover->key = i;
            rover->prob = (double)ftable[i]/total;
            rover->tr = NULL;
            rover++;
            added++;
        }
        i++;
    }
    qsort(result->entries,result->size,sizeof(entry),cmp_entry);
    return result;
}

/*
 * print_ptable(pt) - prints the entries of a probability table, pt.
 */
void print_ptable(PTABLE pt) {
    for(int i = 0; i < pt->size; ++i) {
        printf("%c:%f ",pt->entries[i].key,pt->entries[i].prob);
    }
    printf("\n");
}

/*
 * str_to_ptable(str) - takes a string, str and converts it into a ptable.
 * ptables should always be sorted from lowest to highest in terms of
 * probability
 */
PTABLE str_to_ptable(char *str) {
    int* ftable = init_table();
    int len = strlen(str);

    for(int i = 0; i < len; i++) {
        inc_table(ftable,str[i]);
    }
    inc_table(ftable,'\0'); // add an end character to be encoded

    PTABLE result = prob_table(ftable,TSIZE,len+1);
    free(ftable);
    return result;
}

/*
 * ptable_pop(pt) - removes the element in pt with the smallest probaility
 * (which is garunteed to be the first element).
 */
void ptable_pop(PTABLE pt) {
    --pt->size;
    ENTRY new = (ENTRY)calloc(pt->size,sizeof(entry));
    memcpy(new,(pt->entries + 1),(pt->size)*sizeof(entry));
    free(pt->entries);
    pt->entries = new;
}

/*
 * ptable_add(pt,et) - adds et to pt and resorts the resulting table
 */
void ptable_add(PTABLE pt, ENTRY et) {
    ++pt->size;
    pt->entries = realloc(pt->entries,pt->size*sizeof(entry));
    pt->entries[pt->size - 1] = *et;
    qsort(pt->entries,pt->size,sizeof(entry),cmp_entry);
}


/*int main() {
    PTABLE pt = str_to_ptable("How much wood can a wood chuck chuck if a wood chuck could chuck wood");
    print_ptable(pt);
    ptable_pop(pt);
    print_ptable(pt);
    ENTRY et = (ENTRY)malloc(sizeof(entry));
    et->prob = .1;
    ptable_add(pt,et);
    print_ptable(pt);
}*/
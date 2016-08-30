#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include "table.h"
#include "tree.h"
#include "decode.h"

#define TSIZE 128
#define MAX_SIZE 1073741824

/*
 * name_helper(name) - takes a string and returns a
 * the name with ".huff" added to the end
 */
static char *name_helper(char *name) {
    int len = strlen(name);
    char *result = calloc(len+6,sizeof(char));
    strncat(result,name,len);
    strcat(result,".huff");
    return result;
}

/*
 * compress(filename) - takes a string filename and creates a file named
 * filename + ".huff" that contains the huffman coding of the file. The
 * first 128 * sizeof(code) bytes are used to store the coding table and
 * the remaining bytes contain the actual encoding.
 */
void compress(char *filename) {
    char *buffer = 0;
    long length;
    FILE *f = fopen(filename,"rb");
    
    assert (f != NULL);

    fseek(f,0,SEEK_END);
    length = ftell(f);
    fseek(f,0,SEEK_SET);
    buffer = malloc(length);

    assert(buffer != NULL);

    fread(buffer,1,length,f);
    fclose(f);

    assert(buffer != NULL);

    char *new_name = name_helper(filename);


    FILE *new_file = fopen(new_name,"w+");


    PTABLE pt = str_to_ptable(buffer);
    TREE tr = huffman(pt);

    CODE ctable = (CODE)calloc(TSIZE, sizeof(code));
    

    tree_to_table(tr,ctable);

    destroy_tree(tr);
    
    //print_tree(tr,"",1);
    fwrite(ctable,sizeof(code),TSIZE,new_file);

    int *code = (int*)calloc(1,sizeof(int));


    code = encode(code,buffer,ctable);

    free(ctable);

    int count = 0;

    for(int i = 0; code[i] != 0; ++i) {
        ++count;
    }

    fwrite(code,sizeof(int),count,new_file);
    free(code);
    fclose(new_file);
}

/*
 * decompress(filename) - takes the string of a filename and first checks
 * to make sure that the format of the file is correct. Assuming it is, it
 * will construct the huffman table from this data and decompress the file,
 * returning the original file.
 */
void decompress(char *filename) {
    int len = strlen(filename);
    assert(len > 4);
    char *check = calloc(5,sizeof(char));
    char *new_name;
    for(int i = len - 5; i < len; ++i) {
        check[i-len+5] = filename[i];
    }
    if (!strcmp(check,".huff")) {
        new_name = calloc(len - 5,sizeof(char));
        strncpy(new_name,filename,len-5);
    }
    else {
        printf("Not the correct file format!\n");
        exit(1);
    }
    FILE *f = fopen(filename,"r");
    CODE ctable = malloc(TSIZE*sizeof(code));
    fread(ctable,sizeof(code),TSIZE,f);
    TREE tr = table_to_tree(ctable);

    free(ctable);

    int *code = (int*)calloc(MAX_SIZE,sizeof(int));
    fread(code,sizeof(int),MAX_SIZE,f);
    char *result = decode(tr,code);

    free(code);
    destroy_tree(tr);
    len = strlen(result);

    FILE *new_file = fopen(new_name,"w+");
    fwrite(result,sizeof(char),len,new_file);
    fclose(new_file);
    free(result);
}

/*
 * main - Takes two arguments. The first argument
 * is a flag that compresses the file if it is "-c"
 * and decompresses the file if it is "-d". The second
 * argument is the file to be operated on.
 */
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Error: Not the correct number of arguments.\nArguments should be in the form: compresser [flag] [file]\n");
        exit(1);
    }
    if (!strcmp(argv[1],"-c")) {
        compress(argv[2]);
    }
    else if (!strcmp(argv[1],"-d")){
        decompress(argv[2]);
    }
    else {
        printf("Error: Flags must be either -c for compress or -d for decompress\n");
    }
}
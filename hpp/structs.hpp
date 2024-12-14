#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct files{

    const char* inputName;
    FILE* input;

    const char* dotName;
    FILE* dot;

    const char* htmlName;
    FILE* html;

    const char* saveName;
    FILE* save;

} files_t;

typedef union data_t{

    double num;
    int op;
    int id;

} data_t;

typedef struct node_t{

    node_t*     parent;
    node_t*     left;
    node_t*     right;

    int         type;
    uint64_t    id;

    data_t      data;

} node_t;

typedef struct tree{

    int64_t numElem;
    int64_t numDump;

    node_t* root;

} tree_t;

typedef struct names_t{

    char*       name;
    uint64_t    len;

} names_t;

typedef struct line{

    char*       buffer;
    uint64_t    ptr;

    node_t*     tokens;
    uint64_t    tptr;

    names_t*    id;
    uint64_t    numId;

    files_t     files;

    tree_t*     tree;

    bool        err;

} line_t;

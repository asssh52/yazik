#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../hpp/colors.hpp"

const int     OP_LEN   = 8;
const int64_t MAX_BUFF = 128;
const int64_t MAX_TKNS = 32;
const int64_t MAX_IDS  = 16;

enum nodeTypes{

    T_NUM = 1,
    T_ID  = 2,
    T_OPR = 3

};



typedef struct opName_t{

    char    name[OP_LEN];
    int     opNum;
    int     len;

} opName_t;



typedef struct files{

    const char* inputName;
    FILE* input;

    const char* dotName;
    FILE* dot;

    const char* htmlName;
    FILE* html;

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

} line_t;


int AnalyseInput    (line_t* line);

int LineCtor        (line_t* line);
int LineRead        (line_t* line);
int LineProcess     (line_t* line);

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../hpp/colors.hpp"

enum nodeTypes{

    T_NUM = 1,
    T_VAR = 2,
    T_OPR = 3

};

typedef struct files{

    const char* inputName;
    FILE* input;

    const char* dotName;
    FILE* dot;

    const char* htmlName;
    FILE* html;

} files_t;

typedef struct node_t{

    node_t*     parent;
    node_t*     left;
    node_t*     right;

    int         type;
    uint64_t    id;
    double      data;

} node_t;

typedef struct tree{

    int64_t numElem;
    int64_t numDump;

    node_t* root;

} tree_t;

typedef struct line{

    char*       buffer;
    uint64_t    ptr;

    files_t     files;

    tree_t*     tree;

} line_t;



int LineCtor        (line_t* line);
int LineRead        (line_t* line);
int LineProcess     (line_t* line);

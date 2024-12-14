#pragma once

const int OP_LEN = 8;

typedef struct opName_t{

    char    name[OP_LEN];
    int     opNum;
    int     len;

} opName_t;

enum nodeOperations{

    O_ADD = 43,
    O_SUB = 45,
    O_MUL = 42,
    O_DIV = 47,
    O_EQL = 61,
    O_SIN = 115,
    O_COS = 99,
    O_TAN = 116,
    O_OBR = 40,
    O_CBR = 41,
    O_IFB = 73,
    O_IFC = 79,
    O_PNT = 80,
    O_TRM = 36,
    O_SEP = 59

};

const opName_t opList[] = {

    {"+",       O_ADD, 1},
    {"-",       O_SUB, 1},
    {"*",       O_MUL, 1},
    {"/",       O_DIV, 1},
    {"=",       O_EQL, 1},
    {"sin",     O_SIN, 3},
    {"cos",     O_COS, 3},
    {"tan",     O_TAN, 3},
    {"(",       O_OBR, 1},
    {")",       O_CBR, 1},
    {"skibidi", O_IFB, 7},
    {"ohio",    O_IFC, 4},
    {"sigma",   O_PNT, 5},
    {"@",       O_TRM, 1},
    {"$",       O_SEP, 1}

};

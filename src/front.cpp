#include "../hpp/front.hpp"
#include "../hpp/dump.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

const int64_t MAX_BUFF  = 256;

const char*   DFLT_INPT_FILE = "./meow.txt";
const char*   DFLT_HTML_FILE = "htmldump.html";
const char*   DFLT_DOT_FILE  = "./bin/dot.dot";


enum errors_t{

    OK  = 0,
    ERR = 1

};

enum syntaxExits{

    S_TRMNL   = 1,
    S_NUM     = 2,
    S_BRCKT   = 3

};

enum nodeOperations{

    O_ADD = 43,
    O_SUB = 45,
    O_MUL = 42,
    O_DIV = 47,
    O_SIN = 115,
    O_COS = 99,
    O_TAN = 116

};

static node_t*  GetN        (line_t* line);
static node_t*  GetG        (line_t* line);
static node_t*  GetE        (line_t* line);
static node_t*  GetT        (line_t* line);
static node_t*  GetP        (line_t* line);
static int      GetF        (line_t* line);
static int      SyntaxError (line_t* line, int param);

static node_t*  NewNode     (tree_t* tree, int data, int type, node_t* left, node_t* right);

/*========================================================================*/

int LineCtor(line_t* line){
    line->buffer = (char*)  calloc(MAX_BUFF, sizeof(char));
    line->tree   = (tree_t*)calloc(1,        sizeof(*line->tree));

    if (!line->files.input) line->files.input = fopen(line->files.inputName, "r");
    if (!line->files.input) line->files.input = fopen(DFLT_INPT_FILE , "r");

    line->files.html = fopen(line->files.htmlName, "w");
    if (!line->files.html) line->files.html = fopen(DFLT_HTML_FILE, "w");

    line->files.dotName = DFLT_DOT_FILE;

    return OK;
}

/*========================================================================*/

int LineRead(line_t* line){
    fread(line->buffer, sizeof(char), MAX_BUFF, line->files.input);

    return OK;
}

/*========================================================================*/

int LineProcess(line_t* line){
    line->tree->root = GetG(line);

    printf("%lld\n", line->tree->numElem);
    TreeDump(line);

    return OK;
}

/*========================================================================*/

static node_t* NewNode(tree_t* tree, int data, int type, node_t* left, node_t* right){
    node_t* newNode = (node_t*)calloc(1, sizeof(*newNode));

    newNode->data   = data;
    newNode->type   = type;

    newNode->left   = left;
    newNode->right  = right;

    if (left)  left->parent  = newNode;
    if (right) right->parent = newNode;

    newNode->id = tree->numElem;
    tree->numElem += 1;

    return newNode;
}

/*========================================================================*/

static int SyntaxError(line_t* line, int param){
    printf(PNK "Syntax error in " ORG "\'%c\'\n" RESET, line->buffer[line->ptr]);

    switch (param){
        case S_TRMNL:
            printf(PNK "expected '$' at " CYN "<%llu> '%c'\n" RESET, line->ptr, line->buffer[line->ptr]);
            break;

        case S_NUM:
            printf(PNK "expected 'num' at " CYN "<%llu> '%c'\n" RESET, line->ptr, line->buffer[line->ptr]);
            break;

        case S_BRCKT:
            printf(PNK "expected ')' at " CYN "<%llu> '%c'\n" RESET, line->ptr, line->buffer[line->ptr]);
            break;
    }

    return 0;
}

/*========================================================================*/
//GRAMMAR FUNCS

static node_t* GetG(line_t* line){
    node_t* node = GetE(line);
    if (line->buffer[line->ptr] != '$') SyntaxError(line, S_TRMNL);

    return node;
}

static node_t* GetN(line_t* line){
    int old_p = line->ptr;
    int val = 0;

    while('0' <= line->buffer[line->ptr] && line->buffer[line->ptr] <= '9'){
        val = val * 10 + line->buffer[line->ptr] - '0';
        line->ptr++;
    }

    if (old_p == line->ptr) SyntaxError(line, S_NUM);

    node_t* newNode = NewNode(line->tree, val, T_NUM, nullptr, nullptr);

    return newNode;
}

static node_t* GetE(line_t* line){
    node_t* left = GetT(line);

    while (line->buffer[line->ptr] == '+' || line->buffer[line->ptr] == '-'){
        int op = line->buffer[line->ptr];
        line->ptr++;

        node_t* right = GetT(line);

        if      (op == '+'){
            left = NewNode(line->tree, O_ADD, T_OPR, left, right);
        }

        else if (op == '-'){
            left = NewNode(line->tree, O_SUB, T_OPR, left, right);
        }
    }

    return left;
}

static node_t* GetT(line_t* line){
    node_t* left = GetP(line);

    while (line->buffer[line->ptr] == '*' || line->buffer[line->ptr] == '/'){
        int op = line->buffer[line->ptr];
        line->ptr++;

        node_t* right = GetP(line);

        if      (op == '*'){
            left = NewNode(line->tree, O_MUL, T_OPR, left, right);
        }

        else if (op == '/'){
            left = NewNode(line->tree, O_DIV, T_OPR, left, right);
        }
    }

    return left;
}

static node_t* GetP(line_t* line){
    if (line->buffer[line->ptr] == '('){
        line->ptr++;

        node_t* node = GetE(line);
        if (line->buffer[line->ptr] != ')') SyntaxError(line, S_BRCKT);

        line->ptr++;

        return node;
    }

    else if (line->buffer[line->ptr] == 'x' && !isalpha(line->buffer[line->ptr + 1])){
        line->ptr++;

        return NewNode(line->tree, 'x', T_VAR, nullptr, nullptr);
    }

    else if (isdigit(line->buffer[line->ptr])){
        return GetN(line);
    }
    
    else{
        int op = GetF(line);

        if (line->buffer[line->ptr] == '('){
            line->ptr++;

            node_t* node = GetE(line);
            if (line->buffer[line->ptr] != ')') SyntaxError(line, S_BRCKT);

            line->ptr++;
            node_t* newNode = NewNode(line->tree, op, T_OPR, nullptr, node);

            return newNode;
        }
    }
}

static int GetF(line_t* line){
    int op = 0;

    if (!strncmp(line->buffer + line->ptr, "sin", 3)){
        line->ptr += 3;
        op = O_SIN;
    }

    else if (!strncmp(line->buffer + line->ptr, "cos", 3)){
        line->ptr += 3;
        op = O_COS;
    }

    else if (!strncmp(line->buffer + line->ptr, "tan", 3)){
        line->ptr += 3;
        op = O_TAN;
    }

    return op;
}

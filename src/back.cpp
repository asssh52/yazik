#include "../hpp/back.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

#define PRNT_JMP(name, ...) PlaceNumOp(line, fprintf(line->files.out, name "%llu:", node->id), __LINE__, 0, node); \
                            fprintf(line->files.out, " " __VA_ARGS__ "\n");\

#define PRNT(name, ...)     PlaceNumOp(line, fprintf(line->files.out, name),                  __LINE__, 0, node); \
                            fprintf(line->files.out, " " __VA_ARGS__ "\n");\

#define PRNT_CUSTOM(...)    PlaceNumOp(line, fprintf(line->files.out, __VA_ARGS__), __LINE__, 1, node)\

const char*   DFLT_HTML_FILE = "htmldump.html";
const char*   DFLT_SAVE_FILE = "save.txt";
const char*   DFLT_OUT_FILE  = "out.txt";
const char*   DFLT_DOT_FILE  = "./bin/dot.dot";

const int64_t MAX_IDS  = 16;

enum errors{

    OK  = 0,
    ERR = 1,

};

enum params{

    DFLT = 0,
    EQL  = 1,
    DEF  = 2

};

enum id_types{

    GLOBAL = 103,
    LOCAL  = 108,

    VAR    = 118,
    FUNC   = 102

};

static int BackCtor         (line_t* line);
static int BackProccess     (line_t* line);
static int NodeProcess      (line_t* line, node_t* node, int param);
static int SetNameTypes     (line_t* line, node_t* node);
static int CreateAddr       (line_t* line, node_t* node);
static int ProcessArgs      (line_t* line, node_t* node);

static int ProcessIf        (line_t* line, node_t* node);
static int ProcessDef       (line_t* line, node_t* node);
static int ProcessWhile     (line_t* line, node_t* node);
static int ProcessCall      (line_t* line, node_t* node);
static int ProcessOp        (line_t* line, node_t* node);

static int PlaceNumOp       (line_t* line, int numSpaces, int numLine, int param, node_t* node);

int main(){
    line_t* line = (line_t*)calloc(1, sizeof(*line));

    BackCtor(line);

    LoadTree(line);

    SetNameTypes(line, line->tree->root);
    MEOW
    DumpIds(line, stdout);

    BackProccess(line);

    DumpIds(line, stdout);

    return 0;
}

static int BackCtor(line_t* line){
    line->tree = (tree_t*)  calloc(1, sizeof(*line->tree));
    line->id   = (names_t*) calloc(MAX_IDS,  sizeof(*line->id));

    line->files.html = fopen(line->files.htmlName, "w");
    if (!line->files.html) line->files.html = fopen(DFLT_HTML_FILE, "w");

    line->files.save = fopen(line->files.saveName, "r");
    if (!line->files.save) line->files.save = fopen(DFLT_SAVE_FILE, "r");

    line->files.out = fopen(line->files.outName, "w");
    if (!line->files.out) line->files.out = fopen(DFLT_OUT_FILE, "w");

    line->files.dotName = DFLT_DOT_FILE;

    line->freeAddr = 1;

    return OK;
}

static int BackProccess(line_t* line){

    NodeProcess(line, line->tree->root, DFLT);
    fprintf(line->files.out, "hlt\n");


    return OK;
}

static int NodeProcess(line_t* line, node_t* node, int param){

    int type = node->type;
    int op = node->data.op;

    // =
    if (type == T_OPR && op == O_EQL){
        NodeProcess(line, node->right, DFLT);
        NodeProcess(line, node->left, EQL);
    }

    // IF
    else if(type == T_OPR && op == O_IFB){
        ProcessIf(line, node);
    }

    // WHILE
    else if(type == T_OPR && op == O_WHB){
        ProcessWhile(line, node);
    }

    //DEF FUNC
    else if(type == T_OPR && op == O_DEF){
        ProcessDef(line, node);
    }

    //CALL
    else if( type == T_OPR && op == O_CAL){
        ProcessCall(line, node);
    }

    // OTHER
    else if (type == T_OPR && (op == O_ADD || op == O_SUB || op == O_DIV ||   // +  -  /
                               op == O_MUL || op == O_LES || op == O_LSE ||   // *  <  <=
                               op == O_MOR || op == O_MRE || op == O_EQQ ||   // >  >= ==
                               op == O_NEQ || op == O_POW)){                  // != ^

        if (node->right)  NodeProcess(line, node->right, DFLT);
        if (node->left)   NodeProcess(line, node->left, DFLT);
    }

    else{
        if (node->left)   NodeProcess(line, node->left, DFLT);
        if (node->right)  NodeProcess(line, node->right, DFLT);
    }

    // ID
    if (type == T_ID && param == EQL){
        if (line->id[node->data.id].visibilityType == GLOBAL && line->id[node->data.id].memAddr == 0){
            CreateAddr(line, node);


            PRNT_CUSTOM("pop [%d]", line->id[node->data.id].memAddr);

            PRNT("push bx", "");
            PRNT("push 1", "");

            PRNT("add", "");

            PRNT("pop bx", "");

        }

        else if (line->id[node->data.id].visibilityType == GLOBAL && line->id[node->data.id].memAddr != 0){
            PRNT_CUSTOM("pop [%d]", line->id[node->data.id].memAddr);
        }

        else if (line->id[node->data.id].visibilityType == LOCAL){
            PRNT_CUSTOM("pop [%d]", line->id[node->data.id].memAddr);
        }
    }

    else if (type == T_ID && param != EQL){
        if (line->id[node->data.id].visibilityType == GLOBAL){
            PRNT_CUSTOM("push [%d]", line->id[node->data.id].memAddr);
        }

        else{
            PRNT_CUSTOM("push [bx+%d]", line->id[node->data.id].memAddr);
        }
    }
    //ID



    // NUM
    if (type == T_NUM){
        PRNT_CUSTOM("push %lld", (int64_t)node->data.num);
    }
    // NUM



    //OP
    if (type == T_OPR){
        ProcessOp(line, node);
    }
    //OP


    return OK;
}

static int CreateAddr(line_t* line, node_t* node){

    line->id[node->data.id].memAddr = line->freeAddr;
    line->freeAddr += 1;

    return OK;
}

static int ProcessArgs(line_t* line, node_t* node){
    node_t* nodeSpec = node->left;

    int paramCount = 0;

    node_t* nodeComma = nodeSpec;

    while (nodeComma->right && nodeComma->right->type == T_OPR && nodeComma->right->data.op == O_CMA){
        paramCount += 1;

        nodeComma = nodeComma->right;
        node_t* nodeArg = nodeComma->left;

        line->id[nodeArg->data.id].visibilityType   = LOCAL;
        line->id[nodeArg->data.id].memAddr          = paramCount;
    }

    line->id[nodeSpec->left->data.id].stackFrameSize = paramCount;
    line->id[nodeSpec->left->data.id].numParams = paramCount;

    return OK;
}

static int SetNameTypes(line_t* line, node_t* node){

    if (node->type == T_ID && node->parent && (node->parent->data.op == O_DFS || node->parent->data.op == O_CSP)){
        line->id[node->data.id].idType = FUNC;
        line->id[node->data.id].visibilityType = FUNC;

    }

    else if (node->type == T_ID && node->parent && node->parent->data.op != O_CMA){
        line->id[node->data.id].visibilityType = GLOBAL;
        line->id[node->data.id].idType = VAR;
    }

    else if (node->type == T_ID){
        line->id[node->data.id].idType = VAR;
    }

    if (node->left)  SetNameTypes(line, node->left);
    if (node->right) SetNameTypes(line, node->right);

    return OK;
}

static int PlaceNumOp(line_t* line, int numSpaces, int numLine, int param, node_t* node){

    for (int i = 0; i < 32 - numSpaces; i++){
        fprintf(line->files.out, " ");
    }

    fprintf(line->files.out, "line;%0.3d\t", numLine);

    fprintf(line->files.out, "node;%0.3llu\t", node->id);

    if (param) fprintf(line->files.out, "\n");

    return OK;
}

static int ProcessIf(line_t* line, node_t* node){

    NodeProcess(line, node->left, DFLT);

    PRNT    ("push 0", "НАЧАЛО ИФА");
    PRNT_JMP("je end_if", "");

    NodeProcess(line, node->right, DFLT);

    PRNT_JMP("end_if", "");

    return OK;
}

static int ProcessWhile(line_t* line, node_t* node){
    PRNT_JMP("while", "НАЧАЛО ЦИКЛА");

    NodeProcess(line, node->left, DFLT);

    PRNT    ("push 0", "");
    PRNT_JMP("je end_while", "");

    NodeProcess(line, node->right, DFLT);

    PRNT_JMP("jmp while", "");
    PRNT_JMP("end_while", "");

    return OK;
}

static int ProcessDef(line_t* line, node_t* node){
    ProcessArgs(line, node);

    PRNT("", "НАЧАЛО ОПРЕДЕЛЕНИЯ ФУНКЦИИ");
    PRNT_CUSTOM("jmp def_func_end%d:",   node->left->left->data.id);
    PRNT_CUSTOM("def_func%d:", node->left->left->data.id);

    NodeProcess(line, node->right, DFLT);

    PRNT_CUSTOM("def_func_end%d:", node->left->left->data.id);

    return OK;
}

static int ProcessCall(line_t* line, node_t* node){
    PRNT        ("push bx", "Я НАЧАЛО ВЫЗОВ ФУНКЦИЯ");

    NodeProcess(line, node->left->right, DFLT);

    PRNT        ("push bx", "");
    PRNT_CUSTOM ("push %d", line->id[node->left->left->data.id].stackFrameSize);
    PRNT        ("add", "");
    PRNT        ("pop bx", "");

    int params = line->id[node->left->left->data.id].numParams; // num params

    for (int i = params; i > 0; i--){
        PRNT_CUSTOM("pop [bx+%d]", i);
    }

    PRNT_CUSTOM ("call def_func%d:", node->left->left->data.id);
    PRNT        ("pop bx", "");

    PRNT        ("push ax", "");

    return OK;
}

static int ProcessOp(line_t* line, node_t* node){

    switch (node->data.op){
        // +
        case O_ADD:
            PRNT("add", "");
            break;

        // -
        case O_SUB:
            PRNT("sub", "");
            break;

        // *
        case O_MUL:
            PRNT("mul", "");
            break;

        // /
        case O_DIV:
            PRNT("div", "");
            break;

        // print
        case O_PNT:
            PRNT("out", "");
            break;

        // ;
        case O_SEP:
            PRNT("", "");
            break;

        // =
        case O_EQL:
            PRNT("", "");
            break;

        // if
        case O_IFB:
            PRNT("", "КОНЕЦ ИФА");
            break;

        // while
        case O_WHB:
            PRNT("", "КОНЕЦ ЦИКЛА");
            break;

        // function defenition
        case O_DEF:
            PRNT("", "КОНЕЦ ОПРЕДЕЛЕНИЯ ФУНКЦИИ");
            break;

        // return
        case O_RET:
            PRNT("pop ax", "");
            PRNT("ret", "");
            break;

        // call
        case O_CAL:
            PRNT("", "");
            break;

        // <
        case O_LES:
            PRNT("less", "");
            break;

        // <=
        case O_LSE:
            PRNT("less_equal", "");
            break;

        // >
        case O_MOR:
            PRNT("more", "");
            break;

        // >=
        case O_MRE:
            PRNT("more_equal", "");
            break;

        // ==
        case O_EQQ:
            PRNT("equal", "");
            break;

        // !=
        case O_NEQ:
            PRNT("not_equal", "");
            break;

        // ^
        case O_POW:
            PRNT("power", "");
            break;

        // ,
        case O_CMA:
            PRNT("", "");
            break;

        // sqrt
        case O_SQT:
            PRNT("sqrt", "");
            break;

        default:
            PRNT("uknown node", "");
            break;
    }

    return OK;
}

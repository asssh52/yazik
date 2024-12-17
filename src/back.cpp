#include "../hpp/back.hpp"

#define MEOW fprintf(stderr, PNK  "MEOW\n" RESET);

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
static int SetNameTypes (line_t* line, node_t* node);
static int CreateAddr       (line_t* line, node_t* node);
static int ProcessArgs      (line_t* line, node_t* node);

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

    line->freeAddr = 10;

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

    if (node->type == T_OPR){
        fprintf(line->files.out, "\t\t\t\t#%llu\n", node->id);
    }


    // =
    if (type == T_OPR && op == O_EQL){

        NodeProcess(line, node->right, DFLT);
        NodeProcess(line, node->left, EQL);

    }

    // IF
    else if(type == T_OPR && op == O_IFB){

        NodeProcess(line, node->left, DFLT);

        fprintf(line->files.out, "push 0 \t\t\t#%llu\tЯ НАЧАЛО ИФА\n", node->id);
        fprintf(line->files.out, "je end_if%llu: \t#%llu\n", node->id, node->id);

        NodeProcess(line, node->right, DFLT);

        fprintf(line->files.out, "end_if%llu: \t\t#%llu\n", node->id, node->id);
    }

    // WHILE
    else if(type == T_OPR && op == O_WHB){

        fprintf(line->files.out, "while%llu: \t\t#%llu\n", node->id, node->id);

        NodeProcess(line, node->left, DFLT);

        fprintf(line->files.out, "push 0 \t\t\t#%llu\n", node->id);
        fprintf(line->files.out, "je end_while%llu: \t#%llu\n", node->id, node->id);

        NodeProcess(line, node->right, DFLT);

        fprintf(line->files.out, "jmp while%llu: \t\t#%llu\n", node->id, node->id);
        fprintf(line->files.out, "end_while%llu: \t\t#%llu\n", node->id, node->id);

    }

    //DEF FUNC
    else if(type == T_OPR && op == O_DEF){

        ProcessArgs(line, node);

        fprintf(line->files.out, "jmp def_func_end%d: \t#%llu\tЯ ФУНКЦИЯ\n",   node->left->left->data.id, node->id);
        fprintf(line->files.out, "def_func%d: \t\t#%llu\n",         node->left->left->data.id, node->id);

        NodeProcess(line, node->right, DFLT);

        fprintf(line->files.out, "def_func_end%d: \t\t#%llu\n",     node->left->left->data.id, node->id);
    }

    //CALL
    else if( type == T_OPR && op == O_CAL){

        fprintf(line->files.out, "push bx \t\t#%llu\tЯ НАЧАЛО ВЫЗОВ ФУНКЦИЯ\n\n",             node->id);

        NodeProcess(line, node->left->right->left, DFLT);

        fprintf(line->files.out, "push bx \t\t#%llu\n",             node->id);
        fprintf(line->files.out, "push %d \t\t\t#%llu\n",             line->id[node->left->left->data.id].stackFrameSize, node->id);
        fprintf(line->files.out, "add \t\t\t#%llu\n",             node->id);

        fprintf(line->files.out, "pop bx \t\t\t#%llu\n",             node->id);

        int i = 1; // num params

        fprintf(line->files.out, "pop [bx + %d] \t#%llu\n",     i, node->id);

        fprintf(line->files.out, "call def_func%d: #%llu\n",  node->left->left->data.id, node->id);

        fprintf(line->files.out, "pop bx \t\t\t#%llu\n",              node->id);

        fprintf(line->files.out, "push ax \t\t#%llu\n",              node->id);
    }

    // OTHER
    else if (type == T_OPR && (op == O_ADD || op == O_SUB || op == O_DIV || op == O_MUL || op == O_LES || op == O_LSE || op == O_MOR || op == O_MRE || op == O_EQQ)){
        if (node->right)  NodeProcess(line, node->right, DFLT);
        if (node->left)   NodeProcess(line, node->left, DFLT);
    }

    else{
        if (node->left)   NodeProcess(line, node->left, DFLT);
        if (node->right)  NodeProcess(line, node->right, DFLT);
    }

    // ID
    if (type == T_ID && param == EQL){
        //if (line->id[node->data.id].memAddr == 0) CreateAddr(line, node);

        if (line->id[node->data.id].visibilityType == GLOBAL && line->id[node->data.id].memAddr == 0){
            CreateAddr(line, node);

            fprintf(line->files.out, "pop [%d] \t\t#%llu\n", line->id[node->data.id].memAddr, node->id);

            fprintf(line->files.out, "push bx \t\t#%llu\n", node->id);
            fprintf(line->files.out, "push 1 \t\t#%llu\n", node->id);

            fprintf(line->files.out, "add \t\t#%llu\n", node->id);

            fprintf(line->files.out, "pop bx \t\t#%llu\n", node->id);
        }

        else if (line->id[node->data.id].visibilityType == GLOBAL && line->id[node->data.id].memAddr != 0){
            fprintf(line->files.out, "pop [%d] \t\t#%llu\n", line->id[node->data.id].memAddr, node->id);
        }

        else if (line->id[node->data.id].visibilityType == LOCAL){
            fprintf(line->files.out, "pop [bx + %d] \t\t#%llu\n", line->id[node->data.id].memAddr, node->id);
        }
    }

    else if (type == T_ID && param != EQL){
        if (line->id[node->data.id].visibilityType == GLOBAL){
            fprintf(line->files.out, "push [%d] \t\t#%llu\n", line->id[node->data.id].memAddr, node->id);
        }

        else fprintf(line->files.out, "push [bx + %d] \t\t#%llu\n", line->id[node->data.id].memAddr, node->id);
    }
    //ID



    // NUM
    if (type == T_NUM){
        fprintf(line->files.out, "push %lld \t\t\t#%llu\n", (int64_t)node->data.num, node->id);
    }
    // NUM



    //OP
    if (type == T_OPR){
        switch (node->data.op){

            case O_ADD:
                fprintf(line->files.out, "add \t\t\t#%llu\n", node->id);
                break;

            case O_SUB:
                fprintf(line->files.out, "sub \t\t\t#%llu\n", node->id);
                break;

            case O_MUL:
                fprintf(line->files.out, "mul \t\t\t#%llu\n", node->id);
                break;

            case O_DIV:
                fprintf(line->files.out, "div \t\t\t#%llu\n", node->id);
                break;

            case O_PNT:
                fprintf(line->files.out, "out \t\t\t#%llu\n", node->id);
                break;

            case O_SEP:
                fprintf(line->files.out, "\t\t\t\t#%llu\n", node->id);
                break;

            case O_EQL:
                fprintf(line->files.out, "\t\t\t\t#%llu\n", node->id);
                break;

            case O_IFB:
                fprintf(line->files.out, "\t\t\t\t#%llu\t Я КОНЕЦ ИФА\n", node->id);
                break;

            case O_WHB:
                fprintf(line->files.out, "\t\t\t\t#%llu\n", node->id);
                break;

            case O_DEF:
                fprintf(line->files.out, "\t\t\t\t#%llu\tЯ КОНЕЦ ФУНКЦИЯ\n", node->id);
                break;

            case O_RET:
                fprintf(line->files.out, "pop ax\t\t\t#%llu\n", node->id);
                fprintf(line->files.out, "ret \t\t\t#%llu\n", node->id);
                break;

            case O_CAL:
                fprintf(line->files.out, "\t\t\t\t#%llu\n", node->id);
                break;

            case O_LES:
                fprintf(line->files.out, "less\t\t#%llu\n", node->id);
                break;

            case O_LSE:
                fprintf(line->files.out, "less_equal\t\t#%llu\n", node->id);
                break;

            case O_MOR:
                fprintf(line->files.out, "more\t\t#%llu\n", node->id);
                break;

            case O_MRE:
                fprintf(line->files.out, "more_equal\t\t#%llu\n", node->id);
                break;

            case O_EQQ:
                fprintf(line->files.out, "equal\t\t#%llu\n", node->id);
                break;

            default:
                fprintf(line->files.out, "??? \t\t\t#%llu\n", node->id);
                break;
        }
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

    int frameSize = 0;

    node_t* nodeComma = nodeSpec;

    while (nodeComma->right && nodeComma->right->type == T_OPR && nodeComma->right->data.op == O_CMA){
        frameSize += 1;

        nodeComma = nodeComma->right;
        node_t* nodeArg = nodeComma->left;

        line->id[nodeArg->data.id].visibilityType   = LOCAL;
        line->id[nodeArg->data.id].memAddr          = frameSize;
    }

    line->id[nodeSpec->left->data.id].stackFrameSize = frameSize;

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

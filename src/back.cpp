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

};

static int BackCtor     (line_t* line);
static int BackProccess (line_t* line);
static int NodeProcess  (line_t* line, node_t* node, int param);

int main(){
    line_t* line = (line_t*)calloc(1, sizeof(*line));

    BackCtor(line);

    LoadTree(line);

    BackProccess(line);

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

    return OK;
}

static int BackProccess(line_t* line){

    NodeProcess(line, line->tree->root, DFLT);
    fprintf(line->files.out, "hlt\n");


    return OK;
}

static int NodeProcess(line_t* line, node_t* node, int param){

    if (node->type == T_OPR) fprintf(line->files.out, "\t\t\t\t#%llu\n", node->id);


    // =
    if (node->type == T_OPR && node->data.op == O_EQL){

        NodeProcess(line, node->right, EQL);
        NodeProcess(line, node->left, EQL);

    }

    // IF
    else if(node->type == T_OPR && node->data.op == O_WHB){

        NodeProcess(line, node->left, DFLT);

        fprintf(line->files.out, "push 0 \t\t\t#%llu\n", node->id);
        fprintf(line->files.out, "je end_if%llu: \t#%llu\n", node->id, node->id);

        NodeProcess(line, node->right, DFLT);

        fprintf(line->files.out, "end_if%llu: \t\t#%llu\n", node->id, node->id);
    }

    // WHILE
    else if(node->type == T_OPR && node->data.op == O_WHB){

        fprintf(line->files.out, "while%llu: \t\t#%llu\n", node->id, node->id);

        NodeProcess(line, node->left, DFLT);

        fprintf(line->files.out, "push 0 \t\t\t#%llu\n", node->id);
        fprintf(line->files.out, "je end_while%llu: \t#%llu\n", node->id, node->id);

        NodeProcess(line, node->right, DFLT);

        fprintf(line->files.out, "jmp while%llu: \t\t#%llu\n", node->id, node->id);
        fprintf(line->files.out, "end_while%llu: \t\t#%llu\n", node->id, node->id);

    }

    // OTHER
    else{
        if (node->left)   NodeProcess(line, node->left, DFLT);
        if (node->right)  NodeProcess(line, node->right, DFLT);
    }

    // ID
    if (node->type == T_ID && param == EQL){
        fprintf(line->files.out, "pop [%d] \t\t#%llu\n", node->data.id, node->id);
    }

    else if (node->type == T_ID && param != EQL){
        fprintf(line->files.out, "push [%d] \t\t#%llu\n", node->data.id, node->id);
    }
    //ID



    // NUM
    if (node->type == T_NUM){
        fprintf(line->files.out, "push %lld \t\t\t#%llu\n", (int64_t)node->data.num, node->id);
    }
    // NUM



    //OP
    if (node->type == T_OPR){
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
                fprintf(line->files.out, "\t\t\t\t#%llu\n", node->id);
                break;

            case O_WHB:
                fprintf(line->files.out, "\t\t\t\t#%llu\n", node->id);
                break;

            default:
                fprintf(line->files.out, "??? \t\t\t#%llu\n", node->id);
                break;
        }
    }
    //OP


    return OK;
}

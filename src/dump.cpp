#include "../hpp/dump.hpp"

const int64_t MAXDOT_BUFF    = 8;
const int64_t MAXLEN_COMMAND = 64;
const int64_t MAX_HTML_PRNT  = 1024;

static int NodeDump         (line_t* line, node_t* node, int depth, int param);
static int StartLineDump    (line_t* line);
static int EndLineDump      (line_t* line);
static int DoDot            (line_t* line);
static int HTMLGenerateBody (line_t* line);
static int TokenDump        (line_t* line, node_t* node);
static int HTMLPrint        (line_t* line, char* text);

enum errors_dump{

    OK  = 0,
    ERR = 1

};

enum dump_params{

    SIMPLE   = 52,
    DETAILED = -52

};

int TreeDump(line_t* line){

    StartLineDump(line);
    NodeDump(line, line->tree->root, 0, SIMPLE);
    EndLineDump(line);
    DoDot(line);
    HTMLGenerateBody(line);

    return OK;
}

int TokensDump(line_t* line){

    StartLineDump(line);

    for (int i = 0; i < MAX_TKNS; i++){
        TokenDump(line, line->tokens + i);
    }

    EndLineDump(line);
    DoDot(line);
    HTMLGenerateBody(line);

    char* text = (char*) calloc(MAX_HTML_PRNT, sizeof(*text));

    for (int i = 0; i < line->numId; i++){
        snprintf(text, MAX_HTML_PRNT, "%s%d. name:\"", text, i);

        for (int j = 0; j < line->id[i].len; j++){
            snprintf(text, MAX_HTML_PRNT, "%s%c", text, line->id[i].name[j]);
        }

        snprintf(text, MAX_HTML_PRNT, "%s\"\t len:%llu <br>", text, line->id[i].len);
    }

    HTMLPrint(line, text);

    return OK;
}

int DumpIds(line_t* line, FILE* file){
    fprintf(file, ORG "\n-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n" RESET);
    fprintf(file, "Nametable dump:\n\n");

    for (int i = 0; i < line->numId; i++){
        fprintf(file, "%d. name:", i);
        for (int j = 0; j < line->id[i].len; j++){
            fprintf(file, "%c", line->id[i].name[j]);
        }

        fprintf(file, "\t len:%llu\n", line->id[i].len);
    }
    fprintf(file, ORG "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n" RESET);

    return OK;
}

static int TokenDump(line_t* line, node_t* node){
    char outBuff[MAXDOT_BUFF] = {};

/*---------DETAILED---------*/

    if (node->type == T_NUM){
        fprintf(line->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" {{ %0.3llu } | { p: %p } | { num: %0.2lf } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
            node->id, node->id, node, node->data.num, node->left, node->right, node->parent);
    }

    else if (node->type == T_OPR){
        snprintf(outBuff, MAXDOT_BUFF, "%c", node->data.op);

        fprintf(line->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" {{ %0.3llu } | { p: %p } | { oper: %s } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
            node->id, node->id, node, outBuff, node->left, node->right, node->parent);
    }

    else if (node->type == T_ID){
        fprintf(line->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" {{ %0.3llu } | { p: %p } | { id: %d }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
            node->id, node->id, node, node->data.id, node->left, node->right, node->parent);
    }

    else {
        fprintf(line->files.dot,
            "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" {{ %0.3llu } | { p: %p } | { unknown %0.0lf }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
            node->id, node->id, node, node->data.num, node->left, node->right, node->parent);
    }

    return OK;
}

static int NodeDump(line_t* line, node_t* node, int depth, int param){
    if (!node) return OK;

    char outBuff[MAXDOT_BUFF] = {};
/*---------SIMPLE---------*/
    if (param == SIMPLE){
        if (node->type == T_NUM){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" { %0.3llu } | { num: %0.2lf }\"];\n",
                node->id, node->id, node->data.num);
        }

        else if (node->type == T_OPR){
            snprintf(outBuff, MAXDOT_BUFF, "%c", node->data.op);

            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" { %0.3llu } | { oper: %s }\"];\n",
                node->id, node->id, outBuff);
        }

        else if (node->type == T_ID){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" { %0.3llu } | { id: %d }\"];\n",
                node->id, node->id, node->data.id);
        }

        else {
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" { %0.3llu } | { unknown %0.0lf }\"];\n",
                node->id, node->id, node->data.num);
        }
    }
/*---------SIMPLE---------*/

/*---------DETAILED---------*/
    else if (param == DETAILED){
        if (node->type == T_NUM){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" {{ %0.3llu } | { p: %p } | { num: %0.2lf } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data.num, node->left, node->right, node->parent);
        }

        else if (node->type == T_OPR){
            snprintf(outBuff, MAXDOT_BUFF, "%c", node->data.op);

            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" {{ %0.3llu } | { p: %p } | { oper: %s } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, outBuff, node->left, node->right, node->parent);
        }

        else if (node->type == T_ID){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" {{ %0.3llu } | { p: %p } | { var: %c }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data.id, node->left, node->right, node->parent);
        }

        else {
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" {{ %0.3llu } | { p: %p } | { unknown %0.0lf }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data.num, node->left, node->right, node->parent);
        }
    }
/*---------DETAILED---------*/

    //edges
    if (node->left){
        fprintf(line->files.dot,
                "\tnode%0.3llu -> node%0.3llu [ fontname=\"SF Pro\"; weight=1; color=\"#04BF00\"; style=\"bold\"];\n\n",
                node->id, node->left->id);

        NodeDump(line, node->left, depth + 1, param);
    }

    if (node->right){
        fprintf(line->files.dot,
                "\tnode%0.3llu -> node%0.3llu [ fontname=\"SF Pro\"; weight=1; color=\"#fd4381\"; style=\"bold\"];\n\n",
                node->id, node->right->id);

        NodeDump(line, node->right, depth + 1, param);
    }
    //edges

    return OK;
}

static int StartLineDump(line_t* line){

    line->files.dot = fopen("./bin/dot.dot", "w");

    fprintf(line->files.dot, "digraph G{\n");

    fprintf(line->files.dot, "\trankdir=TB;\n");
    fprintf(line->files.dot, "\tbgcolor=\"#f8fff8\";\n");

    return OK;
}

static int EndLineDump(line_t* line){

    fprintf(line->files.dot, "}\n");

    fclose(line->files.dot);

    return OK;
}

static int DoDot(line_t* line){
    char command[MAXLEN_COMMAND]   = {};
    char out[MAXLEN_COMMAND]       = {};

    const char* startOut= "./bin/png/output";
    const char* endOut  = ".png";

    snprintf(out,     MAXLEN_COMMAND, "%s%llu%s", startOut, line->tree->numDump, endOut);
    snprintf(command, MAXLEN_COMMAND, "dot -Tpng %s > %s", line->files.dotName, out);
    system(command);

    line->tree->numDump++;
    return OK;
}

int HTMLGenerateHead(line_t* line){
    fprintf(line->files.html, "<html>\n");

    fprintf(line->files.html, "<head>\n");
    fprintf(line->files.html, "</head>\n");

    fprintf(line->files.html, "<body style=\"background-color:#f8fff8;\">\n");

    return OK;
}

static int HTMLGenerateBody(line_t* line){
    fprintf(line->files.html, "<div style=\"text-align: center;\">\n");

    fprintf(line->files.html, "\t<h2 style=\"font-family: 'Haas Grot Text R Web', 'Helvetica Neue', Helvetica, Arial, sans-serif;'\"> Dump: %llu</h2>\n", line->tree->numDump);
    fprintf(line->files.html, "\t<h3 style=\"font-family: 'Haas Grot Text R Web', 'Helvetica Neue', Helvetica, Arial, sans-serif;'\"> Num elems: %llu</h3>\n", line->tree->numElem);

    fprintf(line->files.html, "\t<img src=\"./bin/png/output%llu.png\">\n\t<br>\n\t<br>\n\t<br>\n", line->tree->numDump - 1);

    fprintf(line->files.html, "</div>\n");

    return OK;
}

static int HTMLPrint(line_t* line, char* text){
    fprintf(line->files.html, "<div style=\"text-align: left;\">\n");

    fprintf(line->files.html, "\t<h4 style=\"font-family: 'Haas Grot Text R Web', 'Helvetica Neue', Helvetica, Arial, sans-serif;'\"> %s </h3>\n", text);

    fprintf(line->files.html, "</div>\n");

    return OK;
}

int HTMLDumpGenerate(line_t* line){

    fprintf(line->files.html, "</body>\n");
    fprintf(line->files.html, "</html>\n");

    return OK;
}

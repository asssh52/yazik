#include "../hpp/dump.hpp"

const int64_t MAXDOT_BUFF    = 8;
const int64_t MAXLEN_COMMAND = 64;

static int NodeDump         (line_t* line, node_t* node, int depth, int param);
static int StartLineDump    (line_t* line);
static int EndLineDump      (line_t* line);
static int DoDot            (line_t* line);
static int HTMLGenerateBody (line_t* line);


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

static int NodeDump(line_t* line, node_t* node, int depth, int param){
    if (!node) return OK;
    if (depth > line->tree->numElem) return ERR;

    char outBuff[MAXDOT_BUFF] = {};
/*---------SIMPLE---------*/
    if (param == SIMPLE){
        if (node->type == T_NUM){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" { %0.3llu } | { num: %0.2lf }\"];\n",
                node->id, node->id, node->data);
        }

        else if (node->type == T_OPR){
            snprintf(outBuff, MAXDOT_BUFF, "%c", (char)node->data);

            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" { %0.3llu } | { oper: %s }\"];\n",
                node->id, node->id, outBuff);
        }

        else if (node->type == T_VAR){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" { %0.3llu } | { var: %c }\"];\n",
                node->id, node->id, (char)node->data);
        }

        else {
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" { %0.3llu } | { unknown %0.0lf }\"];\n",
                node->id, node->id, node->data);
        }
    }
/*---------SIMPLE---------*/

/*---------DETAILED---------*/
    else if (param == DETAILED){
        if (node->type == T_NUM){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6f2ff\";label = \" {{ %0.3llu } | { p: %p } | { num: %0.2lf } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data, node->left, node->right, node->parent);
        }

        else if (node->type == T_OPR){
            snprintf(outBuff, MAXDOT_BUFF, "%c", (char)node->data);

            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#fff3e6\";label = \" {{ %0.3llu } | { p: %p } | { oper: %s } | {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, outBuff, node->left, node->right, node->parent);
        }

        else if (node->type == T_VAR){
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#e6ffe6\";label = \" {{ %0.3llu } | { p: %p } | { var: %c }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, (char)node->data, node->left, node->right, node->parent);
        }

        else {
            fprintf(line->files.dot,
                "\tnode%0.3llu [rankdir=LR; fontname=\"SF Pro\"; shape=Mrecord; style=filled; color=\"#ffe6fe\";label = \" {{ %0.3llu } | { p: %p } | { unknown %0.0lf }| {left: %p} | {right: %p} | {parent: %p}}\"];\n",
                node->id, node->id, node, node->data, node->left, node->right, node->parent);
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

int HTMLDumpGenerate(line_t* line){

    fprintf(line->files.html, "</body>\n");
    fprintf(line->files.html, "</html>\n");

    return OK;
}

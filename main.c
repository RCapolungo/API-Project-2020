#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct textPile {
    char *text;
    struct textPile *prev;
    struct textPile *next;
};

struct undoPile {
    char command;
    int line1;
    int line2;
    int line3;
    char **text;
    char **overW;
    struct undoPile *prev;
    struct undoPile *next;
};

struct textPileHead {
    struct textPile *first;
    struct textPile *last;
};

struct undoPileHead {
    int cellNum;
    struct undoPile *first;
    struct undoPile *last;
};

struct textPileHead *pile = NULL;
struct undoPileHead *undo = NULL;
struct undoPileHead *redo = NULL;
int pileSize = 0;

void redoFunction(int num);

//Find in the text list the corresponding line.
struct textPile *search(int line) {
    struct textPile *temp;
    int i;
    if(line < pileSize / 2 + 1) {
        temp = pile->first;
        for (i = 1; i != line && temp != NULL; i++, temp = temp->next) {}
    } else {
        temp = pile->last;
        for (i = pileSize; i != line && temp != NULL; i--, temp = temp->prev) {
        }
    }
    return temp;
}

//Base insert function
struct textPile *baseInsert(struct textPile *prev, struct textPile *next, struct textPile *temp) {
    temp->next = next;
    temp->prev = prev;
    if(prev != NULL) {
        prev->next = temp;
    }
    if(next != NULL) {
        next->prev = temp;
    }
    return temp;
}

//Overwrite a line text.
char *overwriteStart(struct textPile *temp, char *text) {
    char *oldText;
    oldText = temp->text;
    temp->text = text;
    return oldText;
}

//Insert a line in the text list.
struct textPile *insertStart(struct textPile *newLine, char **text, int line1, char **overW) {
    struct textPile *temp = NULL;
    if(pile->first == NULL) {
        newLine = malloc(sizeof(struct textPile));
        newLine = baseInsert(pile->first, pile->last, newLine);
        newLine->text = *text;
        newLine->next = NULL;
        pile->first = newLine;
        pile->last = newLine;
        if(overW != NULL) {
            *overW = NULL;
        }
        pileSize++;
        return newLine;
    } else {
        if(line1 == pileSize) {
            if(overW != NULL) {
                *overW = overwriteStart(pile->last, *text);
            } else {
                overwriteStart(pile->last, *text);
            }
            return newLine;
        }
        if(line1 == pileSize + 1 && pileSize != 0) {
            newLine = malloc(sizeof(struct textPile));
            newLine = baseInsert(pile->last, pile->last->next, newLine);
            newLine->text = *text;
            pileSize++;
            if(overW != NULL) {
                *overW = NULL;
            }
            pile->last = newLine;
            return newLine;
        }
        if(newLine == NULL) {
            temp = search(line1);
            if(overW != NULL) {
                *overW = overwriteStart(temp, *text);
            } else {
                overwriteStart(temp, *text);
            }
            newLine = temp;
        } else {
            if(newLine->next == NULL) {
                temp = malloc(sizeof(struct textPile));
                temp = baseInsert(newLine, newLine->next, temp);
                temp->text = *text;
                if (pileSize == line1 - 1) {
                    pile->last = temp;
                }
                pileSize++;
                if(overW != NULL) {
                    *overW = NULL;
                }
                newLine = temp;
            } else {
                if(overW != NULL) {
                    *overW = overwriteStart(newLine->next, *text);
                } else {
                    overwriteStart(newLine->next, *text);
                }
                newLine = newLine->next;
            }
        }
    }
    return newLine;
}

//Delete a line in the text list.
struct textPile *deleteStart(struct textPile *line, int line1, char **text, int lineCounter) {
    struct textPile *temp = NULL;
    if(pile->first != NULL) {
        if (line == NULL) {
            line = search(line1);
        }
        if (line != NULL) {
            temp = line->prev;
            if (temp != NULL) {
                temp->next = line->next;
                if (line->next != NULL) {
                    line->next->prev = temp;
                }
            } else {
                pile->first = line->next;
                temp = line->next;
                if (temp != NULL) {
                    temp->prev = NULL;
                }
            }
            if(text != NULL) {
                *text = line->text;
            }
            temp = line->next;
            if (pileSize + lineCounter - 1 == line1) {
                pile->last = line->prev;
            }
            free(line);
            pileSize--;
        }
    }
    return temp;
}

//Insert a line in the text list pushing the following lines.
struct textPile *push(struct textPile *newLine, char **text, int line) {
    struct textPile *temp = NULL;
    if(pile->first == NULL) {
        newLine = malloc(sizeof(struct textPile));
        newLine = baseInsert(pile->first, pile->last, newLine);
        newLine->text = *text;
        pile->first = newLine;
        pile->last = newLine;
        pileSize++;
        return newLine;
    } else {
        if(line == pileSize + 1) {
            temp = pile->last;
            newLine = malloc(sizeof(struct textPile));
            newLine->next = temp->next;
            temp->next = newLine;
            newLine->prev = temp;
            pile->last = newLine;
            newLine->text = *text;
            pileSize++;
            return newLine;
        } else {
            if(newLine == NULL) {
                temp = search(line);
                newLine = malloc(sizeof(struct textPile));
                if(temp->prev == NULL) {
                    pile->first = newLine;
                }
                baseInsert(temp->prev, temp, newLine);
                newLine->text = *text;
                pileSize++;
                return newLine;
            } else {
                temp = malloc(sizeof(struct textPile));
                baseInsert(newLine, newLine->next, temp);
                temp->text = *text;
                pileSize++;
                return temp;
            }
        }
    }
}

//insert an operation line in the undo pile.
void insertUndo(char command, int line1, int line2, char **text, char **overW, int oldPileSize) {
    struct undoPile *temp;
    temp = malloc(sizeof(struct undoPile));
    temp->command = command;
    temp->line1 = line1;
    temp->line2 = line2;
    if(text != NULL) {
        temp->text = text;
    }
    if(command != 'd') {
        if (line1 <= pileSize) {
            temp->overW = overW;
            temp->line3 = oldPileSize;
        } else {
            temp->overW = NULL;
            temp->line3 = 0;
        }
    } else {
        temp->overW = overW;
        temp->line3 = 0;
    }
    temp->next = undo->first;
    temp->prev = NULL;
    if(temp->next != NULL) {
        temp->next->prev = temp;
    }
    undo->first = temp;
    if(undo->cellNum == 0) {
        undo->last = temp;
    }
    undo->cellNum += 1;
}

//insert an operation in the undo pile from the redo pile.
void insertUndoFromRedo(struct undoPile *temp) {
    temp->next = undo->first;
    temp->prev = NULL;
    if(temp->next != NULL) {
        temp->next->prev = temp;
    }
    if(undo->cellNum == 0) {
        undo->last = temp;
    }
    undo->first = temp;
    undo->cellNum++;
}

//insert an operation in the redo pile.
void insertRedo(struct undoPile *temp) {
    temp->next = NULL;
    if(redo->cellNum != 0) {
        temp->prev = redo->last;
    } else {
        redo->first = temp;
    }
    redo->last = temp;
    redo->cellNum++;
}

//remove an operation from the undo pile.
struct undoPile *removeUndo() {
    struct undoPile *temp = NULL;
    if(undo->cellNum != 0) {
        temp = undo->first;
        undo->first = undo->first->next;
        undo->cellNum--;
        if(undo->cellNum == 0) {
            undo->last = NULL;
        }
    }
    return temp;
}

//Remove an operation from the redo pile.
struct undoPile *removeRedo() {
    struct undoPile *temp = NULL;
    if(redo->cellNum != 0) {
        temp = redo->last;
        redo->last = redo->last->prev;
        temp->next = NULL;
        redo->cellNum--;
        if(redo->cellNum == 0) {
            redo->first = NULL;
        }
    }
    return temp;
}

//Remove all operations from the redo pile
void freeRedo() {
    struct undoPile *temp;
    for(temp = redo->last; temp != NULL; temp = redo->last) {
        free(temp->overW);
        free(temp->text);
        redo->last = temp->prev;
        free(temp);
    }
    redo->first = NULL;
    redo->cellNum = 0;
}

//Remove all lines in the text list.
void freePile() {
    struct textPile *temp;
    for(temp = pile->first; pile->first != NULL; temp = pile->first) {
        pile->first = temp->next;
        free(temp);
    }
    pileSize = 0;
    pile->last = NULL;
}

//Cut the input lines.
void getLines(char **text) {
    char *buffer;
    buffer = strtok(NULL, "\n");
    *text = buffer;
}

//Change the current text between line1 and line2 (extremes included)
void change(int line1, int line2) {
    struct textPile *temp = NULL;
    char **text = malloc(sizeof(char *) * (line2 - line1 + 1));
    char **overW = malloc(sizeof(char *) * (line2 - line1 + 1));
    int oldPileSize = pileSize;
    int j;
    int lineCounter = line1;
    for(j = 0; j < line2 - line1 + 1; j++) {
        getLines(&text[j]);
        temp = insertStart(temp, &text[j], lineCounter, &overW[j]);
        lineCounter++;
    }
    insertUndo('c', line1, line2, text, overW, oldPileSize);
    if(redo->cellNum != 0) {
        freeRedo();
    }
}

//Delete lines between line1 and line2
void delete(struct textPile *cell, int line1, int line2, int isUndo) {
    struct textPile *temp;
    int i;
    int lineCounter = line1;
    int oldPileSize = pileSize;
    int tot = pileSize;
    if(line2 > tot) {
        line2 = tot;
    }
    if(line1 > pileSize) {
        line1 = 0;
        line2 = 0;
    }
    char **text = malloc(sizeof(char *) * (line2 - line1 + 1));
    if(line1 <= pileSize) {
        temp = cell;
        for (i = 0; i < line2 - line1 + 1; i++) {
            temp = deleteStart(temp, lineCounter, &text[i], line2 - line1 + 1);
            lineCounter++;
        }
    }
    if(isUndo == 0) {
        insertUndo('d', line1, line2, text, NULL, oldPileSize);
    }
    if(redo->cellNum != 0 && isUndo == 0) {
        freeRedo();
    }
}

//Undo a Change operation
void undoChange(struct undoPile *prev) {
    struct textPile *line = NULL;
    int i;
    int j;
    int line1 = prev->line1;
    int line2 = prev->line2;
    int line3 = prev->line3;
    if(prev->command == 'd') {
        if(line2 != 0) {
            for (j = 0; j < line2 - line1 + 1; j++) {
                line = push(line, &prev->text[j], line1 + j);
            }
        }
    } else {
        if(*prev->overW != NULL) {
            if(line3 < line2 && line3 != 0) {
                for(j = 0; j < line3 - line1 + 1; j++) {
                    line = insertStart(line, &prev->overW[j], line1 + j, NULL);
                }
                for(i = 0; i < line2 - line3 + 1; i++) {
                    delete(line->next, line3 + 1, line2, 1);
                }
            } else {
                for(j = 0; j < line2 - line1 + 1; j++) {
                    line = insertStart(line, &prev->overW[j], line1 + j, NULL);
                }
            }
        } else {
            delete(NULL, line2, line1, 1);
        }
    }
}

//Print lines between line1 and line2 included
void print(int line1, int line2) {
    struct textPile *temp;
    int i;
    int counter = 0;
    int lineCounter = line2 - line1 + 1;
    if(line1 == 0) {
        printf(".\n");
        counter++;
        line1 = 1;
    }
    if (pile->first == NULL) {
        for(i = 0; i < lineCounter - counter; i++) {
            printf(".\n");
            counter++;
        }
    } else {
        if(line1 > pileSize) {
            for(i = 0; i < lineCounter - counter; i++) {
                printf(".\n");
                counter++;
            }
        } else {
            for (i = line1, temp = search(line1); i <= line2 && temp != NULL; temp = temp->next, i++) {
                if (temp != NULL) {
                    printf("%s\n", temp->text);
                    counter++;
                    if (i == line2) {
                        break;
                    }
                }
            }
        }
    }
    if(counter < lineCounter) {
        for(i = 0; i < lineCounter - counter; i++) {
            printf(".\n");
        }
    }
}

//Undo a number of commands (change or delete) equal to num
void undoFunction(int num) {
    struct undoPile *temp;
    int i;
    if(num == undo->cellNum) {
        if(redo->cellNum == 0) {
            redo->first = undo->first;
            redo->last = undo->last;
            redo->cellNum = undo->cellNum;
            undo->first = NULL;
            undo->last = NULL;
            undo->cellNum = 0;
        } else {
            temp = redo->last;
            temp->next = undo->first;
            redo->last = undo->last;
            redo->cellNum += undo->cellNum;
            undo->first = NULL;
            undo->last = NULL;
            undo->cellNum = 0;
        }
        freePile();
    } else if(undo->cellNum - num > undo->cellNum / 4) {
        for (i = 0; i < num; i++) {
            if (undo->cellNum != 0) {
                temp = removeUndo();
                if (temp != NULL) {
                    if (temp->command == 'd') {
                        undoChange(temp);
                    } else if (undo->cellNum != 0) {
                        if (*temp->overW == NULL) {
                            delete(NULL, temp->line1, temp->line2, 1);
                        } else {
                            undoChange(temp);
                        }
                    } else {
                        delete(NULL, temp->line1, temp->line2, 1);
                    }
                } else {
                    break;
                }
                insertRedo(temp);
            }
        }
    } else {
        freePile();
        int undoNum = undo->cellNum;
        if(redo->cellNum == 0) {
            redo->first = undo->first;
            redo->last = undo->last;
            redo->cellNum = undo->cellNum;
            undo->first = NULL;
            undo->last = NULL;
            undo->cellNum = 0;
        } else {
            temp = redo->last;
            temp->next = undo->first;
            redo->last = undo->last;
            redo->cellNum += undo->cellNum;
            undo->first = NULL;
            undo->last = NULL;
            undo->cellNum = 0;
        }
        redoFunction(undoNum - num);
    }
}

//Cancels the effect of undo for a number of commands equal to num
void redoFunction(int num) {
    struct undoPile *temp;
    for(int i = 0; i < num; i++) {
        struct textPile *textP = NULL;
        temp = removeRedo();
        if (temp != NULL) {
                if (temp->command == 'c') {
                    for (int j = 0; j < temp->line2 - temp->line1 + 1; j++) {
                        textP = insertStart(textP, &temp->text[j], temp->line1 + j, NULL);
                    }
                } else {
                    if (temp->line2 != 0) {
                        for (int j = 0; j < temp->line2 - temp->line1 + 1; j++) {
                            textP = deleteStart(textP, temp->line1 + j, NULL, temp->line2 - temp->line1 + 1);
                        }
                    }
                }
            insertUndoFromRedo(temp);
        } else {
            break;
        }
    }
}

int main() {
    pile = malloc(sizeof(struct textPileHead));
    undo = malloc(sizeof(struct undoPileHead));
    redo = malloc(sizeof(struct undoPileHead));
    int totU;
    int totR;
    char command;
    char *p;
    int line1;
    int line2;
    char *string = NULL;
    char *unRe = NULL;
    pile->first = NULL;
    pile->last = NULL;
    undo->first = NULL;
    undo->last = NULL;
    undo->cellNum = 0;
    redo->first = NULL;
    redo->last = NULL;
    redo->cellNum = 0;
    fseek(stdin, 0, SEEK_END);
    long filesize = ftell(stdin);
    fseek(stdin, 0, SEEK_SET);
    char *buffer = malloc(sizeof(char) * (filesize + 1));
    fread(buffer,sizeof(char), filesize, stdin);
    do {
        p = NULL;
        line1 = 0;
        line2 = 0;
        if(unRe == NULL) {
            if (string == NULL)
                string = strtok(buffer, "\n");
            else
                string = strtok(NULL, "\n");
            if (string[0] != 'q') {
                line1 = (int) strtol(string, &p, 10);
                if (*p == ',') {
                    line2 = (int) strtol(p + 1, &p, 10);
                    command = *p;
                } else {
                    command = *p;
                }
            } else {
                command = string[0];
            }
        } else {
            if (unRe[0] != 'q') {
                string =  unRe;
                line1 = (int) strtol(string, &p, 10);
                line2 = (int) strtol(p + 1, &p, 10);
                command = *p;
                unRe = NULL;
            } else {
                command = 'q';
            }
        }
        switch (command) {
            case 'c':
                change(line1, line2);
                buffer = strtok(NULL, "\n");
                break;
            case 'd':
                delete(NULL, line1, line2, 0);
                break;
            case 'p':
                print(line1, line2);
                break;
            case 'u':
                totU = undo->cellNum;
                totR = redo->cellNum;
                if(line1 > totU) {
                    totR += totU;
                    totU = 0;
                } else {
                    totU -= line1;
                    totR += line1;
                }
                do {
                    string = strtok(NULL, "\n");
                    if(string[0] == 'q') {
                        unRe = string;
                        break;
                    }
                    line1 = (int) strtol(string, &p, 10);
                    if(*p == ',') {
                        unRe = string;
                        break;
                    } else {
                        command = *p;
                        if(command == 'u') {
                            if(line1 > totU) {
                                totR += totU;
                                totU = 0;
                            } else {
                                totU -= line1;
                                totR += line1;
                            }
                        } else {
                            if(line1 > totR) {
                                totU += totR;
                                totR = 0;
                            } else {
                                totR -= line1;
                                totU += line1;
                            }
                        }
                    }
                } while (1 == 1);
                if(undo->cellNum - totU > 0) {
                    undoFunction(undo->cellNum - totU);
                } else if(redo->cellNum - totR > 0) {
                    redoFunction(redo->cellNum - totR);
                }
                break;
            case 'r':
                totU = undo->cellNum;
                totR = redo->cellNum;
                if(line1 > totR) {
                    totU += totR;
                    totR = 0;
                } else {
                    totR -= line1;
                    totU += line1;
                }
                do {
                    string = strtok(NULL, "\n");
                    if(string[0] == 'q') {
                        unRe = string;
                        break;
                    }
                    line1 = (int) strtol(string, &p, 10);
                    if(*p == ',') {
                        unRe =  string;
                        break;
                    } else {
                        command = *p;
                        if(command == 'u') {
                            if(line1 > totU) {
                                totR += totU;
                                totU = 0;
                            } else {
                                totU -= line1;
                                totR += line1;
                            }
                        } else {
                            if(line1 > totR) {
                                totU += totR;
                                totR = 0;
                            } else {
                                totR -= line1;
                                totU += line1;
                            }
                        }
                    }
                } while (1 == 1);
                if(undo->cellNum - totU > 0) {
                    undoFunction(undo->cellNum - totU);
                } else if(redo->cellNum - totR > 0) {
                    redoFunction(redo->cellNum - totR);
                }
                break;
            case 'q':
                break;
            default:
                return -1;
        }
    } while(command != 'q');
}

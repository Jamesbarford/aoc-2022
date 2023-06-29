#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define isTerm(ch) ((ch) == '\0' || (ch) == '\n')

#define DIR_MAX_NAME_LEN 64
#define DIR_MAX_DEPTH    32
#define DIR_MAX_FILES    100

#define DEV_FILE         0
#define DEV_DIR          1

#define ELF_SYS_SIZE 70000000
#define ELF_ENTRY_STACKSIZE 1000

typedef struct elfFile {
    char name[DIR_MAX_NAME_LEN];
    unsigned long size;
} elfFile;

typedef struct elfEntry {
    char name[DIR_MAX_NAME_LEN];
    int subdirs;
    int filecount;
    unsigned long sizeexl_sub;
    unsigned long sizeinc_sub;
    struct elfEntry *parent;
    struct elfEntry *children[DIR_MAX_DEPTH];
    struct elfFile files[DIR_MAX_DEPTH];
} elfEntry;

elfEntry *elfEntryNew(elfEntry *parent, char *name) {
    elfEntry *e = malloc(sizeof(elfEntry));
    e->sizeexl_sub = 0;
    e->sizeinc_sub = 0;
    e->subdirs = 0;
    e->parent = parent;
    e->filecount = 0;
    strncpy(e->name, name, DIR_MAX_NAME_LEN);
    memset(e->children, 0, DIR_MAX_DEPTH);
    return e;
}

void elfEntryAddFile(elfEntry *e, char *filename, unsigned long size) {
    elfFile *f = &e->files[e->filecount++];
    f->size = size;
    strncpy(f->name, filename, DIR_MAX_NAME_LEN);
}

elfEntry *elfEntryCreateFileSystem(char *filename) {
    FILE *fp = fopen(filename, "r");
    char buffer[10000];
    elfEntry *root = elfEntryNew(NULL, "/");
    elfEntry *next, *cd;
    cd = root;

    char tmpname[DIR_MAX_NAME_LEN];

    while (fgets(buffer, sizeof(buffer), fp)) {
        char *ptr = buffer;
        int i = 0;

        if (*ptr == '$') {
            ptr += 2;
            if (!strncmp(ptr, "cd", 2)) {
                ptr += 3;
                if (!strncmp(ptr, "..", 2)) {
                    cd->parent->sizeinc_sub += cd->sizeinc_sub;
                    cd = cd->parent;
                } else if (*ptr == '/') { 
                    /* We're pointing at the root, which is the first command so ignore */
                    continue;
                } else {
                    while (!isTerm(*ptr)) {
                        tmpname[i++] = *ptr++;
                    }
                    tmpname[i] = '\0';
                    next = elfEntryNew(cd,tmpname);
                    cd->children[cd->subdirs++] = next;
                    cd = next;
                }
            } else if (!strncmp(ptr, "ls", 2)) {
                continue;
            }
        } else {
            /* We are now in the output of ls */
            if (isdigit(*ptr)) {
                int collect = 0;
                while (isdigit(*ptr)) {
                    collect = collect * 10 + *ptr++ - '0';
                }
                if (*ptr == ' ') {
                    ptr++;
                    i=0;
                    while (!isTerm(*ptr)) {
                        tmpname[i++] = *ptr++;
                    }
                    tmpname[i] = '\0';
                    elfEntryAddFile(cd,tmpname,collect);
                }
                cd->sizeexl_sub += collect;
                cd->sizeinc_sub += collect;
            } else if (!strncmp(ptr, "dir", 3)) {
                continue;
            }
        }
    }

    fclose(fp);

    while (cd != root) {
        cd->parent->sizeinc_sub += cd->sizeinc_sub;
        cd = cd->parent;
    }
    return root;
}

void elfFileSystemPrint(elfEntry *e, int indent) {
    printf("%*s - %s (dir, total=%lu, size=%lu) \n",
            indent, " ", e->name,e->sizeinc_sub,e->sizeexl_sub);
    if (e->subdirs > 0) {
        for (int i = 0; i < e->subdirs; ++i) {
            elfFileSystemPrint(e->children[i], indent + 4);
        }
    }
    if (e->filecount > 0) {
        for (int i = 0; i < e->filecount; ++i) {
            printf("%*s - %s (file, size=%lu)\n", indent + 2, " ",
                    e->files[i].name, e->files[i].size);
        }
    }
}

unsigned long solve1(elfEntry *root, int limit) {
    unsigned long accum = 0;
    int sp = 0; /* stackpointer */
    elfEntry *stack[ELF_ENTRY_STACKSIZE];
    elfEntry *ptr = NULL;
    stack[sp++] = root;

    while (sp != 0 && (ptr = stack[--sp])) {
        if (ptr->sizeinc_sub < limit) {
            accum += ptr->sizeinc_sub;
        }

        for (int i = 0; i < ptr->subdirs; ++i) {
            stack[sp++] = ptr->children[i];
        }
    }
    return accum;
}

unsigned long solve2(elfEntry *root, unsigned long tofree) {
    unsigned long dirsize = ULONG_MAX;
    int sp = 0;
    elfEntry *stack[ELF_ENTRY_STACKSIZE];
    elfEntry *ptr = NULL;
    stack[sp++] = root;

    while (sp && (ptr = stack[--sp])) {
        if (ptr->sizeinc_sub >= tofree) {
            if (ptr->sizeinc_sub < dirsize) {
                dirsize = ptr->sizeinc_sub;
            }
        }

        for (int i = 0; i < ptr->subdirs; ++i) {
            stack[sp++] = ptr->children[i];
        }
    }

    return dirsize;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    elfEntry *root = elfEntryCreateFileSystem(argv[1]);
    elfFileSystemPrint(root, 0);
    unsigned long size = solve1(root, 100000);
    unsigned long tofree = 30000000 - (ELF_SYS_SIZE - root->sizeinc_sub);
    unsigned long dirsize = solve2(root,tofree);
    printf("pt1: %lu\n", size);
    printf("pt2: %lu\n", dirsize);
}

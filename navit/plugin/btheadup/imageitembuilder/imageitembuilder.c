/*
 ============================================================================
 Name        : imageitembuilder.c
 Author      : Olaf Brandt, olf@eisenzelt.de
 Version     : 1.0
 Copyright   : Public Domain
 Description : Create a subset of itemdef.h including all nav_xyz items
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "item_def.h"

enum item_type {
#define ITEM2(x,y) type_##y=x,
#define ITEM(x) type_##x,
#include "item_def.h"
#undef ITEM2
#undef ITEM
};

struct item_name {
    enum item_type item;
    char *name;
};

struct item_name item_names[]= {
#define ITEM2(x,y) ITEM(y)
#define ITEM(x) { type_##x, #x },
#include "item_def.h"
#undef ITEM2
#undef ITEM
};

int main(void) {

    FILE *fptr, *fptr1;
    fptr = fopen("imageitem_def.h","w+");
    fptr1 = fopen("imageheaders.h","w+");

    fprintf(fptr,
            "#ifndef ITEM\n#define ITEM(x) extern ##x;\n#endif\n#ifndef ITEM2\n#define ITEM2(x,y) extern ##y;\n#endif\n\n");

    for (int i = 0; i<sizeof(item_names)/sizeof(struct item_name) ; i++) {
        if(strstr(item_names[i].name, "nav_") && strcmp(item_names[i].name, "nav_none")
                && strcmp(item_names[i].name, "nav_turnaround")&& strcmp(item_names[i].name, "nav_position")) {
            fprintf(fptr, "ITEM2(%i, %s)\n", item_names[i].item, item_names[i].name);
            fprintf(fptr1, "#include \"images/%s_bk.h\"\n", item_names[i].name);
        }
    }

    fclose(fptr);
    fclose(fptr1);

    return EXIT_SUCCESS;
}

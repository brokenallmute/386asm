#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/asm386.h"

/* Add label to symbol table (pass 1 only) */
void
add_label(const char *name, uint32_t address)
{
	if (asm_ctx.pass != 1)
		return;

	if (asm_ctx.label_count >= MAX_LABELS) {
		fprintf(stderr, "error: too many labels\n");
		exit(1);
	}

	label_t *label = &asm_ctx.labels[asm_ctx.label_count++];
	strncpy(label->name, name, sizeof(label->name) - 1);
	label->name[sizeof(label->name) - 1] = '\0';
	label->address = asm_ctx.origin + address;
}

/* Find label address by name */
int
find_label(const char *name, uint32_t *address)
{
	for (int i = 0; i < asm_ctx.label_count; i++) {
		if (strcmp(asm_ctx.labels[i].name, name) == 0) {
			*address = asm_ctx.labels[i].address;
			return 1;
		}
	}
	return 0;
}

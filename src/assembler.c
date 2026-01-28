#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/asm386.h"

/* Global assembler context */
assembler_t asm_ctx;

/* Process single line of assembly code */
void
process_line(char *line)
{
	/* Remove comments */
	char *comment = strchr(line, ';');
	if (comment)
		*comment = '\0';

	char *p = skip_whitespace(line);
	if (*p == '\0')
		return;

	/* Check for label (ends with ':') */
	char *colon = strchr(p, ':');
	if (colon) {
		*colon = '\0';
		char label[64];
		strncpy(label, p, sizeof(label) - 1);
		label[sizeof(label) - 1] = '\0';

		/* Remove trailing whitespace from label */
		char *end = label + strlen(label) - 1;
		while (end > label && isspace(*end))
			*end-- = '\0';

		add_label(label, asm_ctx.code_pos);
		
		/* Continue with rest of line */
		p = colon + 1;
		p = skip_whitespace(p);
		if (*p == '\0')
			return;
	}

	/* Check for directive (starts with '.') */
	if (*p == '.') {
		char directive[64];
		p = parse_token(p, directive, sizeof(directive));
		process_directive(directive, p);
		return;
	}

	/* Parse instruction mnemonic */
	char mnemonic[64];
	p = parse_token(p, mnemonic, sizeof(mnemonic));

	/* Convert mnemonic to lowercase */
	for (char *c = mnemonic; *c; c++)
		*c = tolower(*c);

	assemble_instruction(mnemonic, p);
}

/* Assemble file (called twice: pass 1 and pass 2) */
void
assemble_file(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "error: cannot open file '%s'\n", filename);
		exit(1);
	}

	char line[MAX_LINE];
	while (fgets(line, sizeof(line), fp)) {
		process_line(line);
	}

	fclose(fp);
}

/* Write assembled code to output file */
void
write_output(const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		fprintf(stderr, "error: cannot create output file '%s'\n", filename);
		exit(1);
	}

	fwrite(asm_ctx.code, 1, asm_ctx.code_pos, fp);
	fclose(fp);
}

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

	char *colon = strchr(p, ':');
	if (colon) {
		char *space_before = p;
		while (space_before < colon && !isspace(*space_before))
			space_before++;
		
		/* If we reached colon without spaces, and colon is followed by whitespace or end, it's a label */
		if (space_before == colon && (colon[1] == '\0' || isspace(colon[1]) || colon[1] == '\n' || colon[1] == '\r')) {
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

	/* Check and remove size prefix from operands */
	char *operands_start = p;
	p = skip_whitespace(p);
	
	/* Reset explicit size */
	asm_ctx.explicit_size = 0;
	
	/* Remove "byte ptr", "word ptr", "dword ptr" or just "byte", "word", "dword" */
	if (strncasecmp(p, "byte ", 5) == 0) {
		asm_ctx.explicit_size = 8;
		p += 5;
		p = skip_whitespace(p);
		if (strncasecmp(p, "ptr ", 4) == 0) {
			p += 4;
			p = skip_whitespace(p);
		}
		operands_start = p;
	} else if (strncasecmp(p, "word ", 5) == 0) {
		asm_ctx.explicit_size = 16;
		p += 5;
		p = skip_whitespace(p);
		if (strncasecmp(p, "ptr ", 4) == 0) {
			p += 4;
			p = skip_whitespace(p);
		}
		operands_start = p;
	} else if (strncasecmp(p, "dword ", 6) == 0) {
		asm_ctx.explicit_size = 32;
		p += 6;
		p = skip_whitespace(p);
		if (strncasecmp(p, "ptr ", 4) == 0) {
			p += 4;
			p = skip_whitespace(p);
		}
		operands_start = p;
	}

	assemble_instruction(mnemonic, operands_start);
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

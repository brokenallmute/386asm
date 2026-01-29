#include <stdio.h>
#include <string.h>
#include "../include/asm386.h"

int
main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <input.asm> <output.bin>\n", argv[0]);
		return 1;
	}

	/* Initialize assembler context */
	memset(&asm_ctx, 0, sizeof(asm_ctx));

	/* Pass 1: collect labels and calculate addresses */
	asm_ctx.pass = 1;
	asm_ctx.code_pos = 0;
	asm_ctx.origin = 0;
	assemble_file(argv[1]);

	/* Pass 2: generate actual machine code */
	asm_ctx.pass = 2;
	asm_ctx.code_pos = 0;
	asm_ctx.origin = 0;  /* Reset origin for pass 2 */
	assemble_file(argv[1]);

	/* Write output binary */
	write_output(argv[2]);

	printf("assembled %d bytes\n", asm_ctx.code_pos);
	return 0;
}

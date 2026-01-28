#ifndef ASM386_H
#define ASM386_H

#include <stdint.h>

#define MAX_LINE 1024
#define MAX_LABELS 1024
#define MAX_CODE 65536

typedef struct {
	char name[64];
	uint32_t address;
} label_t;

typedef struct {
	label_t labels[MAX_LABELS];
	int label_count;
	uint8_t code[MAX_CODE];
	uint32_t code_pos;
	uint32_t origin;
	int pass;
} assembler_t;

typedef enum {
	OPERAND_NONE,
	OPERAND_REG,
	OPERAND_IMM,
	OPERAND_MEM
} operand_type_t;

typedef struct {
	operand_type_t type;
	int reg;
	int base;
	int index;
	int scale;
	int32_t disp;
	uint32_t imm;
	int size;
} operand_t;

extern assembler_t asm_ctx;

/* emit - code emission */
void emit_byte(uint8_t byte);
void emit_word(uint16_t word);
void emit_dword(uint32_t dword);
void emit_modrm(int mod, int reg, int rm);
void emit_sib(int scale, int index, int base);
void emit_memory_operand(int reg, operand_t *mem);

/* labels - label management */
void add_label(const char *name, uint32_t address);
int find_label(const char *name, uint32_t *address);

/* parser - parsing functions */
char *skip_whitespace(char *str);
char *parse_token(char *str, char *token, int max_len);
int parse_number(const char *str, uint32_t *value);
int parse_operand(const char *str, operand_t *op);

/* registers - register handling */
int is_register(const char *token);
int is_segment_register(const char *token);
int get_register_code(const char *token);
int get_register_size(const char *token);
int get_segment_register_code(const char *token);

/* instructions - instruction assembly */
void assemble_instruction(char *mnemonic, char *operands);

/* directives - assembler directives */
void process_directive(char *directive, char *operands);

/* assembler - main assembly logic */
void process_line(char *line);
void assemble_file(const char *filename);
void write_output(const char *filename);

#endif

# 386asm - Simple assembler compiler for i386/8086

Minimalist assembler for i386/8086 architecture written in C (1671 lines). Supports all basic instructions and addressing modes needed for bootloaders, kernel code and simple programs.

Supports opcodes: `mov`, `push`, `pop`, `lea`, `add`, `sub`, `inc`, `dec`, `mul`, `imul`, `div`, `idiv`, `neg`, `xor`, `and`, `or`, `cmp`, `test`, `not`, `shl`, `shr`, `jmp`, `je`, `jz`, `jne`, `jnz`, `jl`, `jg`, `jle`, `jge`, `ja`, `jb`, `call`, `int`, `nop`, `hlt`, `ret`, `cli`, `sti`, `cld`, `std`.

Addressing modes: `[reg]`, `[reg+offset]`, `[reg+reg]`, `[reg+reg*scale]`, `[reg+reg*scale+offset]`.

Directives: `.org`, `.db`, `.dw`, `.dd`, `.align`, `.times`.

Registers: 8-bit (`al`, `ah`, `bl`, `bh`, `cl`, `ch`, `dl`, `dh`), 16-bit (`ax`, `bx`, `cx`, `dx`, `sp`, `bp`, `si`, `di`), 32-bit (`eax`, `ebx`, `ecx`, `edx`, `esp`, `ebp`, `esi`, `edi`), segment (`cs`, `ds`, `es`, `fs`, `gs`, `ss`).

## Features

- Two-pass assembly for forward label references
- Character literals (`'A'`, `'B'`, etc.)
- Hexadecimal numbers (`0x1234`, `1234h`)
- Decimal numbers
- Label support with automatic offset calculation
- Short and long jump optimization
- Memory operands with displacement and scaling
- Full 8/16/32-bit register support

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Install

```bash
sudo cp build/asm386 /usr/local/bin/
```

## Usage

```bash
asm386 input.asm output.bin
```

## License

MIT

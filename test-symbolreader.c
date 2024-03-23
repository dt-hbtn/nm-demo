#include <stdio.h>
#include <stdlib.h>

#include "elfmacros.h"
#include "symbolreader.h"

#define ELF_FILE_PATH "elf_files/ubuntu64"

static void
print_value_and_str(SymbolReader *reader, Elf64_Sym *symbol);

/**
 * main - Entry point for symbol-reading demo
 *
 * Return: `0` for success, `1` for failure
 */
int main(void)
{
	SymbolReader reader;

	/* Initialize the `SymbolReader` */
	if (SymbolReader_Init(&reader, ELF_FILE_PATH) == -1) {
		fprintf(stderr, "Unable to parse file '%s'\n", ELF_FILE_PATH);
		exit(EXIT_FAILURE);
	}

	/* Process all symbol-table entries with `print_value_and_str` */
	SymbolReader_ProcessSymbols(&reader, print_value_and_str, NULL);

	/* Release resources held for processing the ELF file */
	SymbolReader_Destroy(&reader);

	return (0);
}

/**
 * print_value_and_str - Callback for `SymbolReader_ProcessSymbols`
 * @reader: Pointer to `SymbolReader` struct
 * @sym: Pointer to symbol-table entry
 */
static void
print_value_and_str(SymbolReader *reader, Elf64_Sym *symbol)
{
	const char *str = SYMBOL_STRING(reader->str_table, symbol);
	printf("%016lx %s\n", symbol->st_value, str);
}
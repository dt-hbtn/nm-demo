#ifndef _ATLAS_NMDEMO_SYMBOLREADER_H
#define _ATLAS_NMDEMO_SYMBOLREADER_H

#include <elf.h>
#include <stdint.h>

/**
 * struct SymbolReader - Maintains ELF-file addresses for printing symbols
 * @ehdr: Pointer to ELF-file header (first byte of file)
 * @shdrs: Pointer to first section header
 * @sym_sh: Pointer to symbol-table section header
 * @sym_table: Pointer to start of symbol table
 * @sym_count: Number of symbols in symbol table
 * @str_table: Pointer to start of string table
 * @fsize: Size of ELF file in bytes
 * @fd: Files descriptor of ELF file
 */
typedef struct SymbolReader {
	Elf64_Ehdr *ehdr;
	Elf64_Shdr *shdrs;
	Elf64_Shdr *sym_sh;
	Elf64_Sym *sym_table;
	uint64_t sym_count;
	char *str_table;
	uint64_t fsize;
	int fd;
} SymbolReader;

/**
 * sym_action_t - Function pointer for actions on symbol-table entries
 * @reader: Pointer to `SymbolReader` structure
 * @symbol: Pointer to symbol-table entry
 */
typedef void (*sym_action_t)(SymbolReader *reader, Elf64_Sym *symbol);

/**
 * sym_filter_t - Function pointer to filter symbol-table entries
 * @reader: Pointer to `SymbolReader` structure
 * @symbol: Pointer to symbol-table entry
 *
 * Return: Non-`0` = process entry, `0` = do not process entry
 */
typedef int (*sym_filter_t)(SymbolReader *reader, Elf64_Sym *symbol);

/**
 * SymbolReader_Init - Initialize `SymbolReader` struct
 * @reader: Pointer to `SymbolReader` struct
 * @fpath: Path to ELF file
 *
 * Return: `0` on success, `-1` on failure
 */
int
SymbolReader_Init(SymbolReader *reader, const char *fpath);

/**
 * SymbolReader_ProcessSymbols - Sequentially performs `action` on all symbols
 * @reader: Pointer to `SymbolReader` structure
 * @action: Pointer to symbol-processing function
 * @filter: Pointer to optional filter function, symbol processed when true
 */
void
SymbolReader_ProcessSymbols(SymbolReader *reader, sym_action_t action,
	sym_filter_t filter);

/**
 * SymbolReader_Destroy - Releases resources held by `SymbolReader` struct
 * @reader: Pointer to `SymbolReader` structure
 */
void
SymbolReader_Destroy(SymbolReader *reader);

#endif /* _ATLAS_NMDEMO_SYMBOLREADER_H */

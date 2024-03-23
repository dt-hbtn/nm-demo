#ifndef _ATLAS_NMDEMO_ELFMACROS_H
#define _ATLAS_NMDEMO_ELFMACROS_H

#include <elf.h>
#include <stdint.h>

/**
 * OFFSET - Moves a pointer an arbitrary number of bytes
 * @ptr: Pointer to offset
 * @n: Number of bytes to offset by
 *
 * Return: Pointer `ptr` offset by `n` bytes
 */
#define OFFSET(ptr, n) ((char *)(ptr) + (n))

/**
 * SECTION_HEADERS - Fetches pointer to section headers
 * @ehdr: Pointer to ELF header
 *
 * Return: Pointer to first section header
 */
#define SECTION_HEADERS(ehdr) ((Elf64_Shdr *)OFFSET((ehdr), (ehdr)->e_shoff))

/**
 * SECTION_COUNT - Returns the number of section headers
 * @ehdr: Pointer to ELF header
 *
 * Return: Number of section headers
 */
#define SECTION_COUNT(ehdr) ((uint64_t)(ehdr)->e_shnum)

/**
 * SYMBOL_COUNT - Returns number of symbol-table entries
 * @sh: Pointer to section header for symbol table
 *
 * Return: Number of symbol-table entries
 */
#define SYMBOL_COUNT(sh) ((uint64_t)(sh)->sh_size / sizeof(Elf64_Sym))

/**
 * SECTION - Fetches pointer to section given a section header
 * @ehdr: Pointer to ELF header
 * @sh: Pointer to section header
 *
 * Return: Pointer to section (caller will need to cast)
 */
#define SECTION(ehdr, sh) OFFSET((ehdr), (sh)->sh_offset)

/**
 * SYMBOL_TABLE - Fetches typed pointer to symbol table
 * @ehdr: Pointer to ELF header
 * @sh: Pointer to section header for symbol table
 *
 * Return: Pointer to first symbol-table entry (`Elf64_Sym`)
 */
#define SYMBOL_TABLE(ehdr, sh) ((Elf64_Sym *)SECTION(ehdr, sh))

/**
 * STRING_TABLE - Fetches pointer to string table
 * @ehdr: Pointer to ELF header
 * @shdrs: Pointer to first section header
 * @sh: Pointer to section header for symbol table
 *
 * Return: Pointer to first `char` of string table
 */
#define STRING_TABLE(ehdr, shdrs, sh) SECTION(ehdr, (shdrs) + (sh)->sh_link)

/**
 * SYMBOL_STRING - Fetches the string that corresponds to a symbol-table entry
 * @strtab: Pointer to string table
 * @sym: Pointer to symbol-table entry
 *
 * Return: Pointer to first `char` of the symbol's string
 */
#define SYMBOL_STRING(strtab, sym) ((strtab) + (sym)->st_name)

#endif /* _ATLAS_NMDEMO_ELFMACROS_H */

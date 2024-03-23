# nm-demo
Demonstration of reading entries from the symbol table of an ELF file (C/Linux).

## Overview
This will provide you a good start for reading symbols from an ELF file in C. However, since I was trying to just keep this focused on symbol enumeration, I should note the following:
- This example only supports 64-bit ELF files
- This example assumes the input is a well-formed ELF file (I do not validate the ELF header)
- I have implemented only bare-minimum error handling
- There are other types of symbol tables in ELF files (such as those for debug symbols) that are not accessed here, but this will show you how to find the symbols you need to mimic `nm -p FILE`

Feel free to use as much of this as you need.

## Structure
There are two header files in this repo, `elfmacros.h` and `symbolreader.h`. `elfmacros.h` is just macro functions that make working with the various ELF-type pointers more readable. `symbolreader.h` defines an interface works as follows:
- `SymbolReader` struct: Provides contextual data that is needed during the process of symbol enumeration
- `SymbolReader_Init()`: Does all of the mapping/parsing of the ELF file so that it's ready for symbol enumeration
- `SymbolReader_ProcessSymbols()`: Iterates through the symbol table and passes each symbol-table entry to the caller-provided function pointer (for easy modification of functionality). It also takes an optional `filter` function pointer to define conditions under which symbol-table entries should be processed or skipped
- `SymbolReader_Destroy()`: Releases all resources held by the `SymbolReader` instance

## Walkthrough
The code excerpts in this `README` are intended to give a high-level explanation of the focal steps, but you will probably need to look at the rest of the code, especially the static functions in `symbolreader.c` and the definitions of the macro functions in `elfmacros.h`.

### Initial ELF Parsing: `SymbolReader_Init()`
1. `open()` the ELF file and store the file descriptor
2. Use `mmap()` system call to map the file into virtual memory and store it as a pointer to an ELF header (see static function `map_elf_header()`)
3. Store pointers to the first section header and to the section header for the symbol table, then store the number of symbols in the symbol table (see static function `parse_symbol_header()`)
4. Store pointers to the symbol table and string tables (handled directly in `SymbolReader_Init()` with macros from `elfmacros.h`)

```c
/**
 * SymbolReader_Init - Initialize `SymbolReader` struct
 * @reader: Pointer to `SymbolReader` struct
 * @fpath: Path to ELF file
 *
 * Return: `0` on success, `-1` on failure
 */
int
SymbolReader_Init(SymbolReader *reader, const char *fpath)
{
	int fd;

	if (!reader || !fpath)
		return (-1);

	/* open the ELF file */
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		goto file_error;

	/* map ELF file into virtual memory */
	if (map_elf_header(reader, fd) == -1)
		goto map_error;

	/* find symbol section header and get symbol count */
	if (parse_symbol_header(reader) == -1)
		goto sym_table_error;

	/* link symbol and string tables */
	reader->sym_table = SYMBOL_TABLE(reader->ehdr, reader->sym_sh);
	reader->str_table = STRING_TABLE(reader->ehdr, reader->shdrs,
		reader->sym_sh);

	return (0);

sym_table_error:
	munmap(reader->ehdr, reader->fsize);

map_error:
	close(fd);

file_error:
	return (-1);
}
```

### Symbol Enumeration and Processing: `SymbolReader_ProcessSymbols()`
1. Iterate over the symbol-table entries
2. Use the caller provided `filter()` function to determine if an entry should be processed or not
3. Invoke the caller provided `action()` function to process entries for which `filter()` returns true

```c
/**
 * SymbolReader_ProcessSymbols - Sequentially performs `action` on all symbols
 * @reader: Pointer to `SymbolReader` structure
 * @action: Pointer to symbol-processing function
 * @filter: Pointer to optional filter function, symbol processed when true
 */
void
SymbolReader_ProcessSymbols(SymbolReader *reader, sym_action_t action,
	sym_filter_t filter)
{
	uint64_t i;

	if (!reader || !action)
		return;
	
	if (!filter)
		filter = dummy_filter;

	/* iterate over the symbol table entries */
	for (i = 0; i < reader->sym_count; ++i) {
		/* skip symbol if `filter` returns false */
		if (filter(reader, reader->sym_table + i)) {
			/* process symbol with `action` */
			action(reader, reader->sym_table + i);
		}
	}
}
```

### Cleanup: `SymbolReader_Destroy()`
1. Unmap the ELF file contents with the `munmap()` system call
2. `close()` the file descriptor
3. Zero-out the struct with `memset`

```c
/**
 * SymbolReader_Destroy - Releases resources held by `SymbolReader` struct
 * @reader: Pointer to `SymbolReader` structure
 */
void
SymbolReader_Destroy(SymbolReader *reader)
{
	if (!reader)
		return;

	munmap(reader->ehdr, reader->fsize);
	close(reader->fd);
	memset(reader, 0, sizeof(SymbolReader));
}
```

### Example Usage: Printing Symbol Value and String
The `test-symbolreader.c` file uses the `SymbolReader_*()` functions to print the value and string for each symbol in the symbol table of the provided example ELF file `ubuntu64`. Notice that I am not passing a filter function (it is `NULL`) and that `nm -p elf_files/ubuntu64` will skip some of the symbols printed here. You will have to figure out why `nm` is skipping those and filter them out based on certain criteria (as well as figure out how the second column, the symbol-type codes, are determined by `nm`).

```c
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
```

The `print_value_and_str()` function is defined in this file as well.

```c
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
```

OK, that's what I've got for you for now...good luck!

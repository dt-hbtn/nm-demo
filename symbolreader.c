#include <fcntl.h>	/* open */
#include <string.h>	/* memset */
#include <sys/mman.h>	/* mmap, munmap */
#include <sys/stat.h>	/* fstat */
#include <unistd.h>	/* close, */

#include "elfmacros.h"
#include "symbolreader.h"

static int
map_elf_header(SymbolReader *reader, int fd);

static int
parse_symbol_header(SymbolReader *reader);

static int
dummy_filter(SymbolReader *reader, Elf64_Sym *sym);

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

/**
 * SymbolReader_ProcessAll - Sequentially performs `action` on all symbols
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

	for (i = 0; i < reader->sym_count; ++i) {
		if (filter(reader, reader->sym_table + i))
			action(reader, reader->sym_table + i);
	}
}

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

/**
 * map_elf_header - Maps open ELF file into virtual memory
 * @reader: Pointer to `SymbolReader` struct
 * @fd: File descriptor for ELF file
 *
 * Return: `0` on success, `-1` on failure
 */
static int
map_elf_header(SymbolReader *reader, int fd)
{
	struct stat statbuf;

	/* track fd and get size of the ELF file */
	reader->fd = fd;

	if (fstat(reader->fd, &statbuf) == -1)
		return (-1);

	reader->fsize = (uint64_t)statbuf.st_size;

	/* map the ELF-file contents to a virtual-memory address */
	reader->ehdr = mmap(NULL, reader->fsize, PROT_READ, MAP_PRIVATE,
		reader->fd, 0);

	return (reader->ehdr == MAP_FAILED ? -1 : 0);
}

/**
 * parse_symbol_header - Finds symbol-table section header and symbol count
 * @reader: Pointer to `SymbolReader` struct
 *
 * Return: `0` on success, `-1` on failure
 */
static int
parse_symbol_header(SymbolReader *reader)
{
	uint64_t i;

	/* assign section header address and find symbol-table header */
	reader->shdrs = SECTION_HEADERS(reader->ehdr);
	reader->sym_sh = NULL;

	for (i = 0; i < SECTION_COUNT(reader->ehdr); ++i) {
		if (reader->shdrs[i].sh_type == SHT_SYMTAB) {
			reader->sym_sh = reader->shdrs + i;
			break;
		}
	}

	if (!reader->sym_sh)
		return (-1);

	reader->sym_count = SYMBOL_COUNT(reader->sym_sh);
	return (0);
}

/**
 * dummy_filter - Dummy default filter for `SymbolReader_ProcessSymbols`
 * @reader: Pointer to `SymbolReader` struct
 * @sym: Pointer to symbol-table entry
 *
 * Return: Always `1`
 */
static int
dummy_filter(SymbolReader *reader, Elf64_Sym *sym)
{
	(void)reader;
	(void)sym;
	return (1);
}

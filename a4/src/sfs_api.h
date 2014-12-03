#include "sfs_stdio.h" /* dependencies from external code (ugly!) */
#include "sfs_errno.h"
#include "sfs_dirent.h"

#include <stdint.h> /* uint16_t (C99) */

/* index to a spot on the disk */
typedef uint16_t FilePointer;
static const int fp_size = sizeof(FilePointer);
typedef uint16_t FileInteger;
static const int fi_size = sizeof(FileInteger);

/* versions of "hard disks" (files on disk) have a specific fp_size, block_size,
 and no_blocks: if you change these, you must start again with a new disk */

/* block size */
typedef struct { char data[256]; } Block;
static const int block_size = sizeof(Block) / sizeof(char);
static const int block_bits = (sizeof(Block) / sizeof(char)) << 3;

/* how big the is the disk: no_blocks * block_size bytes; bit vector bytes */
#define NO_BLOCKS (256)
static const int    no_blocks = NO_BLOCKS;
static const int no_blocks_bv = (NO_BLOCKS >> 3) + ((NO_BLOCKS & 0xFF) ? 1 : 0);

/* an entry in the file allocation table */
struct Fat {
	char *block; /* viz, block[block_size] */
	struct Fat *next;
};

#define FILENAME_SIZE 13 /* #define used in two initialisations;
                          [MAX_FNAME_LENGTH]; sizeof(8 + '.' + 3 + '\0') = 13 */

/* a file on disk */
struct File {
	char       isOpen;
	char       name[FILENAME_SIZE];
	int        size;
	struct Fat *start;
};
static const int filename_size = sizeof((struct File *)0)->name / sizeof(char);

/* an open file in memory, viz file pointer (FILE *) */
struct OpenFile {
	int         id; /* constant */
	int         inUse;
	char        name[FILENAME_SIZE]; /* a unique key (in practice, copy of File::name) */
	struct File *file;
	int         read;
	int         write;
};

struct DiskSfs {
	FileInteger dir_size;
	FilePointer dir;
	FileInteger fat_size;
	FilePointer fat;
	FileInteger free_total;
	FilePointer free;
};

/* the entire file system; this is taken from the first block: "The very first
 block is always the super block. This block will hold the number of blocks for
 the root directory (only one level directory here), number of blocks for the
 FAT, and the number of data blocks, and number of free blocks." */
struct Sfs {
	Block       free;
	/* the file system */
	int         dir_size, dir_memory, dir_next_memory;
	struct Dir  *dir;
	int         fat_size, fat_memory, fat_next_memory;
	struct Fat  *fat;
	/* this is used by readdir/rewinddir */
	int         next_file_index;
	/* the open files are entirely in memory */
	int         files_open;
	struct OpenFile *open[128]; /* [MAX_FD]; to do the tests, >= 100 */
	struct OpenFile *open_buffer;
};
static const int max_files_open = sizeof((struct Sfs *)0)->open / sizeof(struct OpenFile *);

int mksfs(const int fresh);
void rmsfs(void); /* <-- added this to clean up -Neil */

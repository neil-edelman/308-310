/* This is the simple file system used with disk_emu.c, etc; COMP-310.
 
 "The very first block is always the super block. This block will hold the
 number of blocks for the root directory (only one level directory here),
 number of blocks for the FAT, and the number of data blocks, and number of
 free blocks."
 
 Simulated file system using a file that is 'block_size * no_blocks' bits long.
 
 The first block has a FilePointer to the root directory, the FAT, and the
 used block table.
 
 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free bsearch */
#include <stdio.h>  /* fprintf EOF */
#include <string.h> /* strcmp memset */
#include <stdint.h> /* uint16_t (C99) */
#include "disk_emu.h"
#include "sfs_api.h"

/* private */

/* versions of "hard disks" (files on disk) have a specific fp_size, block_size,
 and no_blocks: if you change these, you must start again with a new disk */

/* index to a spot on the disk */
typedef uint16_t FilePointer;
static const int fp_size = sizeof(FilePointer);

/* block size */
typedef struct { char data[256]; } Block;
static const int block_size = sizeof(Block) / sizeof(char);
static const int block_bits = (sizeof(Block) / sizeof(char)) << 3;

/* how big the is the disk: no_blocks * block_size bytes; bit vector bytes */
#define NO_BLOCKS (256) /* #define used in two initialisations */
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

/* an open file in memory, viz file pointer, (FILE *) */
struct OpenFile {
	int         id; /* constant */
	int         inUse;
	char        name[FILENAME_SIZE]; /* a unique key (in practice, copy of File::name) */
	struct File *file;
};

/* the entire file system; this is taken from the first block: "The very first
 block is always the super block. This block will hold the number of blocks for
 the root directory (only one level directory here), number of blocks for the
 FAT, and the number of data blocks, and number of free blocks." */
struct Sfs {
	/* this is read from the disk: since the root is the only directory, I'm
	 making a simplifying assumption and combining it with the FAT; also, when
	 the assignment says "number of blocks for the FAT," I assume it means a
	 pointer to the FAT, etc. */
	FilePointer disk_fat;
	FilePointer disk_free;
	/* to compute this in the memory (uses Fibonacci) */
	int         fat_size, fat_memory, fat_next_memory;
	struct Fat  *fat;
	Block       free;            /* "the free block list can be contained within a single block," thank you! */
	/* the files are entirely in memory */
	int         files_open;
	struct OpenFile *open[128]; /* [MAX_FD]; to do the tests, >= 100 */
	struct OpenFile *open_buffer;
};
static const int max_files_open = sizeof((struct Sfs *)0)->open / sizeof(struct OpenFile *);

/* for looking up errors */
static const struct PrintError {
	enum SfsError key;
	char *why;
} errors[] = {
	{ ERR_NO,       "no error" },
	{ ERR_NOT_INIT, "sfs not initialised" },
	{ ERR_MALLOC,   "allocating space" },
	{ ERR_DISK,     "disk error" },
	{ ERR_MAX_FILES,"files open limit reached" },
	{ ERR_OUT_OF_BOUNDS, "out of bounds" },
	{ ERR_OPEN,     "opened already" },
	{ ERR_WTF,      "error" }
};
enum SfsError sfs_errno = ERR_NO;

static const char   *diskname = "foo";
static const int      verbose = -1; /* error checking in the supplied programmes
									is non-existant; we have to do it here */

/* private fuctions */
struct File *new_file(const char *name);
static int grow_fat(void);
static int err_cmp(const void *key, const void *elem);
static int openfile_cmp(const void *key, const void *elem);
static int free_query(const Block *free, const int block);
static int free_set(Block *free, const int block, const int isSet);
static int free_search(Block *free);
static void free_map(const Block *free, const char *pretty);
static void block_map(const Block *b, const char *pretty);

/* private; global variables automatically get initialised to zero */
static struct Sfs  *sfs;
static struct File *file_index;

/* public */



/* these provide functionality equivalent to <stdio.h> */



/** Initailises the file system.
 @param fresh
	Boolean value; build up a new file system instead of loading one from disk.
 @return Non-zero on success, if 0, sets sfs_error. */
int mksfs(const int fresh) {
	Block bl;
	void *read;
	int (*disk)(char *, int, int) = fresh ? &init_fresh_disk : &init_disk;
	int i;

	if(block_size < no_blocks_bv) {
		fprintf(stderr, "mksfs: we need a minimum of %db to hold free space list, but that won't fit in block size %db.\n", no_blocks_bv, block_size);
		exit(EXIT_FAILURE);
	}

	if(verbose) fprintf(stderr,
						"mksfs(%d:) block_size %d; block_bits %d; no_blocks %d; no_blocks_bv %d.\n",
						fresh, block_size, block_bits, no_blocks, no_blocks_bv);

	/* it's already loaded */
	if(sfs) return -1;

	/* allocate and fill with defaults */
	if(!(sfs = malloc(sizeof(struct Sfs) + max_files_open * sizeof(struct OpenFile)))) {
		if(verbose) perror("Sfs constructor");
		sfs_errno = ERR_MALLOC;
		rmsfs();
		return 0;
	}
	sfs->disk_free       = 0;
	sfs->disk_fat        = 0;
	memset(&sfs->free, 0, block_size);
	sfs->fat_size        = 0;
	sfs->fat_memory      = 0;
	sfs->fat_next_memory = 1;
	sfs->fat             = 0;
	/* pre-allocate max_files_open open files */
	sfs->files_open      = 0;
	sfs->open_buffer = (struct OpenFile *)(sfs + 1);
	for(i = 0; i < max_files_open; i++) {
		struct OpenFile *open = sfs->open[i] = &sfs->open_buffer[i];
		open->id         = i + 1;
		open->inUse      = 0;
	}
	/* fixme: just to test */
	fprintf(stderr, "test sfs->open[1] #%p = %d\n", (void *)sfs->open[1], sfs->open[1]->id);

	if(disk((char *)diskname, block_size, no_blocks) == -1) {
		if(verbose) fprintf(stderr, "Sfs: shutting down because of disk error #%p.\n", (void *)sfs);
		sfs_errno = ERR_DISK;
		rmsfs();
		return 0;
	}

	/* read in the superblock (0) */
	if(read_blocks(0, 1, (void *)bl.data) != 1) {                               /* <- calling disk_emu! */
		if(verbose) fprintf(stderr, "mksfs: couldn't read superblock.\n");
		sfs_errno = ERR_DISK;
		rmsfs();
		return 0;
	}
	/* ((FilePointer *)&bl)[0] = 5;
	((FilePointer *)&bl)[1] = 10; works! */
	block_map(&bl, "read in block 0");

	/* do the conversion to memory (THIS DEPENDS ON ENDIANESS!)
	 *(((FilePointer *)read)++) "warning: target of assignment not really an
	 lvalue; this will be a hard error in the future," but this is so long :[ */
	read = &bl;
	sfs->disk_fat  = *(FilePointer *)read;
	read           = (void *)((FilePointer *)read + 1);
	sfs->disk_free = *(FilePointer *)read;
	read           = (void *)((FilePointer *)read + 1);
	/* that's it! seems like a waste of space; maybe have the super-block also
	 contain the free vector; that would be smart . . . but maybe too simpifying */
	fprintf(stderr, "  ********* %d, %d\n", sfs->disk_fat, sfs->disk_free);

	/* want to deal with free block (static!) asap to check for discrepencies */
	if(sfs->disk_free != 0) {
		if(read_blocks(sfs->disk_free, 1, (void *)bl.data) != 1 || !free_query(&bl, 0)) { /* <- calling disk_emu! */
			if(verbose) fprintf(stderr, "mksfs: free blocks (%d) bad?\n", sfs->disk_free);
			sfs_errno = ERR_DISK;
			rmsfs();
			return 0;
		}
		memcpy(sfs->free.data, bl.data, sizeof(Block));
	} else {
		/* superblock is used */
		/*if(!free_query(&sfs->free, 0)) */free_set(&sfs->free, 0, -1);
	}

	/* if we have a FAT, load it and check for errors; the simplifying
	 assumption is that we're always going to have enough memory to store all
	 the FAT */
	if(sfs->disk_fat != 0) {
#if 0
		if(read_blocks(sfs->disk_fat, 1, (void *)bl.data) != 1) {                /* <- calling disk_emu! */
			if(verbose) fprintf(stderr, "mksfs: couldn't read fat, block %d.\n", sfs->disk_fat);
			sfs_errno = ERR_DISK;
			rmsfs();
			return 0;
		}
		read = &bl;
#else
		fprintf(stderr, "Not implemented.\n");
		exit(EXIT_FAILURE);
#endif
	}
	
	for(i = 0; i < 20; i++) {
		grow_fat();
	}

	free_map(&sfs->free, "blocks bv");
	sfs->files_open = 2;
	strcpy(sfs->open_buffer[0].name, "foo");
	strcpy(sfs->open_buffer[1].name, "bar");
	sfs->open[0] = &sfs->open_buffer[0];
	sfs->open[1] = &sfs->open_buffer[1];
	for(i = 0; i < sfs->files_open; i++) {
		fprintf(stderr, "[%d] %s\n", sfs->open[i]->id, sfs->open[i]->name);
	}
	fprintf(stderr, "Sorting.\n");
	qsort(sfs->open, sfs->files_open, sizeof(struct OpenFile *), &openfile_cmp);
	for(i = 0; i < sfs->files_open; i++) {
		fprintf(stderr, "[%d] %s\n", sfs->open[i]->id, sfs->open[i]->name);
	}
	
	return -1;
}

/** Destructor. */
void rmsfs(void) {
	if(!sfs) return;
	if(verbose) fprintf(stderr, "~Sfs: erase, #%p.\n", (void *)sfs);
	free(sfs);
	sfs = 0;
}

/** Prints a list of the files to stdout. */
void sfs_ls(void) {
	struct File *d;

	if(!sfs) return;
	while((d = readdir())) {
		printf("%d\t%s\t%s\n", d->size, d->isOpen ? "chilling" : "open", d->name);
	}
}

/* better to have the slow part on open
 file = &((struct OpenFile *)(sfs + 1))[sfs->files_open++];
 file->no = ++sfs->last_file_no;*/

/** Opens a file. "The fopen() function shall open the file whose pathname is
 the string pointed to by filename, and associates a stream with it."
 @param name
	The file name.
 @return "Upon successful completion, fopen() shall return a pointer to the
 object controlling the stream [in this case int]. Otherwise, a null pointer
 shall be returned [int 0], and errno shall be set to indicate the error." */
int sfs_fopen(const char *name) {
	struct OpenFile *open;
	struct File *already;
	int i;

	if(verbose) fprintf(stderr, "sfs_fopen(\"%s\");\n", name);
	if(!sfs) {
		if(verbose) fprintf(stderr, "sfs_fopen: ERR_NOT_INIT.\n");
		sfs_errno = ERR_NOT_INIT;
		return 0;
	} else if(sfs->files_open >= max_files_open) {
		if(verbose) fprintf(stderr, "sfs_fopen: ERR_MAX_FILES.\n");
		sfs_errno = ERR_MAX_FILES;
		return 0;
	}

	/* check wheather it's already an OpenFile */
	already = bsearch(name,         /* key */
					  sfs->open,    /* base, num, size */
					  sfs->files_open,
					  sizeof(struct File *),
					  &openfile_cmp /* comparator */);
	if(already) {
		if(verbose) fprintf(stderr, "sfs_fopen: ERR_OPEN.\n");
		sfs_errno = ERR_OPEN;
		return 0;
	} else {
		if(verbose) fprintf(stderr, "sfs_fopen: %s not already opened; we can open it!\n", name);
	}

	/* search the fat for the file */
	
#if 0
	already = bsearch(name,         /* key */
					  sfs->fat,     /* base, num, size */
					  sfs->files_open,
					  sizeof(struct File *),
					  &openfile_cmp /* comparator */);
	if(already) {
		fprintf(stderr, "******** Found %p.\n", (void *)already);
		return 0;
	}
#endif
	already = 0;

	/* find a spot (eww!) */
	for(i = 0; sfs->open[i]->inUse && (i < max_files_open); i++);
	/* just paranoid */
	if(i >= max_files_open) {
		fprintf(stderr, "sfs_fopen: discrepancy files_open %d but %d full.\n", sfs->files_open, max_files_open);
		abort();
	}
	open = sfs->open[i];
	if(verbose) fprintf(stderr, "sfs_fopen: found %d descriptor %d.\n", i, open->id);

	/* open the file */
	open->file = already ? already : new_file(name);
	if(!open->file) return 0;
	/* copy the key (filename) */
	strncpy(open->name, /* open->open->name */already ? already->name : name, filename_size - 1);
	open->name[filename_size - 1] = '\0';
	/* update the open files table */
	open->inUse = -1;
	sfs->files_open++;

	if(verbose) fprintf(stderr, "sfs_fopen: %s.\n", name);

	exit(0);
	return i + 1;
}

/** Closes a file and returns the resouces. "The fclose() function shall cause
 the stream pointed to by stream to be flushed and the associated file to be
 closed. Any unwritten buffered data for the stream shall be written to the
 file; any unread buffered data shall be discarded. Whether or not the call
 succeeds, the stream shall be disassociated from the file and any buffer set
 by the setbuf() or setvbuf() function shall be disassociated from the stream.
 If the associated buffer was automatically allocated, it shall be deallocated."
 @param fileID
	The file desciptor.
 @return "Upon successful completion, fclose() shall return 0; otherwise, it
 shall return EOF and set errno to indicate the error." */
int sfs_fclose(const int id) {
	if(!sfs) {
		if(verbose) fprintf(stderr, "sfs_fopen: ERR_NOT_INIT.\n");
		sfs_errno = ERR_NOT_INIT;
		return EOF;
	} else if(id <= 0 || max_files_open < id) {
		if(verbose) fprintf(stderr, "sfs_fopen: ERR_OUT_OF_BOUNDS.\n");
		sfs_errno = ERR_OUT_OF_BOUNDS;
		return EOF;
	}

	sfs->open[id - 1]->inUse = 0;
	return 0;
}

/** 
 @return "The fwrite() function shall return the number of elements
 successfully written, which may be less than nitems if a write error is
 encountered. If size or nitems is 0, fwrite() shall return 0 and the state of
 the stream remains unchanged. Otherwise, if a write error occurs, the error
 indicator for the stream shall be set, and errno shall be set to indicate the
 error." */
size_t sfs_fwrite(const int fileID, char *buf, const size_t length) {
	sfs_errno = ERR_WTF;
	return 0;
}

/** "The fread() function shall read into the array pointed to by ptr up to
 nitems elements whose size is specified by size in bytes, from the stream
 pointed to by stream. For each object, size calls shall be made to the
 fgetc() function and the results stored, in the order read, in an array of
 unsigned char exactly overlaying the object. The file position indicator for
 the stream (if defined) shall be advanced by the number of bytes successfully
 read. If an error occurs, the resulting value of the file position indicator
 for the stream is unspecified. If a partial element is read, its value is
 unspecified."
 @return "Upon successful completion, fread() shall return the number of
 elements successfully read which is less than nitems only if a read error or
 end-of-file is encountered. If size or nitems is 0, fread() shall return 0 and
 the contents of the array and the state of the stream remain unchanged.
 Otherwise, if a read error occurs, the error indicator for the stream shall be
 set, and errno shall be set to indicate the error." */
size_t sfs_fread(const int fileID, char *buf, size_t length) {
	sfs_errno = ERR_WTF;
	return 0;
}

/** "The fseek() function shall set the file-position indicator for the stream
 pointed to by stream. If a read or write error occurs, the error indicator for
 the stream shall be set and fseek() fails." This behaves like SEEK_SET.
 @return "The fseek() function shall return 0 if they succeed. Otherwise, they
 shall return -1 and set errno to indicate the error." */
int sfs_fseek(int fileID, int offset) {
	sfs_errno = ERR_WTF;
	return -1;
}

/** "The remove() function shall cause the file named by the pathname pointed
 to by path to be no longer accessible by that name. A subsequent attempt to
 open that file using that name shall fail, unless it is created anew."
 @return "Upon successful completion, 0 shall be returned. Otherwise, -1 shall
 be returned and errno set to indicate the error. If -1 is returned, the named
 file shall not be changed." */
int sfs_remove(const char *file) {
	sfs_errno = ERR_WTF;
	return -1;
}



/* these provide functionality equivalent to <dirent.h> */




/** "The readdir() function shall return a pointer to a structure representing
 the directory entry at the current position in the directory stream specified
 by the argument dirp, and position the directory stream at the next entry. It
 shall return a null pointer upon reaching the end of the directory stream."
 Since the files are being contained in a flat space, we don't need dirp.
 @return "Upon successful completion, readdir() shall return a pointer to an
 object of type struct dirent. When an error is encountered, a null pointer
 shall be returned and errno shall be set to indicate the error. When the end
 of the directory is encountered, a null pointer shall be returned and errno is
 not changed." */
struct File *readdir(void) {
	return 0;
}

/** "The rewinddir() function shall reset the position of the directory stream
 to which dirp refers to the beginning of the directory. It shall also cause
 the directory stream to refer to the current state of the corresponding
 directory, as a call to opendir() would have done. If dirp does not refer to a
 directory stream, the effect is undefined." Since the files are being
 contained in a flat space, we don't need dirp. Fork is undefined. */
void rewinddir(void) {
	
}



/* these provide functionality equivalent to <errno.h> */



/** Gets the human-readable string that corresponds to errnum.
 @param errnum
	The error.
 @return The error defined by errnum. */
char *sfs_strerror(const int errnum) {
	struct PrintError *e = bsearch(&errnum, /* key */
								   errors,  /* base, num, size */
								   sizeof(errors) / sizeof(struct PrintError),
								   sizeof(struct PrintError),
								   &err_cmp /* comparator */);
	return e ? e->why : "not a defined error";
}

/** Prints the last error that was generated.
 @param A string, "<string>: <error>.\n". */
void sfs_perror(const char *s) {
	if(!sfs) {
		fprintf(stderr, "%s: not initialised.\n", s);
		return;
	}
	fprintf(stderr, "%s: %s.\n", s, sfs_strerror(sfs_errno));
}



/* private */



/** creates a new file */
struct File *new_file(const char *name) {
	if(verbose) fprintf(stderr, "new_file: todo (%s.)\n", name);
	sfs_errno = ERR_WTF;
	return 0;
}

/** uses a Fibonacci thing */
static int grow_fat(void) {
	struct Fat *fat;
	int prev =  sfs->fat_memory;
	int   *a = &sfs->fat_memory;
	int   *b = &sfs->fat_next_memory;
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
	*b += *a;
	/* avoids the 1, 1 */
	if(*a == prev) (*a)++;
	if(!(fat = realloc(sfs->fat, sizeof(struct Fat) * sfs->fat_memory))) {
		if(verbose) fprintf(stderr, "grow_fat: ERR_MALLOC.\n");
		sfs_errno = ERR_MALLOC;
		return 0;
	}
	sfs->fat = fat;
	if(verbose) fprintf(stderr, "grow_fat: grew to %d entries.\n", sfs->fat_memory);
	return -1;
}

/** for bsearch */
static int err_cmp(const void *key, const void *elem) {
	return *(enum SfsError *)key - ((struct PrintError *)elem)->key;
}

/** for bsearch */
static int openfile_cmp(const void *key, const void *elem) {
	return strcmp(((struct OpenFile *)key)->name, ((struct OpenFile *)elem)->name);
}

/* @param free
	The space where all the free blocks are stored */
static int free_query(const Block *free, const int block) {
	char *fre = (char *)free;
	int byte = block >> 3;
	int bit  = 0x01 << (block & 0x07);

	if(block >= no_blocks || block < 0) {
		if(verbose) fprintf(stderr, "free: ERR_OUT_OF_BOUNDS.\n");
		sfs_errno = ERR_OUT_OF_BOUNDS;
		return 0;
	}

	return (fre[byte] & bit) ? -1 : 0;
}

static int free_set(Block *free, const int block, const int isSet) {
	char *fre = (char *)free;
	int byte = block >> 3;
	int bit  = 0x01 << (block & 0x07);

	/* debug: fprintf(stderr, "block %d, byte %d, bit %d\n", block, byte, block & 0x07); */
	if(block >= no_blocks || block >= block_size) {
		if(verbose) fprintf(stderr, "free: ERR_OUT_OF_BOUNDS.\n");
		sfs_errno = ERR_OUT_OF_BOUNDS;
		return 0;
	}
	if(verbose && (fre[byte] & bit) ? isSet : !isSet) {
		fprintf(stderr, "freeset: warning bit %d is already changed in free block list.\n", block);
		return -1;
	}
	if(isSet) {
		fre[byte] |= bit;
		if(verbose) fprintf(stderr, "freeset: set %d.\n", block);
	} else {
		fre[byte] &= ~bit;
		if(verbose) fprintf(stderr, "freeset: cleared %d.\n", block);
	}

	return -1;
}

/** Finds a block or returns 0. (First Fit -> Next Fit) */
static int free_search(Block *free) {
	fprintf(stderr, "Todo: free_search().\n");
	return 0;
}

/** debug */
static void free_map(const Block *free, const char *pretty) {
	char *fre = (char *)free;
	int i;
	char b;

	fprintf(stderr, "free_map: %s,\n", pretty);
	for(i = 0; i < no_blocks_bv; i++) {
		for(b = 0x01; b; b <<= 1) {
			fputc((fre[i] & b) ? '1' : '0', stderr);
			fputc(' ', stderr);
		}
		fputc('\n', stderr);
	}
	fprintf(stderr, "(done.)\n");
}

/** debug */
static void block_map(const Block *b, const char *pretty) {
	int i;

	fprintf(stderr, "block_map: %s,\n", pretty);
	for(i = 0; i < block_size; i++) {
		fprintf(stderr, "%2.2X ", b->data[i]);
		if((i & 0x0F) == 0x0F) fputc('\n', stderr);
	}
	fprintf(stderr, "(done.)\n");
}

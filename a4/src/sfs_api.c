/* This is the simple file system used with disk_emu.c; COMP-310.
 
 Todo: prevent overflow on all things.
 
 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free bsearch */
#include <stdio.h>  /* fprintf EOF */
#include <string.h> /* strcmp memset */
#include "disk_emu.h"
#include "sfs_errno.h"
#include "sfs_api.h"

const int            verbose = -1; /* error checking in the supplied programmes
								   is non-existant; we have to do it here */
static const char    *diskname = "foo";

/* global variables automatically get initialised to zero */
struct Sfs             *sfs;

/* private fuctions */
static int openfile_cmp(const void *key, const void *elem);
static void grow(int *a, int *b);
static int free_query(const Block *free, const int block);
static int free_set(Block *free, const int block, const int isSet);
static int free_search(Block *free);
static void free_map(const Block *free, const char *pretty);
static void block_map(const Block *b, const char *pretty);

/* public */

/** Initailises the file system.
 @param fresh
	Boolean value; build up a new file system instead of loading one from disk.
 @return Non-zero on success, if 0, sets sfs_error. */
int mksfs(const int fresh) {
	struct DiskSfs *ds;
	Block bl;
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
	memset(&sfs->free, 0, block_size);
	sfs->dir_size = sfs->dir_memory = 0; sfs->dir_next_memory = 1;
	sfs->dir             = 0;
	sfs->fat_size = sfs->fat_memory = 0; sfs->fat_next_memory = 1;
	sfs->fat             = 0;
	sfs->next_file_index = 0;
	sfs->files_open      = 0;
	sfs->open_buffer = (struct OpenFile *)(sfs + 1);
	for(i = 0; i < max_files_open; i++) {
		struct OpenFile *open = sfs->open[i] = &sfs->open_buffer[i];
		open->id         = i + 1;
		open->inUse      = 0;
	}
	/* fixme: just to test */
	fprintf(stderr, "test sfs->open[1] #%p = %d\n", (void *)sfs->open[1], sfs->open[1]->id);

	if(disk((char *)diskname, block_size, no_blocks) == -1) {                   /* <- calling disk_emu! */
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
	/*((FilePointer *)&bl)[0] = 5;
	((FilePointer *)&bl)[1] = 10; * works! */
	block_map(&bl, "read in block 0");

	/* do the conversion to memory (THIS DEPENDS ON ENDIANESS!)
	 *(((FilePointer *)read)++) "warning: target of assignment not really an
	 lvalue; this will be a hard error in the future," but this is so long;
	 I will make a struct */
	ds = (void *)&bl;
	/*sfs->disk_fat  = *(FilePointer *)read;
	read           = (void *)((FilePointer *)read + 1);
	sfs->disk_free = *(FilePointer *)read;
	read           = (void *)((FilePointer *)read + 1);*/
	/* that's it! seems like a waste of space; maybe have the super-block also
	 contain the free vector; that would be smart . . . but maybe too simpifying */
	/*fprintf(stderr, "  ********* %d, %d\n", sfs->disk_fat, sfs->disk_free);*/
	fprintf(stderr, "  ********* %d, %d\n", ds->dir_size, ds->dir);

	/* check for integrity */
	if(!ds->dir_size != !ds->dir || !ds->fat_size != !ds->fat) {
		if(verbose) fprintf(stderr, "mksfs: ERR_DISK.\n");
		sfs_errno = ERR_DISK;
		rmsfs();
		return 0;
	}

	/* want to deal with free block asap to check for discrepencies;
	 fixme: check for discrepancies */
	if(ds->free != 0) {
		if(read_blocks(ds->free, 1, (void *)bl.data) != 1 || !free_query(&bl, 0)) { /* <- calling disk_emu! */
			if(verbose) fprintf(stderr, "mksfs: free blocks (%d) bad?\n", ds->free);
			sfs_errno = ERR_DISK;
			rmsfs();
			return 0;
		}
		memcpy(sfs->free.data, bl.data, sizeof(Block));
		/*sfs->free_total = ds->free_total;
		 fixme: check sanity of all */
	} else {
		/* new superblock is used already */
		free_set(&sfs->free, 0, -1);
	}

	if(ds->dir != 0) {
		fprintf(stderr, "Not implemented.\n");
		exit(EXIT_FAILURE);
	}

	/* if we have a FAT, load it and check for errors; the simplifying
	 assumption is that we're always going to have enough memory to store all
	 the FAT */
	if(ds->fat != 0) {
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

	/* test grow */
	for(i = 0; i < 10; i++) {
		struct Fat *fat;
		grow(&sfs->fat_memory, &sfs->fat_next_memory);
		if(!(fat = realloc(sfs->fat, sizeof(struct Fat) * sfs->fat_memory))) {
			if(verbose) fprintf(stderr, "grow_fat: ERR_MALLOC.\n");
			sfs_errno = ERR_MALLOC;
			return 0;
		}
		sfs->fat = fat;
		if(verbose) fprintf(stderr, "grow_fat: grew to %d entries.\n", sfs->fat_memory);
	}

	/* test order */
	free_map(&sfs->free, "blocks bv");
#if 0
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
	/* it was working, now it's not :[ */
#endif

	return -1;
}

/** Destructor. */
void rmsfs(void) {
	if(!sfs) return;
	if(verbose) fprintf(stderr, "~Sfs: erase, #%p.\n", (void *)sfs);
	free(sfs);
	sfs = 0;
}

/* private */

/** for bsearch */
static int openfile_cmp(const void *key, const void *elem) {
	return strcmp(((struct OpenFile *)key)->name, ((struct OpenFile *)elem)->name);
}

/** Fibonacci growing thing.
 This is beatiful, actually! I will definitely use this in future projects. */
static void grow(int *a, int *b) {
	int prev = *a;
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
	*b += *a;
	/* make monotonic */
	if(*a == prev) (*a)++;
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

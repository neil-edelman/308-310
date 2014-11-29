/* This is the simple file system used with disk_emu.c, etc; COMP-310
 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free bsearch */
#include <stdio.h>  /* fprintf EOF */
#include "disk_emu.h"
#include "sfs_api.h"

struct OpenFile {
	int desciptor;
	/* ect */
};

struct Sfs {
	int var;
	int lastFileDescriptor;
	struct OpenFile file[2];
};
static const int openfile_size = sizeof((struct Sfs *)0)->file / sizeof(struct OpenFile);

/* for sfs errno */
static const struct PrintError {
	enum SfsError key;
	char *why;
} errors[] = {
	{ ERR_NO,  "No error" },
	{ ERR_WTF, "Error" }
};
enum SfsError sfs_errno = ERR_NO;
struct sfs_dirent {
	char d_name[13];
};
static const int filename_size = sizeof((struct sfs_dirent *)0)->d_name / sizeof(char);

/* private */
static int err_comp(const void *key, const void *elem);

static struct Sfs *sfs = 0;

/* public */



/* these provide functionality equivalent to <stdio.h> */



/** Initailises the file system.
 @param fresh
	Boolean value; build up a new file system instead of loading one from disk.
 @return Non-zero on success, if 0, sets sfs_error. */
int mksfs(const int fresh) {

	/* it's already loaded */
	if(sfs) return -1;

	if(!(sfs = malloc(sizeof(struct Sfs)))) {
		perror("Sfs constructor");
		sfs_errno = ERR_MALLOC;
		rmsfs();
		return 0;
	}
	sfs->var  = 0;
	fprintf(stderr, "Sfs: new, #%p.\n", (void *)sfs);
	if(0) {
		fprintf(stderr, "Sfs: did something with #%p.\n", (void *)sfs);
		sfs_errno = ERR_WTF;
		rmsfs();
		return 0;
	}

	return -1;
}

/** Destructor. */
void rmsfs(void) {

	if(!sfs) return;
	fprintf(stderr, "~Sfs: erase, #%p.\n", (void *)sfs);
	free(sfs);
	sfs = 0;
}

/** Prints a list of the files to stdout. */
void sfs_ls(void) {
	struct sfs_dirent *d;

	if(!sfs) return;
	while((d = readdir())) {
		printf("%s\n", d->d_name);
	}
}

/** Opens a file. "The fopen() function shall open the file whose pathname is
 the string pointed to by filename, and associates a stream with it."
 @param name
	The file name.
 @return "Upon successful completion, fopen() shall return a pointer to the
 object controlling the stream [in this case int]. Otherwise, a null pointer
 shall be returned [int 0], and errno shall be set to indicate the error." */
int sfs_fopen(const char *name) {
	if(!sfs) {
		sfs_errno = ERR_NOT_INIT;
		return 0;
	}
	sfs_errno = ERR_WTF;
	return 0;
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
int sfs_fclose(const int fileID) {
	if(!sfs) return -1;
	sfs_errno = ERR_WTF;
	return EOF;
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
struct sfs_dirent *readdir(void) {
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
	struct PrintError *e = bsearch(&errnum,  /* key */
								   errors,   /* base, num, size */
								   sizeof(errors) / sizeof(struct PrintError),
								   sizeof(struct PrintError),
								   &err_comp /* comparator */);
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



/** Compares the error to the table of errors.
 @peram key
	The error that you want to find out.
 @param elem
	The element in the PrintError errors.
 @return The difference. */
static int err_comp(const void *key, const void *elem) {
	return *(enum SfsError *)key - ((struct PrintError *)elem)->key;
}

/* stdio -Neil */
int mksfs(const int fresh);
void rmsfs(void); /* <-- added this to clean up -Neil */
void sfs_ls(void);
int sfs_fopen(const char *name);
int sfs_fclose(const int fileID);
size_t sfs_fwrite(const int fileID, char *buf, const size_t length);
size_t sfs_fread(const int fileID, char *buf, const size_t length);
int sfs_fseek(const int fileID, const int offset);
int sfs_remove(const char *file);

/* dirent -Neil */
struct sfs_dirent *readdir(void);
void rewinddir(void);

/* errno -Neil */
extern enum SfsError {
	ERR_NO = 0,
	ERR_NOT_INIT,
	ERR_OUT_OF_BOUNDS,
	ERR_MALLOC,
	ERR_DISK,
	ERR_NOFRESH,
	ERR_MAX_FILES,
	ERR_WTF
} sfs_errno;
char *sfs_strerror(const int errnum);
void sfs_perror(const char *s);

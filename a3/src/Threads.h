struct Threads;

struct Threads *Threads(void);
void Threads_(void);
struct Thread *ThreadsCreate(const char *name, void (*func)(int), const int arg, const int stack_size);
int ThreadsRun(void);
void ThreadsExit(struct Thread *exit);
struct Semaphore *ThreadsSemaphore(const int value);
void ThreadsSemaphore_(struct Semaphore **sptr);
void ThreadsSemaphoreDown(struct Semaphore *s);
void ThreadsSemaphoreUp(struct Semaphore *s);
void ThreadsPrintState(FILE *out);
int ThreadsGetVersion(void);
void ThreadsPrintInfo(void);
void ThreadsSetQuantum(const int us);
char *ThreadsDebug(void);
struct Threads;

int Threads(void);
void Threads_(void);
struct Thread *ThreadsCreate(const char *name,
							 void (*func)(const int),
							 const int stack);
void ThreadsExit(const int exit_no/*struct Thread *t*/);
void ThreadsRun(void);
struct Semaphore *ThreadsSemaphore(const int value);
void ThreadsSemaphore_(struct Semaphore **sptr);
void ThreadsSemaphoreDown(struct Semaphore *s);
void ThreadsSemaphoreUp(struct Semaphore *s);
void ThreadsPrintState(FILE *out);
int ThreadsGetVersion(void);
void ThreadsPrintInfo(void);

struct Simple;

struct Simple *Simple(void);
void Simple_(struct Simple **s_ptr);
void SimpleHistory(void);
int SimpleRedo(const char *arg, int *exec_ptr);
void SimpleJobs(void);
int SimpleForground(const char *arg, int *exit_ptr);

struct Job;
struct Client;

struct Job *Job(struct Client *c, int pages);
void Job_(struct Job **);
int JobGetPages(const struct Job *j);
struct Client *JobGetClient(const struct Job *j);
void JobSetBuffer(struct Job *j, const int buffer);
int JobGetBuffer(const struct Job *j);
void JobPrintPages(struct Job *job, const int printed);

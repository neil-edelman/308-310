struct Spool;
struct Job;

struct Spool *Spool(const int jobs_size, const int printers_size, const int clients_size);
void Spool_(struct Spool **);
int SpoolPushJob(/*const */struct Job *job);
struct Job *SpoolPopJob(void);

struct Spool;
struct Job;

struct Spool *Spool(const int jobs_size, const int printers_size, const int clients_size);
void Spool_(struct Spool **);
int SpoolJob(const struct Job *job);

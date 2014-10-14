struct Printer;

struct Printer *Printer(void);
void Printer_(struct Printer **);
int PrinterGetId(const struct Printer *p);
void PrinterPrintJob(const struct Printer *printer, struct Job *job, const int buffer);
int PrinterRun(struct Printer *p);

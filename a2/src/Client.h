struct Client;

struct Client *Client(void);
void Client_(struct Client **);
const char *ClientGetName(const struct Client *c);
int ClientRun(struct Client *c);

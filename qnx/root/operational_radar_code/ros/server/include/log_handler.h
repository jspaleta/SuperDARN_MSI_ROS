#define PRE 0
#define POST 1
#define LOCK 1
#define UNLOCK 0

void logger(char **output_buffer,int post,int lock,char *message,int r,int c,int print);
void *log_handler(void *arg);


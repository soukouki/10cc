
typedef long size_t;
typedef char bool;
extern char true;
extern char false;
extern void* NULL;

void exit();
void* calloc();
void free();
char* strerror();
int isspace();
int isdigit();
int strncmp();
int strlen();
long strtol();
void memcpy();
void strcpy();
void strncpy();

// stdio.h
struct _IO_FILE {};
typedef struct _IO_FILE FILE;
extern int SEEK_SET;
extern int SEEK_CUR;
extern int SEEK_END;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
FILE *fopen();
int fclose();
size_t fread();
int fprintf();
int sprintf();
int printf();

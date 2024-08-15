
// ここから#includeの代わりに展開した部分

typedef long size_t;
extern void* NULL;

void* calloc();
void free();

struct _IO_FILE {};
typedef struct _IO_FILE FILE;
int SEEK_SET = 0;
int SEEK_CUR = 1;
int SEEK_END = 2;

int fseek();
size_t ftell();
size_t fread();
int fclose();
FILE* fopen();

int strncmp();
int strlen();
void strcpy();
void memcpy();
void printf();

void error0(char* file, int line, char* fmt);
void error1(char* file, int line, char* fmt, char* arg1);
void error2(char* file, int line, char* fmt, char* arg1, char* arg2);
void error3(char* file, int line, char* fmt, char* arg1, char* arg2, char* arg3);
void error_at0(char* file, int line, char* loc, char* fmt);
void error_at1(char* file, int line, char* loc, char* fmt, char* arg1);
void error_at2(char* file, int line, char* loc, char* fmt, char* arg1, char* arg2);

// ここまで#includeの代わりに展開した部分

/*
解析できる構文

改行 `#include "` 文字列 `"`
*/

// ファイルを読み込み、バッファに挿入する
void read_file_to_buffer(char* path, char* buf) {
  FILE *fp = fopen(path, "r");
  if (!fp)
    error1(__FILE__, __LINE__, "cannot open %s", path);
  fread(buf, 1, 100000, fp);
  fclose(fp);
}

char* check_preprocess(char* path, char* buf) {
  read_file_to_buffer(path, buf);
  int i = 0;
  while(buf[i]) {
    int include_path_start = 0;
    int include_sharp = i;
    if(strncmp(&buf[i], "#include \"", 10) == 0) {
      i += 10;
      include_path_start = i;
      while(buf[i] != '"') {
        i++;
      }
      int include_path_end = i;
      i++; // ファイル名終わりの"を飛ばす
      char* include_path = calloc(100, 1);
      memcpy(include_path, &buf[include_path_start], include_path_end - include_path_start);
      char* after_buf = calloc(100000, 1);
      strcpy(after_buf, &buf[i]);
      char* content_buf = calloc(100000, 1);
      read_file_to_buffer(include_path, content_buf);
      int content_buf_len = strlen(content_buf);
      strcpy(&buf[include_sharp], content_buf);
      strcpy(&buf[include_sharp + content_buf_len], after_buf);
      i = include_sharp; // iをcontentの先頭にすることで、再帰的に#includeを処理する
    } else {
      i++;
    }
  }
  return buf;
}

char* read_file(char* path) {
  char* buf = calloc(100000, 1); // とりあえず10万文字
  return check_preprocess(path, buf);
}

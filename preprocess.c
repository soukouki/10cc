
#include "10cc.h"

/*
解析できる構文

改行 `#include<空白>"<文字列>"`
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
  int line = 1;
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
      check_preprocess(include_path, content_buf);
      int content_buf_len = strlen(content_buf);
      strcpy(&buf[include_sharp], content_buf);
      strcpy(&buf[include_sharp + content_buf_len], after_buf);
      free(include_path);
      free(after_buf);
      free(content_buf);
      i = include_sharp + content_buf_len;
    } else if (buf[i] == '\n') { // もし改行文字が来たら、`// ファイル名:行数` を挿入する
      char* file_and_line = calloc(100, 1);
      sprintf(file_and_line, " // %s:%d", path, line);
      int file_and_line_len = strlen(file_and_line);
      char* after_buf = calloc(100000, 1);
      strcpy(after_buf, &buf[i]);
      strcpy(&buf[i], file_and_line);
      strcpy(&buf[i + file_and_line_len], after_buf);
      free(file_and_line);
      free(after_buf);
      i += file_and_line_len;
      line++;
      i++;
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

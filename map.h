
typedef struct Map Map;

Map* map_new();
void* map_get (Map* map, const char* key);
void* map_get2(Map* map, const char* key, int key_len);
void map_put (Map* map, const char* key, void* value); // 同じキーがある場合はうまく動作しない
void map_put2(Map* map, const char* key, int key_len, void* value);
void map_delete(Map* map, const char* key);
void map_delete2(Map* map, const char* key, int key_len);


typedef struct Map Map;

Map* map_new();
void* map_get (Map* map, const char* key);
void map_put (Map* map, const char* key, void* value); // 同じキーがある場合はうまく動作しない
void map_delete(Map* map, const char* key);
void** map_values(Map* map);
char** map_keys(Map* map);

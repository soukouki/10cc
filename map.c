
// ここから#includeの代わりに展開した部分

typedef struct Map Map;

Map* map_new();
void* map_get (Map* map, const char* key);
void map_put (Map* map, const char* key, void* value); // 同じキーがある場合はうまく動作しない
void map_delete(Map* map, const char* key);
void** map_values(Map* map);
char** map_keys(Map* map);

void* NULL = 0;

void* malloc();
void free();
int strlen();
void* memcmp();
void memcpy();

// ここまで#includeの代わりに展開した部分

typedef struct MapEntry MapEntry;

struct MapEntry {
  char* key;
  int key_len;
  void* data;
  MapEntry* next;
};

struct Map {
  MapEntry* first;
  int size;
};

static void* map_get2(Map* map, const char* key, int key_len);
static void map_put2(Map* map, const char* key, int key_len, void* data);
static void map_delete2(Map* map, const char* key, int key_len);

Map* map_new() {
  Map* map = malloc(sizeof(Map));
  map->first = NULL;
  map->size = 0;
  return map;
}

void* map_get(Map* map, const char* key) {
  return map_get2(map, key, strlen(key));
}

static void* map_get2(Map* map, const char* key, int key_len) {
  MapEntry* entry = map->first;
  while (entry != NULL) {
    if (entry->key_len == key_len && memcmp(entry->key, key, key_len) == 0) {
      return entry->data;
    }
    entry = entry->next;
  }
  return NULL;
}

void map_put(Map* map, const char* key, void* data) {
  map_put2(map, key, strlen(key), data);
}

static void map_put2(Map* map, const char* key, int key_len, void* data) {
  if(map_get2(map, key, key_len) != NULL) {
    map_delete2(map, key, key_len);
  }
  MapEntry* entry = malloc(sizeof(MapEntry));
  entry->key = malloc(key_len);
  memcpy(entry->key, key, key_len);
  entry->key_len = key_len;
  entry->data = data;
  entry->next = map->first;
  map->first = entry;
  map->size++;
}

void map_delete(Map* map, const char* key) {
  map_delete2(map, key, strlen(key));
}

static void map_delete2(Map* map, const char* key, int key_len) {
  MapEntry* entry = map->first;
  MapEntry* prev = NULL;
  while (entry != NULL) {
    if (entry->key_len == key_len && memcmp(entry->key, key, key_len) == 0) {
      if (prev == NULL) {
        map->first = entry->next;
      } else {
        prev->next = entry->next;
      }
      free(entry->key);
      free(entry);
      map->size--;
      return;
    }
    prev = entry;
    entry = entry->next;
  }
}

void** map_values(Map* map) {
  void** all = malloc(sizeof(void*) * (map->size + 1));
  MapEntry* entry = map->first;
  for (int i = 0; i < map->size; i++) {
    all[i] = entry->data;
    entry = entry->next;
  }
  all[map->size] = NULL;
  return all;
}

char** map_keys(Map* map) {
  char** all = malloc(sizeof(char*) * (map->size + 1));
  MapEntry* entry = map->first;
  for (int i = 0; i < map->size; i++) {
    all[i] = entry->key;
    entry = entry->next;
  }
  all[map->size] = NULL;
  return all;
}

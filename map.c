
#include <stdlib.h>
#include <string.h>

#include "map.h"

typedef struct MapEntry MapEntry;

struct MapEntry {
  char* key;
  int key_len;
  void* data;
  MapEntry* next;
};

struct Map {
  MapEntry* first;
};

Map* map_new() {
  Map* map = malloc(sizeof(Map));
  map->first = NULL;
  return map;
}

void* map_get(Map* map, const char* key) {
  return map_get2(map, key, strlen(key));
}

void* map_get2(Map* map, const char* key, int key_len) {
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

void map_put2(Map* map, const char* key, int key_len, void* data) {
  MapEntry* entry = malloc(sizeof(MapEntry));
  entry->key = malloc(key_len);
  memcpy(entry->key, key, key_len);
  entry->key_len = key_len;
  entry->data = data;
  entry->next = map->first;
  map->first = entry;
}

void map_delete(Map* map, const char* key) {
  map_delete2(map, key, strlen(key));
}

void map_delete2(Map* map, const char* key, int key_len) {
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
      return;
    }
    prev = entry;
    entry = entry->next;
  }
}

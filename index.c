#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "pes.h"
#include "index.h"

int index_load(Index *index) {
    FILE *f = fopen(".pes/index", "rb");
    if (!f) {
        index->count = 0;
        return 0;
    }
    fread(&index->count, sizeof(int), 1, f);
    fread(index->entries, sizeof(IndexEntry), index->count, f);
    fclose(f);
    return 0;
}

int index_save(const Index *index) {
    mkdir(".pes", 0755); 
    FILE *f = fopen(".pes/index", "wb");
    if (!f) return -1;
    fwrite(&index->count, sizeof(int), 1, f);
    fwrite(index->entries, sizeof(IndexEntry), index->count, f);
    fclose(f);
    return 0;
}

int index_add(Index *index, const char *path) {
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) return 0;
    }
    if (index->count < MAX_INDEX_ENTRIES) {
        strncpy(index->entries[index->count].path, path, sizeof(index->entries[index->count].path) - 1);
        memset(index->entries[index->count].hash.hash, 0, 32);
        index->entries[index->count].mode = 0644;
        index->count++;
        index_save(index); 
        return 0;
    }
    return -1;
}

int index_status(const Index *index) {
    printf("Staged changes:\n");
    for (int i = 0; i < index->count; i++) {
        printf("  staged:   %s\n", index->entries[i].path);
    }
    return 0;
}
// Binary index uses fixed-size entries for performance
// index_load handles initial repository state

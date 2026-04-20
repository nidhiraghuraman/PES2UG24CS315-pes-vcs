#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "index.h"
#include "object.h"

#define INDEX_FILE ".pes/index"

/* -------------------- LOAD INDEX -------------------- */
int index_load(Index *index) {
    FILE *fp = fopen(INDEX_FILE, "r");

    index->count = 0;

    // If index file does not exist → empty index
    if (!fp) {
        return 0;
    }

    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        IndexEntry entry;

        sscanf(line, "%o %s %ld %ld %s",
               &entry.mode,
               entry.hash,
               &entry.mtime,
               &entry.size,
               entry.path);

        index->entries[index->count++] = entry;
    }

    fclose(fp);
    return 0;
}

/* -------------------- SAVE INDEX -------------------- */
int index_save(const Index *index) {
    FILE *fp = fopen(INDEX_FILE, "w");
    if (!fp) {
        perror("index_save");
        return -1;
    }

    for (int i = 0; i < index->count; i++) {
        fprintf(fp, "%o %s %ld %ld %s\n",
                index->entries[i].mode,
                index->entries[i].hash,
                index->entries[i].mtime,
                index->entries[i].size,
                index->entries[i].path);
    }

    fclose(fp);
    return 0;
}

/* -------------------- ADD FILE -------------------- */
int index_add(Index *index, const char *path) {

    // Check if file already exists → update
    for (int i = 0; i < index->count; i++) {
        if (strcmp(index->entries[i].path, path) == 0) {
            // Remove old entry by shifting
            for (int j = i; j < index->count - 1; j++) {
                index->entries[j] = index->entries[j + 1];
            }
            index->count--;
            break;
        }
    }

    // Open file
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        perror("index_add fopen");
        return -1;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    // Read file data
    char *data = malloc(size);
    fread(data, 1, size, fp);
    fclose(fp);

    // Compute hash + store object
    char hash[65];   // SHA-256 hex string
    object_write("blob", data, size, hash);

    free(data);

    // Get file metadata
    struct stat st;
    stat(path, &st);

    // Create entry
    IndexEntry entry;
    entry.mode = 0644;
    strcpy(entry.hash, hash);
    entry.mtime = st.st_mtime;
    entry.size = size;
    strcpy(entry.path, path);

    // Add to index
    index->entries[index->count++] = entry;

    // Save index
    return index_save(index);
}

/* -------------------- STATUS -------------------- */
int index_status(const Index *index) {
    printf("Staged changes:\n");

    if (index->count == 0) {
        printf("  (nothing to show)\n");
        return 0;
    }

    for (int i = 0; i < index->count; i++) {
        printf("  staged:   %s\n", index->entries[i].path);
    }

    return 0;
}

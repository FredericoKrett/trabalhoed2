#include "hash.h"
#include <assert.h>

#define BUCKET_SIZE 4 // numero de registros por bucket pagina

typedef struct {
    int local_depth;
    int record_count;
} HashBucketHeader;

struct HashFile {
    char dir_filename[256];
    char data_filename[256];
    FILE* dir_file;
    FILE* data_file;
    int global_depth;
    int dir_size;
    long* directory; 
    int record_size;
    int key_offset;
};

static inline size_t get_bucket_disk_size(HashFile* hf) {
    return sizeof(HashBucketHeader) + (BUCKET_SIZE * hf->record_size);
}

static inline void* get_record_ptr(void* bucket_buf, int index, HashFile* hf) {
    char* records_area = (char*)bucket_buf + sizeof(HashBucketHeader);
    return records_area + (index * hf->record_size);
}

static inline int get_key(void* record_ptr, HashFile* hf) {
    return *(int*)((char*)record_ptr + hf->key_offset);
}

bool hash_create(const char* filename_prefix, int record_size, int key_offset) {
    if (!filename_prefix) return false;
    char dir_filename[256];
    char data_filename[256];
    snprintf(dir_filename, sizeof(dir_filename), "%s.dir", filename_prefix);
    snprintf(data_filename, sizeof(data_filename), "%s.dat", filename_prefix);

    FILE* d_file = fopen(dir_filename, "wb");
    FILE* dat_file = fopen(data_filename, "wb");
    if (!d_file || !dat_file) {
        if (d_file) fclose(d_file);
        if (dat_file) fclose(dat_file);
        return false;
    }

    int global_depth = 0;
    fwrite(&global_depth, sizeof(int), 1, d_file);
    fwrite(&record_size, sizeof(int), 1, d_file);
    fwrite(&key_offset, sizeof(int), 1, d_file);
    
    long offset = 0;
    fwrite(&offset, sizeof(long), 1, d_file); // dir_size = 1 for global_depth = 0
    
    HashBucketHeader b_header;
    b_header.local_depth = 0;
    b_header.record_count = 0;
    
    size_t bucket_full_size = sizeof(HashBucketHeader) + (BUCKET_SIZE * record_size);
    void* zero_bucket = calloc(1, bucket_full_size);
    if (!zero_bucket) {
        fclose(d_file);
        fclose(dat_file);
        return false;
    }
    memcpy(zero_bucket, &b_header, sizeof(HashBucketHeader));
    
    fwrite(zero_bucket, bucket_full_size, 1, dat_file);
    
    free(zero_bucket);
    fclose(d_file);
    fclose(dat_file);
    return true;
}

HashFile* hash_open(const char* filename_prefix) {
    if (!filename_prefix) return NULL;
    HashFile* hf = malloc(sizeof(HashFile));
    if (!hf) return NULL;
    
    snprintf(hf->dir_filename, sizeof(hf->dir_filename), "%s.dir", filename_prefix);
    snprintf(hf->data_filename, sizeof(hf->data_filename), "%s.dat", filename_prefix);

    hf->dir_file = fopen(hf->dir_filename, "r+b");
    hf->data_file = fopen(hf->data_filename, "r+b");

    if (!hf->dir_file || !hf->data_file) {
        if (hf->dir_file) fclose(hf->dir_file);
        if (hf->data_file) fclose(hf->data_file);
        free(hf);
        return NULL;
    }

    fread(&hf->global_depth, sizeof(int), 1, hf->dir_file);
    fread(&hf->record_size, sizeof(int), 1, hf->dir_file);
    fread(&hf->key_offset, sizeof(int), 1, hf->dir_file);
    
    hf->dir_size = 1 << hf->global_depth;
    hf->directory = malloc(hf->dir_size * sizeof(long));
    if (!hf->directory) {
        fclose(hf->dir_file);
        fclose(hf->data_file);
        free(hf);
        return NULL;
    }
    fread(hf->directory, sizeof(long), hf->dir_size, hf->dir_file);

    return hf;
}

void hash_close(HashFile* hf) {
    if (!hf) return;
    fseek(hf->dir_file, 0, SEEK_SET);
    fwrite(&hf->global_depth, sizeof(int), 1, hf->dir_file);
    fwrite(&hf->record_size, sizeof(int), 1, hf->dir_file);
    fwrite(&hf->key_offset, sizeof(int), 1, hf->dir_file);
    fwrite(hf->directory, sizeof(long), hf->dir_size, hf->dir_file);
    fclose(hf->dir_file);
    fclose(hf->data_file);
    free(hf->directory);
    free(hf);
}

// insercao recursiva interna
static bool hash_insert_internal(HashFile* hf, void* reg) {
    int key = get_key(reg, hf);
    int h = key & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* bucket_buf = malloc(full_bucket_size);
    if (!bucket_buf) return false;
    
    fseek(hf->data_file, offset, SEEK_SET);
    fread(bucket_buf, full_bucket_size, 1, hf->data_file);
    
    HashBucketHeader* header = (HashBucketHeader*)bucket_buf;

    for (int i = 0; i < header->record_count; i++) {
        int existing_key = get_key(get_record_ptr(bucket_buf, i, hf), hf);
        if (existing_key == key) {
            free(bucket_buf);
            return false; // chave ja existe
        }
    }

    if (header->record_count < BUCKET_SIZE) {
        void* dest = get_record_ptr(bucket_buf, header->record_count, hf);
        memcpy(dest, reg, hf->record_size);
        header->record_count++;
        
        fseek(hf->data_file, offset, SEEK_SET);
        fwrite(bucket_buf, full_bucket_size, 1, hf->data_file);
        free(bucket_buf);
        return true;
    }

    if (header->local_depth == hf->global_depth) {
        int old_size = hf->dir_size;
        hf->global_depth++;
        hf->dir_size = 1 << hf->global_depth;
        hf->directory = realloc(hf->directory, hf->dir_size * sizeof(long));
        if (!hf->directory) {
            free(bucket_buf);
            return false; 
        }
        
        for (int i = 0; i < old_size; i++) {
            hf->directory[i + old_size] = hf->directory[i];
        }
    }

    header->local_depth++;
    
    void* new_bucket_buf = malloc(full_bucket_size);
    if (!new_bucket_buf) {
        free(bucket_buf);
        return false;
    }
    
    HashBucketHeader* new_header = (HashBucketHeader*)new_bucket_buf;
    new_header->local_depth = header->local_depth;
    new_header->record_count = 0;

    fseek(hf->data_file, 0, SEEK_END);
    long new_b_offset = ftell(hf->data_file);

    // copy old records to temp
    void* temp_records = malloc(BUCKET_SIZE * hf->record_size);
    if (!temp_records) {
        free(bucket_buf);
        free(new_bucket_buf);
        return false;
    }
    memcpy(temp_records, (char*)bucket_buf + sizeof(HashBucketHeader), BUCKET_SIZE * hf->record_size);
    
    header->record_count = 0;

    int bit = 1 << (header->local_depth - 1);

    for (int i = 0; i < hf->dir_size; i++) {
        if (hf->directory[i] == offset) {
            if ((i & bit) != 0) {
                hf->directory[i] = new_b_offset;
            }
        }
    }

    for (int i = 0; i < BUCKET_SIZE; i++) {
        void* rec_ptr = (char*)temp_records + (i * hf->record_size);
        int cur_hash = get_key(rec_ptr, hf);
        if ((cur_hash & bit) != 0) {
            void* dest = get_record_ptr(new_bucket_buf, new_header->record_count, hf);
            memcpy(dest, rec_ptr, hf->record_size);
            new_header->record_count++;
        } else {
            void* dest = get_record_ptr(bucket_buf, header->record_count, hf);
            memcpy(dest, rec_ptr, hf->record_size);
            header->record_count++;
        }
    }

    fseek(hf->data_file, offset, SEEK_SET);
    fwrite(bucket_buf, full_bucket_size, 1, hf->data_file);
    fseek(hf->data_file, new_b_offset, SEEK_SET);
    fwrite(new_bucket_buf, full_bucket_size, 1, hf->data_file);

    free(temp_records);
    free(bucket_buf);
    free(new_bucket_buf);

    return hash_insert_internal(hf, reg); 
}

bool hash_insert(HashFile* hf, void* reg) {
    if (!hf || !reg) return false;
    return hash_insert_internal(hf, reg);
}

bool hash_search(HashFile* hf, int key, void* out_reg) {
    if (!hf || !out_reg) return false;
    int h = key & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* bucket_buf = malloc(full_bucket_size);
    if(!bucket_buf) return false;
    
    fseek(hf->data_file, offset, SEEK_SET);
    fread(bucket_buf, full_bucket_size, 1, hf->data_file);

    HashBucketHeader* header = (HashBucketHeader*)bucket_buf;

    for (int i = 0; i < header->record_count; i++) {
        void* rec_ptr = get_record_ptr(bucket_buf, i, hf);
        if (get_key(rec_ptr, hf) == key) {
            memcpy(out_reg, rec_ptr, hf->record_size);
            free(bucket_buf);
            return true;
        }
    }
    free(bucket_buf);
    return false;
}

bool hash_delete(HashFile* hf, int key) {
    if (!hf) return false;
    int h = key & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* bucket_buf = malloc(full_bucket_size);
    if (!bucket_buf) return false;
    
    fseek(hf->data_file, offset, SEEK_SET);
    fread(bucket_buf, full_bucket_size, 1, hf->data_file);
    
    HashBucketHeader* header = (HashBucketHeader*)bucket_buf;

    for (int i = 0; i < header->record_count; i++) {
        void* rec_ptr = get_record_ptr(bucket_buf, i, hf);
        if (get_key(rec_ptr, hf) == key) {
            if (i < header->record_count - 1) {
                void* last_rec = get_record_ptr(bucket_buf, header->record_count - 1, hf);
                memcpy(rec_ptr, last_rec, hf->record_size);
            }
            header->record_count--;
            fseek(hf->data_file, offset, SEEK_SET);
            fwrite(bucket_buf, full_bucket_size, 1, hf->data_file);
            free(bucket_buf);
            return true;
        }
    }
    free(bucket_buf);
    return false;
}

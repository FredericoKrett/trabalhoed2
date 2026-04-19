#include "hashfile.h"
#include <assert.h>

#define BUCKET_CAPACITY 4 // numero de registros por bucket/pagina

typedef struct {
    int local_depth;
    int record_count;
    bool is_active[BUCKET_CAPACITY];
} HashBucketHeader;

struct hashfile {
    char dir_filename[512];
    char data_filename[512];
    FILE* dir_file;
    FILE* data_file;
    int global_depth;
    int dir_size;
    long* directory; 
    int record_size;
    int key_offset;
    int key_size;
};

// Utils: Calcular tamanhos e buscar ponteiros
static inline size_t get_bucket_disk_size(struct hashfile* hf) {
    return sizeof(HashBucketHeader) + (BUCKET_CAPACITY * hf->record_size);
}

static inline void* get_record_ptr(void* bucket_buf, int index, struct hashfile* hf) {
    char* records_area = (char*)bucket_buf + sizeof(HashBucketHeader);
    return records_area + (index * hf->record_size);
}

static inline void get_key_str(void* record_ptr, struct hashfile* hf, char* out_key) {
    strncpy(out_key, (char*)record_ptr + hf->key_offset, hf->key_size);
    out_key[hf->key_size] = '\0';
}

static int hash_djb2(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return (int)(hash & 0x7FFFFFFF);
}

// Utils: Resolucoes de caminhos
static void build_filepath(char* buffer, size_t max_len, const char* dir, const char* prefix, const char* ext) {
    if (dir && strlen(dir) > 0) {
        snprintf(buffer, max_len, "%s/%s%s", dir, prefix, ext);
    } else {
        snprintf(buffer, max_len, "%s%s", prefix, ext);
    }
}

// ----------------------------------------------------
// Core Functions
// ----------------------------------------------------

HashFile hash_create(const char* out_dir, const char* filename_prefix, int record_size, int key_offset, int key_size) {
    if (!filename_prefix) return NULL;
    
    struct hashfile* hf = malloc(sizeof(struct hashfile));
    if (!hf) return NULL;

    build_filepath(hf->dir_filename, sizeof(hf->dir_filename), out_dir, filename_prefix, ".dir");
    build_filepath(hf->data_filename, sizeof(hf->data_filename), out_dir, filename_prefix, ".dat");

    hf->dir_file = fopen(hf->dir_filename, "w+b");
    hf->data_file = fopen(hf->data_filename, "w+b");

    if (!hf->dir_file || !hf->data_file) {
        if (hf->dir_file) fclose(hf->dir_file);
        if (hf->data_file) fclose(hf->data_file);
        free(hf);
        return NULL;
    }

    hf->global_depth = 0;
    hf->record_size = record_size;
    hf->key_offset = key_offset;
    hf->key_size = key_size;
    hf->dir_size = 1;
    hf->directory = malloc(sizeof(long));
    hf->directory[0] = 0; // offset do primeiro bucket

    fwrite(&hf->global_depth, sizeof(int), 1, hf->dir_file);
    fwrite(&hf->record_size, sizeof(int), 1, hf->dir_file);
    fwrite(&hf->key_offset, sizeof(int), 1, hf->dir_file);
    fwrite(&hf->key_size, sizeof(int), 1, hf->dir_file);
    fwrite(hf->directory, sizeof(long), hf->dir_size, hf->dir_file);
    
    HashBucketHeader b_header;
    b_header.local_depth = 0;
    b_header.record_count = 0;
    for(int i = 0; i < BUCKET_CAPACITY; i++) b_header.is_active[i] = false;
    
    size_t full_size = get_bucket_disk_size(hf);
    void* zero_bucket = calloc(1, full_size);
    memcpy(zero_bucket, &b_header, sizeof(HashBucketHeader));
    fwrite(zero_bucket, full_size, 1, hf->data_file);
    free(zero_bucket);

    return hf;
}

HashFile hash_open(const char* in_dir, const char* filename_prefix) {
    if (!filename_prefix) return NULL;
    
    struct hashfile* hf = malloc(sizeof(struct hashfile));
    if (!hf) return NULL;

    build_filepath(hf->dir_filename, sizeof(hf->dir_filename), in_dir, filename_prefix, ".dir");
    build_filepath(hf->data_filename, sizeof(hf->data_filename), in_dir, filename_prefix, ".dat");

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
    fread(&hf->key_size, sizeof(int), 1, hf->dir_file);
    
    hf->dir_size = 1 << hf->global_depth;
    hf->directory = malloc(hf->dir_size * sizeof(long));
    fread(hf->directory, sizeof(long), hf->dir_size, hf->dir_file);

    return hf;
}

void hash_close(HashFile hf_gen) {
    struct hashfile* hf = (struct hashfile*) hf_gen;
    if (!hf) return;
    fseek(hf->dir_file, 0, SEEK_SET);
    fwrite(&hf->global_depth, sizeof(int), 1, hf->dir_file);
    fwrite(&hf->record_size, sizeof(int), 1, hf->dir_file);
    fwrite(&hf->key_offset, sizeof(int), 1, hf->dir_file);
    fwrite(&hf->key_size, sizeof(int), 1, hf->dir_file);
    fwrite(hf->directory, sizeof(long), hf->dir_size, hf->dir_file);
    
    fclose(hf->dir_file);
    fclose(hf->data_file);
    free(hf->directory);
    free(hf);
}

// ----------------------------------------------------
// Auxiliary Insert Logic (SRP)
// ----------------------------------------------------

static bool directory_doubling(struct hashfile* hf) {
    int old_size = hf->dir_size;
    hf->global_depth++;
    hf->dir_size = 1 << hf->global_depth;
    long* new_dir = realloc(hf->directory, hf->dir_size * sizeof(long));
    if (!new_dir) return false;
    hf->directory = new_dir;
    
    // Duplicar ponteiros
    for (int i = 0; i < old_size; i++) {
        hf->directory[i + old_size] = hf->directory[i];
    }
    return true;
}

static bool bucket_split(struct hashfile* hf, long old_bucket_offset, void* old_bucket_buf) {
    HashBucketHeader* old_header = (HashBucketHeader*)old_bucket_buf;
    if (old_header->local_depth == hf->global_depth) {
        if (!directory_doubling(hf)) return false;
    }
    
    old_header->local_depth++;
    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* new_bucket_buf = calloc(1, full_bucket_size);
    if (!new_bucket_buf) return false;

    HashBucketHeader* new_header = (HashBucketHeader*)new_bucket_buf;
    new_header->local_depth = old_header->local_depth;
    new_header->record_count = 0;
    
    fseek(hf->data_file, 0, SEEK_END);
    long new_bucket_offset = ftell(hf->data_file);

    // Ajustar diretorio
    int bit_to_check = 1 << (old_header->local_depth - 1);
    for (int i = 0; i < hf->dir_size; i++) {
        if (hf->directory[i] == old_bucket_offset) {
            if ((i & bit_to_check) != 0) {
                hf->directory[i] = new_bucket_offset;
            }
        }
    }

    // Redistribuir registros 
    void* temp_records = malloc(BUCKET_CAPACITY * hf->record_size);
    bool temp_active[BUCKET_CAPACITY];
    memcpy(temp_records, (char*)old_bucket_buf + sizeof(HashBucketHeader), BUCKET_CAPACITY * hf->record_size);
    memcpy(temp_active, old_header->is_active, BUCKET_CAPACITY * sizeof(bool));

    old_header->record_count = 0;
    for(int i = 0; i < BUCKET_CAPACITY; i++) old_header->is_active[i] = false;

    for (int i = 0; i < BUCKET_CAPACITY; i++) {
        if (!temp_active[i]) continue;
        void* rec_ptr = (char*)temp_records + (i * hf->record_size);
        char temp_key[256];
        get_key_str(rec_ptr, hf, temp_key);
        int cur_hash = hash_djb2(temp_key);
        
        if ((cur_hash & bit_to_check) != 0) {
            // Vai para o novo bucket
            void* dest = get_record_ptr(new_bucket_buf, new_header->record_count, hf);
            memcpy(dest, rec_ptr, hf->record_size);
            new_header->is_active[new_header->record_count] = true;
            new_header->record_count++;
        } else {
            // Fica no bucket antigo
            void* dest = get_record_ptr(old_bucket_buf, old_header->record_count, hf);
            memcpy(dest, rec_ptr, hf->record_size);
            old_header->is_active[old_header->record_count] = true;
            old_header->record_count++;
        }
    }

    fseek(hf->data_file, old_bucket_offset, SEEK_SET);
    fwrite(old_bucket_buf, full_bucket_size, 1, hf->data_file);
    fseek(hf->data_file, new_bucket_offset, SEEK_SET);
    fwrite(new_bucket_buf, full_bucket_size, 1, hf->data_file);

    free(temp_records);
    free(new_bucket_buf);
    return true;
}

bool hash_insert(HashFile hf_gen, void* reg) {
    struct hashfile* hf = (struct hashfile*) hf_gen;
    if (!hf || !reg) return false;
    char key_str[256];
    get_key_str(reg, hf, key_str);
    int h = hash_djb2(key_str) & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* bucket_buf = malloc(full_bucket_size);
    if (!bucket_buf) return false;
    
    fseek(hf->data_file, offset, SEEK_SET);
    fread(bucket_buf, full_bucket_size, 1, hf->data_file);
    HashBucketHeader* header = (HashBucketHeader*)bucket_buf;

    // Reject se existe
    for (int i = 0; i < BUCKET_CAPACITY; i++) {
        if (header->is_active[i]) {
            char existing_key[256];
            get_key_str(get_record_ptr(bucket_buf, i, hf), hf, existing_key);
            if (strncmp(existing_key, key_str, hf->key_size) == 0) {
                free(bucket_buf);
                return false;
            }
        }
    }

    if (header->record_count < BUCKET_CAPACITY) {
        // Inserir no primeiro espaco livre
        for (int i = 0; i < BUCKET_CAPACITY; i++) {
            if (!header->is_active[i]) {
                void* dest = get_record_ptr(bucket_buf, i, hf);
                memcpy(dest, reg, hf->record_size);
                header->is_active[i] = true;
                header->record_count++;
                fseek(hf->data_file, offset, SEEK_SET);
                fwrite(bucket_buf, full_bucket_size, 1, hf->data_file);
                free(bucket_buf);
                return true;
            }
        }
    }

    // Split e tentar de novo recursivamente
    bool split_ok = bucket_split(hf, offset, bucket_buf);
    free(bucket_buf);
    
    if (!split_ok) return false;
    return hash_insert(hf, reg);
}

// ----------------------------------------------------
// Search & Delete
// ----------------------------------------------------

bool hash_search(HashFile hf_gen, const char* key, void* out_reg) {
    struct hashfile* hf = (struct hashfile*) hf_gen;
    if (!hf || !out_reg || !key) return false;
    int h = hash_djb2(key) & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* bucket_buf = malloc(full_bucket_size);
    if(!bucket_buf) return false;
    
    fseek(hf->data_file, offset, SEEK_SET);
    fread(bucket_buf, full_bucket_size, 1, hf->data_file);
    HashBucketHeader* header = (HashBucketHeader*)bucket_buf;

    for (int i = 0; i < BUCKET_CAPACITY; i++) {
        if (header->is_active[i]) {
            void* rec_ptr = get_record_ptr(bucket_buf, i, hf);
            char existing_key[256];
            get_key_str(rec_ptr, hf, existing_key);
            if (strncmp(existing_key, key, hf->key_size) == 0) {
                memcpy(out_reg, rec_ptr, hf->record_size);
                free(bucket_buf);
                return true;
            }
        }
    }
    
    free(bucket_buf);
    return false;
}

bool hash_delete(HashFile hf_gen, const char* key) {
    struct hashfile* hf = (struct hashfile*) hf_gen;
    if (!hf || !key) return false;
    int h = hash_djb2(key) & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* bucket_buf = malloc(full_bucket_size);
    if (!bucket_buf) return false;
    fseek(hf->data_file, offset, SEEK_SET);
    fread(bucket_buf, full_bucket_size, 1, hf->data_file);
    HashBucketHeader* header = (HashBucketHeader*)bucket_buf;

    for (int i = 0; i < BUCKET_CAPACITY; i++) {
        if (header->is_active[i]) {
            void* rec_ptr = get_record_ptr(bucket_buf, i, hf);
            char existing_key[256];
            get_key_str(rec_ptr, hf, existing_key);
            if (strncmp(existing_key, key, hf->key_size) == 0) {
                // Efetua o "Tombstone" - marca como inativo apenas
                header->is_active[i] = false;
                header->record_count--;
                fseek(hf->data_file, offset, SEEK_SET);
                fwrite(bucket_buf, full_bucket_size, 1, hf->data_file);
                free(bucket_buf);
                return true;
            }
        }
    }
    free(bucket_buf);
    return false;
}

// ----------------------------------------------------
// Directory Output
// ----------------------------------------------------

void hash_print_directory(HashFile hf_gen, const char* out_dir, const char* filename_txt) {
    struct hashfile* hf = (struct hashfile*) hf_gen;
    if (!hf || !filename_txt) return;
    
    char hfd_path[512];
    build_filepath(hfd_path, sizeof(hfd_path), out_dir, filename_txt, ".hfd");
    
    FILE* txt = fopen(hfd_path, "w");
    if (!txt) return;

    fprintf(txt, "Global Depth: %d\n", hf->global_depth);
    fprintf(txt, "Directory Size: %d\n", hf->dir_size);
    fprintf(txt, "Bucket Capacity: %d\n", BUCKET_CAPACITY);
    
    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* bucket_buf = malloc(full_bucket_size);

    for (int i = 0; i < hf->dir_size; i++) {
        long offset = hf->directory[i];
        fseek(hf->data_file, offset, SEEK_SET);
        fread(bucket_buf, full_bucket_size, 1, hf->data_file);
        
        HashBucketHeader* header = (HashBucketHeader*)bucket_buf;
        fprintf(txt, "Dir[%d] -> Offset: %ld | Local Depth: %d | Records: %d/%d\n", 
                i, offset, header->local_depth, header->record_count, BUCKET_CAPACITY);
    }
    
    free(bucket_buf);
    fclose(txt);
}

void hash_for_each(HashFile hf_gen, HashIteratorFunc callback, void* context) {
    struct hashfile* hf = (struct hashfile*) hf_gen;
    if (!hf || !callback) return;

    size_t full_bucket_size = get_bucket_disk_size(hf);
    void* bucket_buf = malloc(full_bucket_size);
    if (!bucket_buf) return;

    // Vai pro inicio do .dat
    fseek(hf->data_file, 0, SEEK_SET);

    while (fread(bucket_buf, full_bucket_size, 1, hf->data_file) == 1) {
        HashBucketHeader* header = (HashBucketHeader*)bucket_buf;
        
        // Ignora buckets vazios pra acelerar
        if (header->record_count == 0) continue;

        for (int i = 0; i < BUCKET_CAPACITY; i++) {
            if (header->is_active[i]) {
                void* rec_ptr = get_record_ptr(bucket_buf, i, hf);
                // Call back repassando um ponteiro para os dados
                callback(rec_ptr, context);
            }
        }
    }

    free(bucket_buf);
}

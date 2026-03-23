#include "hash.h"
#include <assert.h>

#define BUCKET_SIZE 4 // numero de registros por bucket pagina
typedef struct {
    int local_depth;
    int record_count;
    Record records[BUCKET_SIZE];
} HashBucket;

// contexto do hash file
struct HashFile {
    char dir_filename[256];
    char data_filename[256];
    FILE* dir_file;
    FILE* data_file;
    int global_depth;
    int dir_size;
    long* directory; // diretorio em memoria
};

bool hash_create(const char* filename_prefix) {
    assert(filename_prefix != NULL && "filename_prefix is NULL");
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
    
    long offset = 0;
    fwrite(&offset, sizeof(long), 1, d_file); // dir_size = 1 for global_depth = 0
    
    HashBucket b;
    b.local_depth = 0;
    b.record_count = 0;
    fwrite(&b, sizeof(HashBucket), 1, dat_file);
    
    fclose(d_file);
    fclose(dat_file);
    return true;
}

HashFile* hash_open(const char* filename_prefix) {
    assert(filename_prefix != NULL && "filename_prefix is NULL");
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
    assert(hf != NULL && "hf is NULL");
    if (!hf) return;
    fseek(hf->dir_file, 0, SEEK_SET);
    fwrite(&hf->global_depth, sizeof(int), 1, hf->dir_file);
    fwrite(hf->directory, sizeof(long), hf->dir_size, hf->dir_file);
    fclose(hf->dir_file);
    fclose(hf->data_file);
    free(hf->directory);
    free(hf);
}

// insercao recursiva interna
static bool hash_insert_internal(HashFile* hf, Record rec) {
    assert(hf != NULL && "hf is NULL");
    int h = rec.id & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    HashBucket b;
    fseek(hf->data_file, offset, SEEK_SET);
    fread(&b, sizeof(HashBucket), 1, hf->data_file);

    for (int i = 0; i < b.record_count; i++) {
        if (b.records[i].id == rec.id) return false; // chave ja existe
    }

    if (b.record_count < BUCKET_SIZE) {
        b.records[b.record_count++] = rec;
        fseek(hf->data_file, offset, SEEK_SET);
        fwrite(&b, sizeof(HashBucket), 1, hf->data_file);
        return true;
    }

    if (b.local_depth == hf->global_depth) {
        int old_size = hf->dir_size;
        hf->global_depth++;
        hf->dir_size = 1 << hf->global_depth;
        hf->directory = realloc(hf->directory, hf->dir_size * sizeof(long));
        if (!hf->directory) return false; // erro de memoria
        
        for (int i = 0; i < old_size; i++) {
            hf->directory[i + old_size] = hf->directory[i];
        }
    }

    b.local_depth++;
    HashBucket new_b;
    new_b.local_depth = b.local_depth;
    new_b.record_count = 0;

    fseek(hf->data_file, 0, SEEK_END);
    long new_b_offset = ftell(hf->data_file);

    Record temp[BUCKET_SIZE];
    memcpy(temp, b.records, sizeof(Record) * BUCKET_SIZE);
    b.record_count = 0;

    int bit = 1 << (b.local_depth - 1);

    for (int i = 0; i < hf->dir_size; i++) {
        if (hf->directory[i] == offset) {
            if ((i & bit) != 0) {
                hf->directory[i] = new_b_offset;
            }
        }
    }

    for (int i = 0; i < BUCKET_SIZE; i++) {
        int hash_val = temp[i].id;
        if ((hash_val & bit) != 0) {
            new_b.records[new_b.record_count++] = temp[i];
        } else {
            b.records[b.record_count++] = temp[i];
        }
    }

    fseek(hf->data_file, offset, SEEK_SET);
    fwrite(&b, sizeof(HashBucket), 1, hf->data_file);
    fseek(hf->data_file, new_b_offset, SEEK_SET);
    fwrite(&new_b, sizeof(HashBucket), 1, hf->data_file);

    return hash_insert_internal(hf, rec); // tenta inserir novamente
}

bool hash_insert(HashFile* hf, Record rec) {
    assert(hf != NULL && "hf is NULL");
    if (!hf) return false;
    return hash_insert_internal(hf, rec);
}

bool hash_search(HashFile* hf, int key, Record* out_rec) {
    assert(hf != NULL && "hf is NULL");
    assert(out_rec != NULL && "out_rec is NULL");
    if (!hf) return false;
    int h = key & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    HashBucket b;
    fseek(hf->data_file, offset, SEEK_SET);
    fread(&b, sizeof(HashBucket), 1, hf->data_file);

    for (int i = 0; i < b.record_count; i++) {
        if (b.records[i].id == key) {
            if (out_rec) *out_rec = b.records[i];
            return true;
        }
    }
    return false;
}

bool hash_delete(HashFile* hf, int key) {
    assert(hf != NULL && "hf is NULL");
    if (!hf) return false;
    int h = key & ((1 << hf->global_depth) - 1);
    long offset = hf->directory[h];
    
    HashBucket b;
    fseek(hf->data_file, offset, SEEK_SET);
    fread(&b, sizeof(HashBucket), 1, hf->data_file);

    for (int i = 0; i < b.record_count; i++) {
        if (b.records[i].id == key) {
            b.records[i] = b.records[b.record_count - 1];
            b.record_count--;
            fseek(hf->data_file, offset, SEEK_SET);
            fwrite(&b, sizeof(HashBucket), 1, hf->data_file);
            return true;
        }
    }
    return false;
}

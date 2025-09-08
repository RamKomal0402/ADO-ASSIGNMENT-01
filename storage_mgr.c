#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "storage_mgr.h"
#include "dberror.h"

/* Global storage system state */
typedef struct {
    unsigned char initialized;
    unsigned int session_id;
} StorageSystem;

static StorageSystem storage_sys = {0, 0};

/* Internal constants */
#define BLOCK_CAPACITY PAGE_SIZE
#define STORAGE_SIGNATURE 0xDEADBEEF
#define MAX_FILENAME_LENGTH 256

/* Error checking macro */
#define CHECK_NULL(ptr) if(!(ptr)) return RC_FILE_HANDLE_NOT_INIT

/* Memory allocation wrapper */
static void* safe_malloc(size_t bytes) {
    void* ptr = malloc(bytes);
    if (ptr) {
        memset(ptr, 0, bytes);
    }
    return ptr;
}

/* String utilities */
static char* copy_filename(const char* original) {
    if (!original) return NULL;
    
    size_t length = strlen(original);
    if (length >= MAX_FILENAME_LENGTH) return NULL;
    
    char* copy = (char*)malloc(length + 1);
    if (copy) {
        memcpy(copy, original, length);
        copy[length] = '\0';
    }
    return copy;
}

/* File size calculation */
static int compute_total_pages(FILE* stream) {
    long saved_position = ftell(stream);
    
    fseek(stream, 0L, SEEK_END);
    long total_size = ftell(stream);
    fseek(stream, saved_position, SEEK_SET);
    
    return (int)(total_size / BLOCK_CAPACITY);
}

/* Page validation */
static int valid_page_range(int page, int max_pages) {
    return (page >= 0 && page < max_pages);
}

/* File handle verification */
static RC check_handle_validity(SM_FileHandle *handle) {
    if (!handle) return RC_FILE_HANDLE_NOT_INIT;
    if (!handle->mgmtInfo) return RC_FILE_HANDLE_NOT_INIT;
    return RC_OK;
}

/* Initialize the storage management system */
void initStorageManager(void) {
    if (storage_sys.initialized) {
        printf("Storage system already active (session %u)\n", storage_sys.session_id);
        return;
    }
    
    storage_sys.initialized = 1;
    storage_sys.session_id = (unsigned int)time(NULL) & 0xFFFF;
    
    printf("Storage Manager Online - Session %u\n", storage_sys.session_id);
}

/* Create a new page file */
RC createPageFile(char *fileName) {
    if (!fileName || strlen(fileName) == 0) {
        return RC_FILE_NOT_FOUND;
    }
    
    FILE* output_file = fopen(fileName, "wb");
    if (!output_file) {
        return RC_FILE_NOT_FOUND;
    }
    
    char* blank_page = (char*)safe_malloc(BLOCK_CAPACITY);
    if (!blank_page) {
        fclose(output_file);
        return RC_WRITE_FAILED;
    }
    
    size_t written_bytes = fwrite(blank_page, sizeof(char), BLOCK_CAPACITY, output_file);
    free(blank_page);
    
    int file_closed = fclose(output_file);
    
    if (written_bytes != BLOCK_CAPACITY || file_closed != 0) {
        return RC_WRITE_FAILED;
    }
    
    return RC_OK;
}

/* Open an existing page file */
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    CHECK_NULL(fileName);
    CHECK_NULL(fHandle);
    
    FILE* input_stream = fopen(fileName, "r+b");
    if (!input_stream) {
        return RC_FILE_NOT_FOUND;
    }
    
    char* name_copy = copy_filename(fileName);
    if (!name_copy) {
        fclose(input_stream);
        return RC_WRITE_FAILED;
    }
    
    int page_count = compute_total_pages(input_stream);
    
    /* Initialize file handle */
    fHandle->fileName = name_copy;
    fHandle->totalNumPages = page_count;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = (void*)input_stream;
    
    return RC_OK;
}

/* Close the page file */
RC closePageFile(SM_FileHandle *fHandle) {
    RC status = check_handle_validity(fHandle);
    if (status != RC_OK) {
        return status;
    }
    
    FILE* stream = (FILE*)fHandle->mgmtInfo;
    int close_status = fclose(stream);
    
    /* Free allocated resources */
    if (fHandle->fileName) {
        free(fHandle->fileName);
        fHandle->fileName = NULL;
    }
    
    /* Reset handle state */
    fHandle->mgmtInfo = NULL;
    fHandle->totalNumPages = 0;
    fHandle->curPagePos = -1;
    
    return (close_status == 0) ? RC_OK : RC_FILE_NOT_FOUND;
}

/* Destroy page file */
RC destroyPageFile(char *fileName) {
    CHECK_NULL(fileName);
    
    int delete_result = remove(fileName);
    return (delete_result == 0) ? RC_OK : RC_FILE_NOT_FOUND;
}

/* Read block from file */
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    RC validation = check_handle_validity(fHandle);
    if (validation != RC_OK) return validation;
    
    CHECK_NULL(memPage);
    
    if (!valid_page_range(pageNum, fHandle->totalNumPages)) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    FILE* stream = (FILE*)fHandle->mgmtInfo;
    long offset = (long)pageNum * BLOCK_CAPACITY;
    
    if (fseek(stream, offset, SEEK_SET) != 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    
    size_t read_count = fread(memPage, sizeof(char), BLOCK_CAPACITY, stream);
    
    /* Zero-fill incomplete reads */
    if (read_count < BLOCK_CAPACITY) {
        memset((char*)memPage + read_count, 0, BLOCK_CAPACITY - read_count);
    }
    
    fHandle->curPagePos = pageNum;
    return RC_OK;
}

/* Get current block position */
int getBlockPos(SM_FileHandle *fHandle) {
    return fHandle ? fHandle->curPagePos : -1;
}

/* Read first block */
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

/* Read previous block */
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    CHECK_NULL(fHandle);
    return readBlock(fHandle->curPagePos - 1, fHandle, memPage);
}

/* Read current block */
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    CHECK_NULL(fHandle);
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

/* Read next block */
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    CHECK_NULL(fHandle);
    return readBlock(fHandle->curPagePos + 1, fHandle, memPage);
}

/* Read last block */
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    CHECK_NULL(fHandle);
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

/* Write block to file */
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    RC validation = check_handle_validity(fHandle);
    if (validation != RC_OK) return validation;
    
    CHECK_NULL(memPage);
    
    if (!valid_page_range(pageNum, fHandle->totalNumPages)) {
        return RC_WRITE_FAILED;
    }
    
    FILE* stream = (FILE*)fHandle->mgmtInfo;
    long offset = (long)pageNum * BLOCK_CAPACITY;
    
    if (fseek(stream, offset, SEEK_SET) != 0) {
        return RC_WRITE_FAILED;
    }
    
    size_t write_count = fwrite(memPage, sizeof(char), BLOCK_CAPACITY, stream);
    if (write_count != BLOCK_CAPACITY) {
        return RC_WRITE_FAILED;
    }
    
    /* Sync to disk */
    fflush(stream);
    
    fHandle->curPagePos = pageNum;
    return RC_OK;
}

/* Write current block */
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    CHECK_NULL(fHandle);
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

/* Append empty block */
RC appendEmptyBlock(SM_FileHandle *fHandle) {
    RC validation = check_handle_validity(fHandle);
    if (validation != RC_OK) return validation;
    
    char* new_block = (char*)safe_malloc(BLOCK_CAPACITY);
    if (!new_block) {
        return RC_WRITE_FAILED;
    }
    
    FILE* stream = (FILE*)fHandle->mgmtInfo;
    
    if (fseek(stream, 0L, SEEK_END) != 0) {
        free(new_block);
        return RC_WRITE_FAILED;
    }
    
    size_t write_count = fwrite(new_block, sizeof(char), BLOCK_CAPACITY, stream);
    free(new_block);
    
    if (write_count != BLOCK_CAPACITY) {
        return RC_WRITE_FAILED;
    }
    
    /* Update metadata */
    fHandle->totalNumPages++;
    fHandle->curPagePos = fHandle->totalNumPages - 1;
    
    fflush(stream);
    return RC_OK;
}

/* Ensure capacity */
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    RC validation = check_handle_validity(fHandle);
    if (validation != RC_OK) return validation;
    
    if (numberOfPages <= fHandle->totalNumPages) {
        return RC_OK;
    }
    
    int blocks_needed = numberOfPages - fHandle->totalNumPages;
    
    for (int i = 0; i < blocks_needed; i++) {
        RC result = appendEmptyBlock(fHandle);
        if (result != RC_OK) {
            return result;
        }
    }
    
    return RC_OK;
}
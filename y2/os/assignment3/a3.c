#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

// SECTION FILE CONSTANTS
#define VARIANT             53425
#define SF_MAGIC            0x464D4478
#define MIN_SF_VER          (u_int8_t) 63
#define MAX_SF_VER          (u_int8_t) 183
#define MIN_SEC_NO          (u_int8_t) 8
#define MAX_SEC_NO          (u_int8_t) 17
#define TARGET_SECTION      (u_int8_t) 73
#define SEC_ARR_SIZE        7
#define LINE_ENDING         (char) 0xA
#define BUFFER_SIZE         255
#define SF_ALIGNMENT        5120

// PIPE CONSTANTS
#define RESP_PIPE_NAME      "RESP_PIPE_53425"
#define REQ_PIPE_NAME       "REQ_PIPE_53425"

// SHARED MEMORY CONSTANTS
#define SHM_NAME            "/iDDlqnDZ"
#define SHM_PERM            0664

// RETURN CODES
#define PIPE_CREAT_FAIL     -1
#define PIPE_REQ_FAIL       -2
#define PIPE_RESP_FAIL      -3
#define WRITE_FAIL          -4
#define READ_FAIL           -5
#define SHM_CREAT_FAIL      -6
#define SHM_TRUNC_FAIL      -7
#define MEM_MAP_FAIL        -8
#define FILE_OPEN_FAIL      -9
#define FILE_STAT_FAIL      -10
#define OUT_OF_BOUNDS       -11

const u_int8_t sectionTypes[] = {54, 73, 52, 85, 99, 99, 42};

#pragma pack(push, 1)

typedef struct _SF_FILE_HEADER {
    u_int32_t magic;
    u_int16_t headerSize;
    u_int8_t version;
    u_int8_t noOfSections;
} SF_FILE_HEADER;

typedef struct _SF_SECTION_HEADER {
    char sectName[11];
    u_int8_t sectType;
    u_int32_t sectOffset;
    u_int32_t sectSize;
} SF_SECTION_HEADER;

typedef struct _SHM_OBJECT {
    void* content;
    int fileDescriptor;
    u_int32_t size;
    int8_t error;
} SHM_OBJECT;

typedef struct _MMAP_OBJECT {
    void* content;
    int fileDescriptor;
    u_int32_t size;
    int8_t error;
} MMAP_OBJECT;

#pragma pack(pop)

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int allocateSharedMemory(SHM_OBJECT* sharedMem) {
    int retVal = 0;

    do {

        sharedMem->fileDescriptor = shm_open(SHM_NAME, O_RDWR | O_CREAT, SHM_PERM);
        if (sharedMem->fileDescriptor == -1) {
            retVal = SHM_CREAT_FAIL;
            sharedMem->error = SHM_CREAT_FAIL;
            break;
        }

        if (ftruncate(sharedMem->fileDescriptor, sharedMem->size) == -1) {
            retVal = SHM_TRUNC_FAIL;
            sharedMem->error = SHM_TRUNC_FAIL;
            break;
        }

        sharedMem->content = mmap(NULL, sharedMem->size, PROT_READ | PROT_WRITE, 
                                    MAP_SHARED, sharedMem->fileDescriptor, 0);

        if (sharedMem->content == MAP_FAILED) {
            retVal = MEM_MAP_FAIL;
            sharedMem->error = MEM_MAP_FAIL;
            break;
        }

        sharedMem->error = 0;

    } while (0);

    return retVal;
}

void deallocateSharedMemory(SHM_OBJECT* sharedMem) {

    if (sharedMem->error != SHM_CREAT_FAIL) {
        shm_unlink(SHM_NAME);
        sharedMem->fileDescriptor = -1;
    } else {
        return;
    }

    if (sharedMem->error != SHM_TRUNC_FAIL && sharedMem->error != MEM_MAP_FAIL) {
        munmap(sharedMem->content, sharedMem->size);
        sharedMem->content = (void*) -1;
        sharedMem->size = 0;
    }

}

int mapFile(MMAP_OBJECT* mappedFile, char* path) {
    int retVal = 0;
    struct stat fStat;

    do {

        mappedFile->fileDescriptor = open(path, O_RDWR);
        if (mappedFile->fileDescriptor == -1) {
            retVal = FILE_OPEN_FAIL;
            mappedFile->error = FILE_OPEN_FAIL;
            break;
        }

        if (fstat(mappedFile->fileDescriptor, &fStat) == -1) {
            retVal = FILE_STAT_FAIL;
            mappedFile->error = FILE_STAT_FAIL;
            break;
        }

        mappedFile->size = fStat.st_size;
        mappedFile->content = mmap(NULL, mappedFile->size, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, mappedFile->fileDescriptor, 0);

        if (mappedFile->content == MAP_FAILED) {
            retVal = MEM_MAP_FAIL;
            mappedFile->error = MEM_MAP_FAIL;
            break;
        }

        mappedFile->error = 0;

    } while (0);

    return retVal;
}

void unmapFile(MMAP_OBJECT* mappedFile) {
    
    if (mappedFile->error != FILE_OPEN_FAIL) {
        close(mappedFile->fileDescriptor);
        mappedFile->fileDescriptor = -1;
    } else {
        return;
    }

    if (mappedFile->error != MEM_MAP_FAIL) {
        munmap(mappedFile->content, mappedFile->size);
        mappedFile->content = (void*) -1;
        mappedFile->size = 0;
    }

}

int readSFOffset(MMAP_OBJECT* mappedFile, SHM_OBJECT* sharedMem, u_int32_t offset, u_int32_t size) {
    int retVal = 0;

    do {

        if (mappedFile == NULL) {
            retVal = MEM_MAP_FAIL;
            break;
        }

        if (mappedFile->error != 0) {
            retVal = MEM_MAP_FAIL;
            break;
        }

        if (sharedMem == NULL) {
            retVal = SHM_CREAT_FAIL;
            break;
        }

        if (sharedMem->error != 0) {
            retVal = SHM_CREAT_FAIL;
            break;
        }

        if (offset + size > mappedFile->size) {
            retVal = OUT_OF_BOUNDS;
            break;
        }

        if (size > sharedMem->size) {
            retVal = OUT_OF_BOUNDS;
            break;
        }

        memcpy(sharedMem->content, (u_int8_t*) mappedFile->content + offset, size);

    } while (0);

    return retVal;
}

int handleVerdict(char* task, char* verdict, int respDesc) {
    int retVal = 0;
    int respLen = 0;
    char writeBuf[BUFFER_SIZE] = { 0 };

    do {

        respLen = sprintf(writeBuf, "*%s*%s", task, verdict);
        if (respLen < 0) {
            retVal = WRITE_FAIL;
        }

        writeBuf[0] = strlen(task);
        writeBuf[strlen(task) + 1] = strlen(verdict);

        if (write(respDesc, writeBuf, respLen) == -1) {
            retVal = WRITE_FAIL;
            break;
        }

    } while (0);

    return retVal;
}

int readSFSection(MMAP_OBJECT* mappedFile, SHM_OBJECT* sharedMem, u_int8_t secNum, u_int32_t offset, u_int32_t size) {
    int retVal = 0;
    SF_FILE_HEADER* sfHdr = mappedFile->content;
    SF_SECTION_HEADER* sfSecHdrs = (SF_SECTION_HEADER*) ((u_int8_t*) mappedFile->content + sizeof(SF_FILE_HEADER));

    do {

        if (mappedFile == NULL) {
            retVal = MEM_MAP_FAIL;
            break;
        }

        if (mappedFile->error != 0) {
            retVal = MEM_MAP_FAIL;
            break;
        }

        if (sharedMem == NULL) {
            retVal = SHM_CREAT_FAIL;
            break;
        }

        if (sharedMem->error != 0) {
            retVal = SHM_CREAT_FAIL;
            break;
        }

        if (size > sharedMem->size) {
            retVal = OUT_OF_BOUNDS;
            break;
        }

        if (secNum < 1 || sfHdr->noOfSections < secNum) {
            retVal = OUT_OF_BOUNDS;
            break;
        }

        --secNum;

        if (offset + size > sfSecHdrs[secNum].sectSize) {
            retVal = OUT_OF_BOUNDS;
            break;
        }

        memcpy(sharedMem->content, (u_int8_t*) mappedFile->content + sfSecHdrs[secNum].sectOffset + offset, size);

    } while (0);

    return retVal;
}

int readSFLogic(MMAP_OBJECT* mappedFile, SHM_OBJECT* sharedMem, u_int32_t offset, u_int32_t size) {
    int retVal = 0;
    u_int32_t pageLimit = 0;
    u_int32_t totalOffset = 0;
    u_int32_t copiedBytes = 0;
    u_int32_t memOffset = 0;
    u_int8_t* content = (u_int8_t*) mappedFile->content;
    SF_FILE_HEADER* sfHdr = mappedFile->content;
    SF_SECTION_HEADER* sfSecHdrs = (SF_SECTION_HEADER*) ((u_int8_t*) mappedFile->content + sizeof(SF_FILE_HEADER));

    do {

        if (mappedFile == NULL) {
            retVal = MEM_MAP_FAIL;
            break;
        }

        if (mappedFile->error != 0) {
            retVal = MEM_MAP_FAIL;
            break;
        }

        if (sharedMem == NULL) {
            retVal = SHM_CREAT_FAIL;
            break;
        }

        if (sharedMem->error != 0) {
            retVal = SHM_CREAT_FAIL;
            break;
        }

        if (size > sharedMem->size) {
            retVal = OUT_OF_BOUNDS;
            break;
        }

        for (int i = 0; i < sfHdr->noOfSections && size; ++i) {
            pageLimit = (sfSecHdrs[i].sectSize + SF_ALIGNMENT - 1) / SF_ALIGNMENT * SF_ALIGNMENT;

            if (offset < pageLimit) {
                copiedBytes = min(pageLimit - offset, size);

                memcpy((u_int8_t*) sharedMem->content + memOffset, content + sfSecHdrs[i].sectOffset + offset,
                        copiedBytes);

                size -= copiedBytes;
                memOffset += copiedBytes;
            }

            offset = max(offset - pageLimit, 0);
            totalOffset += pageLimit;
        }

        if (size) {
            retVal = OUT_OF_BOUNDS;
            break;
        }

    } while (0);

    return retVal;
}

int main() {
    int retVal = 0;
    int reqDesc = -1;
    int respDesc = -1;
    u_int8_t reqLen = 0;
    int respLen = 0;
    char readBuf[BUFFER_SIZE] = { 0 };
    char writeBuf[BUFFER_SIZE] = { 0 };
    u_int32_t numBuffer = 0;
    SHM_OBJECT sharedMem = { 0, -1, 0, SHM_CREAT_FAIL };
    MMAP_OBJECT mappedFile = { 0, -1, 0, FILE_OPEN_FAIL };
    u_int32_t readOffset = 0;
    u_int8_t secNum = 0;

    do {

        if (mkfifo(RESP_PIPE_NAME, 0666) != 0) {
            retVal = PIPE_CREAT_FAIL;
            break;
        }

        if ((reqDesc = open(REQ_PIPE_NAME, O_RDONLY)) == -1) {
            fprintf(stderr, "The following error has occurred: %d\n", errno);
            retVal = PIPE_REQ_FAIL;
            break;
        }

        if ((respDesc = open(RESP_PIPE_NAME, O_WRONLY)) == -1) {
            retVal = PIPE_RESP_FAIL;
            break;
        }

        if (write(respDesc, "\7CONNECT", 8) == -1) {
            retVal = WRITE_FAIL;
            break;
        }

        printf("SUCCESS\n");



        while (strncmp(readBuf, "EXIT", 4) != 0 && retVal == 0) {

            if (read(reqDesc, &reqLen, 1) == -1) {
                retVal = READ_FAIL;
                break;
            }

            if (read(reqDesc, readBuf, reqLen) == -1) {
                retVal = READ_FAIL;
                break;
            }


            if (strncmp(readBuf, "PING", 4) == 0) {

                if ((respLen = sprintf(writeBuf, "\4PING\4PONG")) < 0) {
                    retVal = WRITE_FAIL;
                    break;
                }

                if (write(respDesc, writeBuf, respLen) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }

                numBuffer = VARIANT;

                if (write(respDesc, &numBuffer, 4) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }
            } else if (strncmp(readBuf, "CREATE_SHM", 10) == 0) {
                
                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = READ_FAIL;
                    break;
                }

                sharedMem.size = numBuffer;

                if (allocateSharedMemory(&sharedMem) != 0) {
                    retVal = handleVerdict("CREATE_SHM", "ERROR", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                } else {
                    retVal = handleVerdict("CREATE_SHM", "SUCCESS", respDesc);
                    if (retVal != 0)
                        break;
                }


            } else if (strncmp(readBuf, "WRITE_TO_SHM", 12) == 0) {

                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = READ_FAIL;
                    break;
                }


                if (numBuffer + 4 > sharedMem.size) {
                    retVal = handleVerdict("WRITE_TO_SHM", "ERROR", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                } else {
                    u_int32_t offset = numBuffer;
                    u_int8_t* var = sharedMem.content;

                    if (read(reqDesc, &numBuffer, 4) == -1) {
                        retVal = READ_FAIL;
                        break;
                    }

                    *(u_int32_t*)(var + offset) = numBuffer;

                    retVal = handleVerdict("WRITE_TO_SHM", "SUCCESS", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                }
            } else if (strncmp(readBuf, "MAP_FILE", 8) == 0) {

                if (read(reqDesc, &reqLen, 1) == -1) {
                    retVal = READ_FAIL;
                    break;
                }

                if (read(reqDesc, readBuf, reqLen) == -1) {
                    retVal = READ_FAIL;
                    break;
                }

                if (mapFile(&mappedFile, readBuf) != 0) {
                    retVal = handleVerdict("MAP_FILE", "ERROR", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                } else {
                    retVal = handleVerdict("MAP_FILE", "SUCCESS", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                }


            } else if (strncmp(readBuf, "READ_FROM_FILE_OFFSET", 21) == 0) {

                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }

                readOffset = numBuffer;

                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }

                if (readSFOffset(&mappedFile, &sharedMem, readOffset, numBuffer) != 0) {
                    retVal = handleVerdict("READ_FROM_FILE_OFFSET", "ERROR", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                } else {
                    retVal = handleVerdict("READ_FROM_FILE_OFFSET", "SUCCESS", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                }
            } else if (strncmp(readBuf, "READ_FROM_FILE_SECTION", 22) == 0) {
                
                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }

                secNum = numBuffer;

                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }
                
                readOffset = numBuffer;

                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }

                if (readSFSection(&mappedFile, &sharedMem, secNum, readOffset, numBuffer) != 0) {
                    retVal = handleVerdict("READ_FROM_FILE_SECTION", "ERROR", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                } else {
                    retVal = handleVerdict("READ_FROM_FILE_SECTION", "SUCCESS", respDesc);
                    if (retVal != 0) {
                        break;
                    }
                }
            } else if (strncmp(readBuf, "READ_FROM_LOGICAL_SPACE_OFFSET", 30) == 0) {

                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }

                readOffset = numBuffer;

                if (read(reqDesc, &numBuffer, 4) == -1) {
                    retVal = WRITE_FAIL;
                    break;
                }

                if (readSFLogic(&mappedFile, &sharedMem, readOffset, numBuffer) != 0) {
                    retVal = handleVerdict("READ_FROM_LOGICAL_SPACE_OFFSET", "ERROR", respDesc);
                    if (retVal != 0){
                        break;
                    }
                } else {
                    retVal = handleVerdict("READ_FROM_LOGICAL_SPACE_OFFSET", "SUCCESS", respDesc);
                    if (retVal != 0){
                        break;
                    }
                }

            } else {
                sprintf(readBuf, "EXIT");
            }

        }

    } while (0);



    if (retVal == PIPE_CREAT_FAIL) {
        fprintf(stderr, "ERROR\ncannot create the response pipe\n");
        return retVal;
    } else {
        unlink(RESP_PIPE_NAME);
    }

    if (retVal == PIPE_REQ_FAIL) {
        fprintf(stderr, "ERROR\ncannot open the request pipe\n");
        return retVal;
    } else {
        close(reqDesc);
    }

    if (retVal == PIPE_RESP_FAIL) {
        fprintf(stderr, "ERROR\ncannot open the response pipe\n");
        return retVal;
    } else {
        close(respDesc);
    }

    if (retVal == WRITE_FAIL) {
        fprintf(stderr, "ERROR\ncannot write to response pipe\n");
    }

    if (retVal == READ_FAIL) {
        fprintf(stderr, "ERROR\ncannot read from the request pipe\n");
    }

    if (retVal == SHM_CREAT_FAIL) {
        fprintf(stderr, "ERROR\ncannot create shared memory object\n");
    }

    if (retVal == SHM_TRUNC_FAIL) {
        fprintf(stderr, "ERROR\ncannot truncate shared memory object\n");
    }

    deallocateSharedMemory(&sharedMem);
    unmapFile(&mappedFile);

    return retVal;
}
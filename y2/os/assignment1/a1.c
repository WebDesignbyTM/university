#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>


#define BUFFER_SIZE         2048
#define VARIANT             53425
#define SF_MAGIC            0x464D4478
#define MIN_SF_VER          (u_int8_t) 63
#define MAX_SF_VER          (u_int8_t) 183
#define MIN_SEC_NO          (u_int8_t) 8
#define MAX_SEC_NO          (u_int8_t) 17
#define TARGET_SECTION      (u_int8_t) 73
#define SEC_ARR_SIZE        7
#define LINE_ENDING         (char) 0xA

#define RECURSIVE_FLAG      (u_char) 0x1
#define EXEC_FLAG           (u_char) 0x2
#define SEC_SEARCH_FLAG     (u_char) 0x4
#define ROOT_CALL_FLAG      (u_char) 0x8

#define PARSE_FORMAT        (u_char) 0x1
#define PARSE_SECTION       (u_char) 0x2
#define PARSE_FILTER        (u_char) 0x4

#define CORRUPT_MAGIC       (u_int8_t) 0x1
#define CORRUPT_VERSION     (u_int8_t) 0x2
#define CORRUPT_NO_SEC      (u_int8_t) 0x4
#define CORRUPT_SEC_TYPE    (u_int8_t) 0x8

#define EXTR_WRNG_FILE      (u_int8_t) 0x1
#define EXTR_WRNG_SEC       (u_int8_t) 0x2
#define EXTR_WRNG_LINE      (u_int8_t) 0x4

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

#pragma pack(pop)

int min(int a, int b) {
    return a < b ? a : b;
}

int parseSectionFile(char* path, u_char flags, int section, int line);

/**
 * @brief List all files in given folder that respect certain requirements
 * 
 * @param[in] flags constraints bitmap 
 * @param[in] path directory path
 * @param[in] namePrefix required file name prefix
 * @return 0 for success, non-zero otherwise
 */
int listFiles(u_char flags, char* path, char* namePrefix) {
    char currentPath[2 * NAME_MAX] = { 0 };
    int retVal = 0;
    DIR *dp;
    struct dirent* currentEntry = NULL;
    struct stat currentStat;

    do {
        if (path == NULL) {
            perror("ERROR\ninvalid directory path");
            retVal = 1;
            break;
        }
        if (namePrefix == NULL) {
            perror("ERROR\ninvalid name prefix");
            retVal = 1;
            break;
        }

        dp = opendir(path);
        if (dp == NULL) {
            perror("ERROR\ninvalid directory path");
            retVal = 2;
            break;
        }

        if ((flags & ROOT_CALL_FLAG) != 0)
            printf("SUCCESS\n");

        while ((currentEntry = readdir(dp)) != NULL) {

            if (strcmp(currentEntry->d_name, ".") == 0 || strcmp(currentEntry->d_name, "..") == 0 )
                continue;

            snprintf(currentPath, sizeof(currentPath), "%s/%s", path, currentEntry->d_name);
            stat(currentPath, &currentStat);

            if (currentEntry->d_type == DT_DIR) {
                if ((flags & RECURSIVE_FLAG) != 0) 
                    listFiles(flags & (~ROOT_CALL_FLAG), currentPath, namePrefix);
            }
            
            if ((flags & SEC_SEARCH_FLAG) == 0) {
                if (strstr(currentEntry->d_name, namePrefix) == currentEntry->d_name) {
                    if (((flags & EXEC_FLAG) != 0 && (S_IXUSR & currentStat.st_mode) != 0) 
                        || (flags & EXEC_FLAG) == 0)
                        printf("%s\n", currentPath);
                }
            } else {
                if (currentEntry->d_type != DT_DIR && parseSectionFile(currentPath, PARSE_FILTER, 0, 0))
                    printf("%s\n", currentPath);
            }
        }

        closedir(dp);

    } while(0);

    return retVal;
}

/**
 * @brief Extract relevant information from a SF file
 * 
 * @param path[in] file path
 * @param flags[in] requested parsing information
 * @param section[in] required section
 * @param line[in] required line
 * @return negative for failure, 0 for success, 1 for found
 */
int parseSectionFile(char* path, u_char flags, int section, int line) {
    char writeBuf[12] = { 0 };
    char readBuf[BUFFER_SIZE] = { 0 };
    int fd = -1;
    int retVal = 0;
    int readBytes = 0;
    SF_FILE_HEADER header;
    SF_SECTION_HEADER sections[MAX_SEC_NO];
    u_int8_t corruptedFields = 0;
    u_int8_t extractionErrors = 0;

    do {
        if ((fd = open(path, O_RDONLY)) < 0) {
            perror("ERROR\ninvalid file path");
            retVal = -1;
            break;
        }

        if (read(fd, &header, sizeof(SF_FILE_HEADER)) == -1) {
            perror("ERROR\nsomething really went wrong");
            retVal = -2;
            break;
        }

        if (header.magic != SF_MAGIC)
            corruptedFields |= CORRUPT_MAGIC;

        if (header.version < MIN_SF_VER || MAX_SF_VER < header.version)
            corruptedFields |= CORRUPT_VERSION;

        if (header.noOfSections < MIN_SEC_NO || MAX_SEC_NO < header.noOfSections)
            corruptedFields |= CORRUPT_NO_SEC;

        for (int i = 0; i < min(header.noOfSections, MAX_SEC_NO); ++i) {
            if (read(fd, &sections[i], sizeof(SF_SECTION_HEADER)) == -1) {
                perror("ERROR\nsomething really went wrong");
                retVal = -2;
                break;
            }

            int ok = 0;
            for (int j = 0; j < SEC_ARR_SIZE; ++j)
                if (sectionTypes[j] == sections[i].sectType)
                    ok = 1;
            
            if (!ok)
                corruptedFields |= CORRUPT_SEC_TYPE;
        }

        if (flags == PARSE_FORMAT) {
            if (corruptedFields == 0) {
                printf("SUCCESS\nversion=%u\nnr_sections=%u\n", header.version, header.noOfSections);
                for (int i = 0; i < header.noOfSections; ++i) {
                    strncpy(writeBuf, sections[i].sectName, sizeof(writeBuf) - 1);
                    printf("section%d: %s %u %u\n", i + 1, writeBuf, 
                            sections[i].sectType, sections[i].sectSize);
                }
            } else {
                printf("ERROR\nwrong ");
                int leadingBar = 0;
                if (corruptedFields & CORRUPT_MAGIC) {
                    printf("magic");
                    leadingBar = 1;
                }
                if (corruptedFields & CORRUPT_VERSION) {
                    if (leadingBar)
                        printf("|");
                    printf("version");
                    leadingBar = 1;
                }
                if (corruptedFields & CORRUPT_NO_SEC) {
                    if (leadingBar)
                        printf("|");
                    printf("sect_nr");
                    leadingBar = 1;
                }
                if (corruptedFields & CORRUPT_SEC_TYPE) {
                    if (leadingBar)
                        printf("|");
                    printf("sect_types");
                }
            }
        } else if (flags == PARSE_SECTION) {
            if (corruptedFields != 0)
                extractionErrors |= EXTR_WRNG_FILE;

            if (header.noOfSections < section) {
                extractionErrors |= EXTR_WRNG_SEC;
            } else {

                if (lseek(fd, sections[section-1].sectOffset, SEEK_SET) == -1) {
                    perror("ERROR\ninvalid section offset");
                    retVal = -3;
                    break;
                }

                int secIdx = sections[section-1].sectSize;
                --line;
                int ok = 0;
                
                while ((readBytes = read(fd, readBuf, sizeof(readBuf))) != 0 && line > -1 && secIdx) {

                    for (int i = 0; i < readBytes && line > -1 && secIdx; ++i, --secIdx) {
                        if (readBuf[i] == LINE_ENDING) {
                            --line;
                        } else {
                            if (!line) {
                                if (!ok) {
                                    printf("SUCCESS\n");
                                    ok = 1;
                                }
                                printf("%c", readBuf[i]);
                            }
                        }
                    }
                }


                if (line > 0) 
                    extractionErrors |= EXTR_WRNG_LINE;
            }

            if (extractionErrors) {
                printf("ERROR\nwrong ");
                int leadingBar = 0;
                if (extractionErrors & EXTR_WRNG_FILE) {
                    printf("file");
                    leadingBar = 1;
                }
                if (extractionErrors & EXTR_WRNG_SEC) {
                    if (leadingBar)
                        printf("|");
                    printf("section");
                    leadingBar = 1;
                }
                if (extractionErrors & EXTR_WRNG_LINE) {
                    if (leadingBar)
                        printf("|");
                    printf("line");
                }   
            }
        } else if (flags == PARSE_FILTER) {
            if (corruptedFields)
                break;
            
            for (int i = 0; i < header.noOfSections; ++i)
                if (sections[i].sectType == TARGET_SECTION) {
                    retVal = 1;
                    break;
                }
        }

        close(fd);
    } while (0);

    return retVal;
}

int main(int argc, char **argv){
    if (argc < 2) {
        printf("Usage: %s [OPTIONS: variant, list, parse, extract, findall] "
                "[PARAMETERS: name_starts_with, recursive, has_perm_execute, section, line]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "variant") == 0) {

        printf("%d\n", VARIANT);

    } else {

        u_char command = 0;
        u_char flags = ROOT_CALL_FLAG;
        char path[NAME_MAX] = { 0 };
        char namePrefix[NAME_MAX] = { 0 };
        int section = 0;
        int line = 0;

        for (int i = 1; i < argc; ++i) {

            if (strstr(argv[i], "path") != NULL) {

                strtok(argv[i], "=");
                char *pPath = strtok(NULL, "=");
                snprintf(path, NAME_MAX, "%s", pPath);

            } else if (strstr(argv[i], "name_starts_with") != NULL) {

                strtok(argv[i], "=");
                char *pName = strtok(NULL, "=");
                snprintf(namePrefix, NAME_MAX, "%s", pName);

            } else if (strcmp(argv[i], "recursive") == 0) {
                flags |= RECURSIVE_FLAG;

            } else if (strcmp(argv[i], "has_perm_execute") == 0) {
                flags |= EXEC_FLAG;

            } else if (strstr(argv[i], "section") != NULL) {

                strtok(argv[i], "=");
                section = atoi(strtok(NULL, "="));

            } else if (strstr(argv[i], "line") != NULL) {

                strtok(argv[i], "=");
                line = atoi(strtok(NULL, "="));

            } else if (strcmp(argv[i], "list") == 0) { 
                command = 1;

            } else if (strcmp(argv[i], "parse") == 0) {
                command = 2;

            } else if (strcmp(argv[i], "extract") == 0) {
                command = 3;

            } else if (strcmp(argv[i], "findall") == 0) {
                flags |= RECURSIVE_FLAG;
                flags |= SEC_SEARCH_FLAG;
                command = 4;
            }
        }

        if (command == 1) {
            listFiles(flags, path, namePrefix);
        } else if (command == 2) {
            parseSectionFile(path, PARSE_FORMAT, section, line);
        } else if (command == 3) {
            parseSectionFile(path, PARSE_SECTION, section, line);
        } else if (command == 4) {
            listFiles(flags, path, namePrefix);
        }
    }

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BLOCKSIZE       512
#define BLOCKING_FACTOR 20
#define RECORDSIZE      10240

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

typedef struct tar_t tar_t;
typedef struct tar_header tar_header;

struct tar_header
{                              /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    char padding[12];             /* 500 */
};

struct tar_t {
    int (*read)(tar_t *tar, void *data, unsigned size);
    int (*write)(tar_t *tar, const void *data, unsigned size);
    int (*seek)(tar_t *tar, unsigned pos);
    int (*close)(tar_t *tar);
    void *stream;
    unsigned pos;
    unsigned remaining_data;
    unsigned last_header;
};

const char typeflag[] = {'0', '1', '2', '3', '4', '5', '6', '7', 'D', 'K', 'L', 'M', 'N', 'S', 'V'};
const int mode[] = {04000, 02000, 01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, 00002, 00001};

int main(int argc, char* argv[])
{
    srand(time(NULL));   // Initialization of random number

    const size_t size = 2000;

    struct tar_header tar;
    char *text1 = malloc(sizeof(char) * (size +1));
    random_strings(size, text1);
    char *text2 = malloc(sizeof(char) * (size +1));
    random_strings(size, text2);
        
    return 0;
}

unsigned int calculate_checksum(struct tar_header* entry){
     memset(entry->chksum, ' ', 8);
    
    unsigned int check = 0;
    unsigned char* raw = (unsigned char*) entry;
    for(int i = 0; i < 512; i++){
        check += raw[i];
    }

    snprintf(entry->chksum, sizeof(entry->chksum), "%06o0", check);

    entry->chksum[6] = '\0';
    entry->chksum[7] = ' ';
    return check;
}

int random_number(int start, int end) {
    int result = 0;
    int min_num = 0;
    int max_num = 0;
    if (start < end) {
        min_num = start;
        max_num = end + 1; // To include end number in the ouput
    } else {
        min_num = end + 1; // To include the last number in the ouput
        max_num = start;
    }
    result = (rand() % (max_num - min_num)) + min_num;
    return result;
}

void random_strings(size_t length, char *randomString) { // const size_t length, supra

    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    if (length) {
        if (randomString) {
            int l = (int) (sizeof(charset) -1);
            for (int n = 0;n < length;n++) {
                int key = rand() % l;          // per-iteration instantiation
                randomString[n] = charset[key];
            }

            randomString[length] = '\0';
        }
    }
}

static int write_file(tar_t *tar, void *data, size_t size) {
    size_t res = fwrite(data, 1, size, tar->stream);
    return (size == res) ? 0 : -1;
}

static int read_file(tar_t *tar, void *data, size_t size) {
    size_t res = fread(data, 1, size, tar->stream);
    return (size == res) ? 0 : -1;
}

static int seek_file(tar_t *tar, int offset) {
    size_t res = fseek(tar->stream, offset, SEEK_SET);
    return res;
}

static int close_file(tar_t *tar) {
    return fclose(tar->stream); 
}

int open_tar(tar_t *tar, char *archive_name) {
    tar_header *head;

    memset(tar, 0, sizeof(*tar));
    tar->stream = fopen(archive_name, "wb");
    return 0;
}

int write_tar_header (tar_header *tar, char *archive_name) {
    struct stat st;

    return 0;
}

int write_tar_data (tar_t *tar, char *name, unsigned size) {
    tar_header header;
    memset(&header, 0, sizeof(header));
    strcpy(header.name, name);
    header.size = size;

    return 0;
}

int write_file_header (tar_header *tar, char *archive_name) {
    return 0;
}

int close_file (tar_header *tar, char *archive_name) {
    return 0;
}

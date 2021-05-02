#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

const char typeflag[] = {'0', '1', '2', '3', '4', '5', '6', '7', 'D', 'K', 'L', 'M', 'N', 'S', 'V'};
const char version[] = "00";

struct tar_t
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


int main(int argc, char* argv[])
{
    srand(time(NULL));   // Initialization of random number

    const size_t size = 2000;
    char *text = malloc(sizeof(char) * (size +1));
    random_strings(size, text);
    
    return 0;
}

unsigned int calculate_checksum(struct tar_t* entry){
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

int save_the_file(struct tar_t *archive, char *name) {
    FILE *outfile;

    //open file for writing
    outfile = fopen(name, "w");
    if(outfile == NULL){
        fprintf(stderr, "\nError open file\n");
        exit(1);
    }
    
    fwrite(&archive, sizeof(struct tar_t), 1, outfile);

    if(fwrite != 0)
        printf("write_header_old_tar_null Written sucessfully");
    
    return 0;
}

int write_input_entries(int fd, struct tar_t ** archive, struct tar_t ** head, char * files[], int * offset, size_t filecount){
    struct tar_t ** input = archive;
    for (int i = 0; i < filecount; i++)
    {
        if ()
    }
    
    return 0;
}

int write_tar_format(struct tar_t * input, char * filename) {
    if (!input)
    {
        ERROR("Bad input");
    }
    
    
}

int rename_working_files() {
    return 0;
}
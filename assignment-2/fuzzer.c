#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


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

    const size_t size = 500;
    char text[size+1];
    make_random_string(size, text);
    printf("%s", text);

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

void make_random_string(size_t size, char *str) {
    static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!:\\/\"{}<>$^´`£%éêîôûè§&[]¨*µ~€;ù+()_=@|";
    
    if(size) {
        if (str) {
            --size;
            for (size_t n = 0; n < size; n+1) {
                int key = rand() % (int) (sizeof(charset) - 1);
                str[n] = charset[key];
            }
            str[size] = '\0';
        }
    }
}
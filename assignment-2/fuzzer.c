#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

#define BLOCKSIZE       512
#define BLOCKING_FACTOR 20
#define RECORDSIZE      10240



#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* Values used in typeflag field.  */
#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */

#define XHDTYPE  'x'            /* Extended header referring to the
                                   next file in the archive */
#define XGLTYPE  'g'            /* Global extended header */

/* Bits used in the mode field, values in octal.  */
#define TSUID    04000          /* set UID on execution */
#define TSGID    02000          /* set GID on execution */
#define TSVTX    01000          /* reserved */
                                /* file permissions */
#define TUREAD   00400          /* read by owner */
#define TUWRITE  00200          /* write by owner */
#define TUEXEC   00100          /* execute/search by owner */
#define TGREAD   00040          /* read by group */
#define TGWRITE  00020          /* write by group */
#define TGEXEC   00010          /* execute/search by group */
#define TOREAD   00004          /* read by other */
#define TOWRITE  00002          /* write by other */
#define TOEXEC   00001          /* execute/search by other */

const unsigned typeflags[] = {REGTYPE, AREGTYPE, LNKTYPE, SYMTYPE, CHRTYPE, BLKTYPE, DIRTYPE, FIFOTYPE, CONTTYPE, XHDTYPE, XGLTYPE};
const mode_t modes[] = {04000, 02000, 01000, 00400, 00200, 00100, 00040, 00020, 00010, 00004, 00002, 00001};

struct tar_header {               /* byte offset */
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

struct tar_t 
{
    unsigned pos;
    unsigned remaining_data;
    unsigned last_header;
    void * stream;
};


int main(int argc, char* argv[])
{
    srand(time(NULL));   // Initialization of random number

    if (argc < 2){
        printf("Failure: There are no arguments\n");
        return -1;
    }
    int rv = 0;
    char cmd[51];
    strncpy(cmd, argv[1], 25);
    cmd[26] = '\0';
    int nbfound = 0, nbmax = 3, cpt = 0;
    while (nbfound <=5 & cpt < nbmax) {
        char *id;
        char archive[15];
        snprintf(archive, 15, "archive%d.tar", cpt);
        if (cpt == 1) {
            if_512_bytes(archive);
        }
        else if (cpt == 0)
        {
            if_data_failed(archive);
        }
        else if (cpt == 2) 
        {
            create_correct_tar_files(archive, DIRTYPE, 0664);
        }
        
        
        char for_command[strlen(archive) + 1];

        strcpy(for_command, " ");
        strcat(for_command, archive);
        strncat(cmd, for_command, 25);
        char buf[37];
        FILE *fp;

        if ((fp = popen(cmd, "r")) == NULL) {
            printf("Error opening pipe!\n");
            return -1;
        }

        if(fgets(buf, 37, fp) == NULL) {
            printf("No output\n");
            goto finally;
        }
        if(strstr(buf, "*** The program has crashed ***\n")) {
            nbfound++;
            char new_name[30];
            strcpy(new_name, "success_");
            strcat(new_name, archive);

            // success file already exists, remove it
            if( access( new_name, F_OK ) == 0 ) {
                remove(new_name);
            } 
            // copy the successful file into another file
            FILE *fptr1, *fptr2;
            fptr1 = fopen(archive, "r");
            fptr2 = fopen(new_name, "w");
            char c = fgetc(fptr1);
            while (c != EOF ) {
                fputc(c, fptr2);
                c = fgetc(fptr1);
            }
            fclose(fptr1);
            fclose(fptr2); 
            printf("*** The program has crashed with ***\n");  
            goto finally;
        } else {
            printf("Crash message\n");
            rv = 1;
            goto finally;
        }
        finally:
        if(pclose(fp) == -1) {
            printf("Command not found\n");
            rv = -1;
        }
        
        cpt++;
    }
    printf("We found  %d crashes\n", nbfound);
    return rv;
}

// To calculate the checksum
unsigned int calculate_checksum(struct tar_header * entry){
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

// To generate random integer: This is useful to generate random string 
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

// To generate random string for our text-file
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

/* 
    To open the stream file
    *tar: the struct data used to save the structure of the tar file
    *filename: the filename of the archive
*/
void prepare_tar(struct tar_t *tar, char *filename){
    char mode[] = "wb";
    tar->stream = fopen(filename, mode);
}


/*
    This function elaborate the content of the archive header, complete its content and save it in the stream file.
    mode: the mode used during the initiation of the 
*/
int tar_write_header(struct tar_t *tar, char *name, unsigned size, mode_t mode, char type){
    struct tar_header hdr;
	struct passwd *pw = getpwuid(getuid());
	struct group *gr = getgrgid(getgid());
    memset(&hdr, 0, sizeof(struct tar_header));
    strncpy(hdr.name, name, 100);
    snprintf(hdr.mode, sizeof(hdr.mode), "%07o", mode & 0777);
    snprintf(hdr.uid, sizeof(hdr.uid), "%07o", getuid());
    snprintf(hdr.gid, sizeof(hdr.gid), "%07o", getgid());
    snprintf(hdr.uname, sizeof(hdr.uname), "%s", pw->pw_name);
    snprintf(hdr.gname, sizeof(hdr.gname), "%s", gr->gr_name);
    snprintf(hdr.mtime, sizeof(hdr.mtime), "%011o", (int) time(NULL));
    memcpy(hdr.version, TVERSION, 2);
    memcpy(hdr.magic, TMAGIC, 6);
    memset(hdr.size, '0', 12);
    hdr.typeflag = type;
    if(type == REGTYPE) {
		snprintf(hdr.size, sizeof(hdr.size), "%011o", (unsigned)size);
	} 
    else if(type == SYMTYPE) {
		readlink(name, hdr.linkname, 100);
	} 
    else if(type == CHRTYPE) {
		snprintf(hdr.devmajor,  8, "%07o", 0U);
		snprintf(hdr.devminor,  8, "%07o", 0U);
	} 
    else if(type == BLKTYPE) {
		snprintf(hdr.devmajor,  8, "%07o", 0U);
		snprintf(hdr.devminor,  8, "%07o", 0U);
	} 
    else if(not_in_array(type, typeflags) ) {
        printf("Failure: unable to determine the filetype\n");
        return -1;
    }
    calculate_checksum(&hdr);
    tar->remaining_data = hdr.size;
    fwrite(&hdr, sizeof(hdr), 1, tar->stream);
    return 0;
}

// To check if an array contains a value
int not_in_array(unsigned *element, unsigned arr[]) {
    int n = sizeof(arr)/sizeof(mode_t);
    for (int i = 0; i < n; i++) {
        if (arr[i] == element) {
            return 0;
        }
    }
    return 1;
}

// Write the value or the content of a file
int tar_write_data(struct tar_t *tar, void *data, unsigned size) {
    fwrite(data, size, 1, tar->stream); 
    tar->remaining_data -= size;
    if (tar->remaining_data == 0) {
        return write_null_bytes(tar, (512 - tar->pos% 512)%512);
    }
    return 0;
}

int tar_finalize(struct tar_t *tar) {
    write_null_bytes(tar, sizeof(struct tar_header) * 2);
    fclose(tar->stream);
    return 0;
}

int write_null_bytes(struct tar_t *tar, int n) {
    int i, err;
    char nul = '\0';
    for (i = 0; i < n; i++) {
        if (fwrite(&nul, 1, 1, tar->stream) != 1) {
            printf("Error while writting null values\n");
            return -1;
        }
    }
    return 0;
} 

void removeSpaces(char *str) {
    int count = 0;
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i];
    str[count] = '\0';
}

void create_correct_tar_files(char *name, unsigned typeflag, unsigned mode){

    const size_t size = 2000;
    char *text = malloc(sizeof(char) * (size +1));
    random_strings(size, text);
    
    struct tar_t tar;

    prepare_tar(&tar, name);
    tar_write_header(&tar, "test.txt", strlen(text), mode, typeflag);
    tar_write_data(&tar, text, strlen(text));
    tar_finalize(&tar);
}

void if_512_bytes(char *name) {
    const size_t size = 2000;
    char *text = malloc(sizeof(char) * (size +1));
    random_strings(size, text);
    
    struct tar_t tar;

    prepare_tar(&tar, name);
    tar_write_header(&tar, "test.txt", strlen(text), 0664, REGTYPE);
    tar_write_data(&tar, text, strlen(text));
}

void if_data_failed(char *name) {
    const size_t size = 2000;
    char *text = malloc(sizeof(char) * (size +1));
    random_strings(size, text);
    
    struct tar_t tar;

    prepare_tar(&tar, name);
    tar_write_header(&tar, "test.txt", strlen(text), 0664, REGTYPE);
    tar_finalize(&tar);
}
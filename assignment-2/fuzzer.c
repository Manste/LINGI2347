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

// This are the different typeflags used to 
const char typeflags[] = {REGTYPE, AREGTYPE, LNKTYPE, SYMTYPE, CHRTYPE, BLKTYPE, DIRTYPE, FIFOTYPE, CONTTYPE, XHDTYPE, XGLTYPE};

// The structure of the archive header
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

//
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
    char cmd[61];
    strncpy(cmd, argv[1], 25);
    cmd[26] = '\0';
    int nbmax = 270;

    for (int cpt = 0; cpt < nbmax; cpt++) {
        char *id;
        char archive[15];
        snprintf(archive, 22, "archive%d.tar", cpt);

        // We are using cloned process to execute the command
        int pid = fork();
        if (pid == 0) {
            
            if (cpt == 0) {
                // This will create an archive file without completing adding at the end of the archive file two 512-byte blocks filled with binary zeros as an end-of-file marker
                if_512_bytes(archive);
            }
            else if (cpt == 1)
            {
                // Here, we create a archive file specifying, there exist a file test.txt but no data exist for the archive file.
                if_data_failed(archive);
            }
            else  if (cpt - 2 < 11)
            {
                // Try possible typeflags settings
                create_correct_tar_files(archive, typeflags[cpt - 2], 0664);
            }
            else if (cpt - 13 < 255) {
                char txt[cpt] ;
                random_strings(cpt, txt);
                // To try out type with possible string for the type 
                create_correct_tar_files(archive, txt, 0664);
            } 
            else if (cpt = 268) {
                // To try using wronly pointer and addresses for header
                create_tar_with_wrong_ptr(archive);
            }
            else {
                // Try with many text file in the archive file
                create_correct_tar_multiple_files(archive, REGTYPE, 0664, cpt);
            }
            
            char for_command[strlen(archive) + 1];

            strncpy(for_command, " ", 2);
            strncat(for_command, archive, 15);
            strncat(cmd, for_command, 25);
            char buf[33];
            FILE *fp;

            if ((fp = popen(cmd, "r")) == NULL) {
                printf("Error opening pipe!\n");
                return -1;
            }

            if(fgets(buf, sizeof buf, fp) == NULL) {
                printf("No output\n");
                goto finally;
            }
            if(strcmp(buf, "*** The program has crashed ***\n") == 0) {
                char new_name[30];
                strcpy(new_name, "success_");
                strcat(new_name, archive);

                // success file already exists, remove it
                if( access( new_name, F_OK ) == 0 ) {
                    remove(new_name);
                } 
                // copy the successful archive file into another file(sucess_archive)
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
                printf("*** The program has crashed with %s ***\n", new_name);  
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
            return 0;
        }
        else if(pid != -1){ // The parent is waiting
            wait(NULL);
        }
        else {
            perror("Error while calling the fork function");
        }
    }
    return rv;
}

/* 
    To calculate the checksum

    -> struct tar_header * entry:  the struct used to save the stream for the creation of the tar file
*/
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

/*
    To generate random integer: This is useful to generate random string 

    -> int start: The min value
    -> int end: The max value
*/
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

/*
    To generate random string for our text-file

    -> size_t length: The size defined for our string
    -> char *randomString: The pointer to memory allocated to the string
*/
void random_strings(size_t length, char *randomString) {

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

    -> struct tar_t *tar: the struct data used to save the structure of the tar file
    -> char *filename: the filename of the archive
*/
void prepare_tar(struct tar_t *tar, char *filename){
    char mode[] = "wb";
    tar->stream = fopen(filename, mode);
}


/*
    This function elaborate the content of the archive header, complete its content and save it in the stream file.
    
    -> unsigned mode: the mode used during the initiation of the tar file
    -> unsigned size: the size of the file
    -> unsigned type: the type of the tar_header
    -> unsigned *name: the name of the tar file
    -> struct tar_t *tar: the struct used to save the stream for the creation of the tar file
    -> struct tar_header hdr: the struct given the header information
*/
int tar_write_header(struct tar_t *tar, struct tar_header hdr, char *name, unsigned size, unsigned mode, unsigned type){
	struct passwd *pw = getpwuid(getuid());
	struct group *gr = getgrgid(getgid());
    memset(&hdr, 0, sizeof(struct tar_header));
    strncpy(hdr.name, name, 100);
    snprintf(hdr.mode, sizeof(hdr.mode), "%07o", mode );
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
    calculate_checksum(&hdr);
    tar->remaining_data = hdr.size;
    fwrite(&hdr, sizeof(hdr), 1, tar->stream);
    return 0;
}
/*
    We are wrong pointer for the header

    -> char *name: the name of the archive file
*/
void create_tar_with_wrong_ptr(char * name) {
    const size_t size = 2000;
    char *text = malloc(sizeof(char) * (size +1));
    random_strings(size, text);
    
    struct tar_t tar;
    struct tar_header hdr;

    prepare_tar(&tar, name);
    complete_header_only(&tar, hdr, "test.txt", strlen(text), 0664, REGTYPE);
    calculate_checksum(&hdr);
    tar.remaining_data = hdr.size;
    fwrite(&hdr, sizeof(hdr), 1, tar.stream);
    tar_write_data(&tar, text, strlen(text));
    tar_finalize(&tar);
}

/* 
    Write the value or the content of a file

    -> struct tar_t *tar: 
    -> void *data: The pointer for the data file
    -> unsigned size: 
*/
int tar_write_data(struct tar_t *tar, void *data, unsigned size) {
    fwrite(data, size, 1, tar->stream); 
    tar->remaining_data -= size;
    if (tar->remaining_data == 0) {
        return write_null_bytes(tar, (512 - tar->pos% 512)%512); // With "(512 - tar->pos% 512)%512" we calculate the number zero bytes to complete the file
    }
    return 0;
}

/*
    This function finalize the creation of tar file adding at the end of the archive file 
    two 512-byte blocks filled with binary zeros as an end-of-file marker. In the second part 
    of the function, we just close the stream creating the archive file.
    
    -> struct tar_t *tar: contains the stream for the creation of archive file.
*/
int tar_finalize(struct tar_t *tar) {
    write_null_bytes(tar, sizeof(struct tar_header) * 2);
    fclose(tar->stream);
    return 0;
}

/*
    This function complete the stream creating the archive file.

    -> struct tar_t *tar: contains the stream for the creation of archive file.
    -> int n: The number of necessary to complete the file
*/
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

/*
    This function create a simple archive file with one text file.

    -> char *name: The name of the archive file
    -> unsigned typeflag: the typeflag assigned to the archive file
    -> unsigned mode: The mode  fixed for the archive file
*/
void create_correct_tar_files(char *name, unsigned typeflag, unsigned mode){

    // Create text for simple file
    const size_t size = 2000;
    char *text = malloc(sizeof(char) * (size +1));
    random_strings(size, text);
    
    struct tar_t tar;
    struct tar_header hdr;

    prepare_tar(&tar, name);
    // Save a text file 
    tar_write_header(&tar, hdr, "test.txt", strlen(text), mode, typeflag);
    tar_write_data(&tar, text, strlen(text));
    tar_finalize(&tar);
}

/*
    This function create an archive file without completing adding at the end of the archive file 
    two 512-byte blocks filled with binary zeros as an end-of-file marker.

    -> char *name: the name of the archive file
*/
void if_512_bytes(char *name) {
    const size_t size = 2000;
    char *text = malloc(sizeof(char) * (size +1));
    random_strings(size, text);
    
    struct tar_t tar;
    struct tar_header hdr;

    prepare_tar(&tar, name);
    tar_write_header(&tar, hdr, "test.txt", strlen(text), 0664, REGTYPE);
    tar_write_data(&tar, text, strlen(text));
}

/*
    This function create a archive file that create an entry for data without saving the 
    corresponding data to that file.

    -> char *name: the name of the archive file
*/
void if_data_failed(char *name) {
    const size_t size = 2000;
    char *text = malloc(sizeof(char) * (size +1));
    random_strings(size, text);
    
    struct tar_t tar;
    struct tar_header hdr;

    prepare_tar(&tar, name);
    tar_write_header(&tar, hdr, "test.txt", strlen(text), 0664, REGTYPE);
    tar_finalize(&tar);
}

/*
    This function create a simple archive file with multiples files.

    -> char *name: The name of the archive file
    -> unsigned typeflag: the typeflag assigned to the archive file
    -> unsigned mode: The mode  fixed for the archive file
    -> int nb: The number of text files
*/
void create_correct_tar_multiple_files(char *name, unsigned typeflag, unsigned mode, int nb){
    
    struct tar_t tar;
    struct tar_header hdr;

    prepare_tar(&tar, name);
    // Save the text files
    for (int i = 0; i < nb; i++) {
        // Create text for simple file
        const size_t size = 2000;
        char *text = malloc(sizeof(char) * (size +1));
        random_strings(size, text);

        char *text_file = malloc(15);
        snprintf(text_file, 22, "text%d.txt", i);
        tar_write_header(&tar, hdr, *text_file, strlen(text), mode, typeflag);
        tar_write_data(&tar, text, strlen(text));
    }
    tar_finalize(&tar);
}

/*
    Here we are only complete the tar-header

    > unsigned mode: the mode used during the initiation of the tar file
    -> unsigned size: the size of the file
    -> unsigned type: the type of the tar_header
    -> unsigned *name: the name of the tar file
    -> struct tar_t *tar: the struct used to save the stream for the creation of the tar file
    -> struct tar_header hdr: the struct given the header information
*/
int complete_header_only(struct tar_t *tar, struct tar_header hdr, char *name, unsigned size, unsigned mode, unsigned type) {
    struct passwd *pw = getpwuid(getuid());
	struct group *gr = getgrgid(getgid());
    memset(&hdr, 0, sizeof(struct tar_header));
    strncpy(hdr.name, name, 100);
    snprintf(hdr.mode, sizeof(hdr.mode), "%07o", mode );
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
    return 0;    
}

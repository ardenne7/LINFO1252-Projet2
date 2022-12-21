#include "lib_tar.h"


unsigned int read_header(int tar_fd, tar_header_t* current) {
    unsigned int checksum_verif = 0;
    unsigned char* current_int = (unsigned char*) malloc(sizeof(unsigned int));
    read(tar_fd, current->name, 100);
    read(tar_fd, current->mode, 8);
    read(tar_fd, current->uid, 8);
    read(tar_fd, current->gid, 8);
    read(tar_fd, current->size, 12);
    read(tar_fd, current->mtime, 12);
    read(tar_fd, current->chksum, 8);
    read(tar_fd, &current->typeflag, 1);
    read(tar_fd, current->linkname, 100);
    read(tar_fd, current->magic, 6);
    read(tar_fd, current->version, 2);
    read(tar_fd, current->uname, 32);
    read(tar_fd, current->gname, 32);
    read(tar_fd, current->devmajor, 8);
    read(tar_fd, current->devminor, 8);
    read(tar_fd, current->prefix, 155);
    read(tar_fd, current->padding, 12);
    lseek(tar_fd, -512, SEEK_CUR);

    for (int i = 0; i < 512; i++) {
        if (i==148) {
            lseek(tar_fd, 8, SEEK_CUR);
            checksum_verif+=256;
            i+=8;
        }      
        read(tar_fd, current_int, 1);
        checksum_verif += *current_int;
    }

    return checksum_verif;
}

int count_slashes(char *str) {
  int count = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] == 47) {
      count++;
    }
  }
  return count;
}


/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    unsigned int checksum_verif;

    int end = lseek(tar_fd, 0, SEEK_END);
    lseek(tar_fd, 0, SEEK_SET);
    int count = 0;
    char first5bits[6];

    while (lseek(tar_fd,0,SEEK_CUR) < end) {
        tar_header_t* current = malloc(sizeof(tar_header_t));
        checksum_verif = read_header(tar_fd, current);
        first5bits[0] = current->magic[0];
        first5bits[1] = current->magic[1];
        first5bits[2] = current->magic[2];
        first5bits[3] = current->magic[3];
        first5bits[4] = current->magic[4];
        first5bits[5] = '\0';

        if (current->name[0] == '\0') {
            break;
        }
        count++;

        if (first5bits[0] != 'u' || first5bits[1] != 's' || first5bits[2] != 't' || first5bits[3] != 'a' || first5bits[4] != 'r' || first5bits[5] != '\0') {
            printf("%s \n", first5bits);
            return -1;
        }

        if (current->version[0] != '0' || current->version[1] != '0') {
            return -2;
        }

        if (checksum_verif != (unsigned int) TAR_INT(current->chksum)) {
            printf("%s \n", current->name);
            return -3;
        }

        int nb_blocks = TAR_INT(current->size) / 512;
        if (TAR_INT(current->size) % 512 != 0) {
            nb_blocks++;
        }
        lseek(tar_fd, nb_blocks*512, SEEK_CUR);
    }
    lseek(tar_fd, 0, SEEK_SET);

    return count;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    int end = lseek(tar_fd, 0, SEEK_END);
    lseek(tar_fd, 0, SEEK_SET);

    while (lseek(tar_fd,0,SEEK_CUR) < end) {
        tar_header_t* current = malloc(sizeof(tar_header_t));
        read_header(tar_fd, current);
        if (strcmp(current->name, path) == 0) {
            lseek(tar_fd, 0, SEEK_SET);
            return 1;
        }
        int nb_blocks = TAR_INT(current->size) / 512;
        if (TAR_INT(current->size) % 512 != 0) {
            nb_blocks++;
        }
        lseek(tar_fd, nb_blocks*512, SEEK_CUR);
    }
    lseek(tar_fd, 0, SEEK_SET);
    return 0;

}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    int end = lseek(tar_fd, 0, SEEK_END);
    lseek(tar_fd, 0, SEEK_SET);
    while(lseek(tar_fd,0,SEEK_CUR) < end) {
        tar_header_t* current = malloc(sizeof(tar_header_t));
        read_header(tar_fd, current);
        if (strcmp(current->name, path) == 0 && current->typeflag == '5') {
            lseek(tar_fd, 0, SEEK_SET);
            return 1;
        }
        int nb_blocks = TAR_INT(current->size) / 512;
        if (TAR_INT(current->size) % 512 != 0) {
            nb_blocks++;
        }
        lseek(tar_fd, nb_blocks*512, SEEK_CUR);
    }
    lseek(tar_fd, 0, SEEK_SET);
    return 0;
    
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    int end = lseek(tar_fd, 0, SEEK_END);
    lseek(tar_fd, 0, SEEK_SET);
    while(lseek(tar_fd,0,SEEK_CUR) < end) {
        tar_header_t* current = malloc(sizeof(tar_header_t));
        read_header(tar_fd, current);
        if (strcmp(current->name, path) == 0 && current->typeflag=='0') {
            lseek(tar_fd, 0, SEEK_SET);
            return 1;
        }
        int nb_blocks = TAR_INT(current->size) / 512;
        if (TAR_INT(current->size) % 512 != 0) {
            nb_blocks++;
        }
        lseek(tar_fd, nb_blocks*512, SEEK_CUR);
    }
    lseek(tar_fd, 0, SEEK_SET);
    return 0;
    
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    int end = lseek(tar_fd, 0, SEEK_END);
    lseek(tar_fd, 0, SEEK_SET);
    while(lseek(tar_fd,0,SEEK_CUR) < end) {
        tar_header_t* current = malloc(sizeof(tar_header_t));
        read_header(tar_fd, current);
        if (strcmp(current->name, path) == 0 && current->typeflag=='2') {
            lseek(tar_fd, 0, SEEK_SET);
            return 1;
        }
        int nb_blocks = TAR_INT(current->size) / 512;
        if (TAR_INT(current->size) % 512 != 0) {
            nb_blocks++;
        }
        lseek(tar_fd, nb_blocks*512, SEEK_CUR);
    }
    lseek(tar_fd, 0, SEEK_SET);
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    int end = lseek(tar_fd, 0, SEEK_END);
    lseek(tar_fd, 0, SEEK_SET);
    size_t i = 0;
    int nb_backslash = count_slashes(path);
    // printf("path: %s\n", path);

    while(lseek(tar_fd,0,SEEK_CUR) < end) {
        tar_header_t* current = malloc(sizeof(tar_header_t));
        read_header(tar_fd, current);
        if (current->typeflag == SYMTYPE && strcmp(current->name, path) == 0) {
            if(is_file(tar_fd, current->linkname) == 1) {
                return 0;
            }
            char* newPath = strcat(current->linkname, "/");
            if(is_dir(tar_fd, newPath) == 0) {
                return 0;
            }
            printf("newPath: %s", newPath);
            return list(tar_fd, newPath, entries, no_entries);
        }

        if ((strstr(current->name, path) != NULL) && (strcmp(current->name, path) != 0) && ((count_slashes(current->name) == nb_backslash) || (count_slashes(current->name) == nb_backslash+1 && current->name[strlen(current->name)-1] == 47))) {
            entries[i] = current->name;
            i++;
        }


        int nb_blocks = TAR_INT(current->size) / 512;
        if (TAR_INT(current->size) % 512 != 0) {
            nb_blocks++;
        }
        lseek(tar_fd, nb_blocks*512, SEEK_CUR);
    }
    *no_entries = i;
    lseek(tar_fd, 0, SEEK_SET);

    if(i == 0) {
        return 0;
    }

    return 1;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    if (path[strlen(path)-1] =='/'){
        return -1;
    }
    int end = lseek(tar_fd, 0, SEEK_END);
    lseek(tar_fd, 0, SEEK_SET);

    while(lseek(tar_fd,0,SEEK_CUR) < end) {
        tar_header_t* current = malloc(sizeof(tar_header_t));
        read_header(tar_fd, current);
        if (current->typeflag == SYMTYPE && strcmp(current->name, path) == 0) {
            return read_file(tar_fd, current->linkname,offset,dest,len);
        }

        if (strcmp(current->name, path) == 0) {
            if(offset>TAR_INT(current->size)){
                return -2;
            }
            printf("size: %ld \n", TAR_INT(current->size));
            if(TAR_INT(current->size) - offset <= *len){
                *len = TAR_INT(current->size) - offset;
                lseek(tar_fd,offset,SEEK_CUR);
                read(tar_fd,dest, TAR_INT(current->size) - offset);
                return 0;
            }
            else if(TAR_INT(current->size) - offset > *len){
                lseek(tar_fd,offset,SEEK_CUR);
                read(tar_fd,dest,*len);
                return (ssize_t) TAR_INT(current->size)-offset-*len;
            }
        }

    }
    return -1;

    
}
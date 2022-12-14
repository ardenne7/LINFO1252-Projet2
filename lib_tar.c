#include "lib_tar.h"


unsigned int read_header(int tar_fd, tar_header_t* current) {
    unsigned int checksum_verif = 0;
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

    checksum_verif += (unsigned int) TAR_INT(current->name);
    checksum_verif += (unsigned int) TAR_INT(current->mode);
    checksum_verif += (unsigned int) TAR_INT(current->uid);
    checksum_verif += (unsigned int) TAR_INT(current->gid);
    checksum_verif += (unsigned int) TAR_INT(current->size);
    checksum_verif += (unsigned int) TAR_INT(current->mtime);
    checksum_verif += (unsigned int) TAR_INT(&current->typeflag);
    checksum_verif += (unsigned int) TAR_INT(current->linkname);
    checksum_verif += (unsigned int) TAR_INT(current->magic);
    checksum_verif += (unsigned int) TAR_INT(current->version);
    checksum_verif += (unsigned int) TAR_INT(current->uname);
    checksum_verif += (unsigned int) TAR_INT(current->gname);
    checksum_verif += (unsigned int) TAR_INT(current->devmajor);
    checksum_verif += (unsigned int) TAR_INT(current->devminor);
    checksum_verif += (unsigned int) TAR_INT(current->prefix);
    checksum_verif += (unsigned int) TAR_INT(current->padding);

    return checksum_verif;
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
    tar_header_t* current = malloc(sizeof(tar_header_t));
    unsigned int checksum_verif = read_header(tar_fd, current);

    char first5bits = (*current->magic & 0b11111) >> 1;
    printf("%s \n",(char*) &first5bits);
    printf("%d \n",*current->magic & 0b000001);

    if (strcmp(&first5bits, TMAGIC) == 0 && (*current->magic & 0b000001) == 0x00) {
        return -1;
    }

    if (strcmp(current->version, TVERSION) == 0) {
        return -2;
    }

    if (checksum_verif != (unsigned int) TAR_INT(current->chksum)) {
        return -3;
    }

    printf("Name of file: %s \n", current->name);
    printf("Size of file: %s \n", current->size);

    printf("%d \n", checksum_verif);

    return 0;
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
    return 0;
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
    return 0;
}
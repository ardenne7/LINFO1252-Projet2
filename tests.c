#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);

    int ret2 = exists(fd, "folder1/");
    printf("exist returned %d\n", ret2);

    int ret3 = is_file(fd, "folder1/");
    printf("is_file returned %d\n", ret3);

    int ret4 = is_dir(fd, "folder1/");
    printf("is_dir returned %d\n", ret4);

    int ret5 = is_symlink(fd, "test_ls");
    printf("is_symlink returned %d\n", ret5);

    // char** entries = malloc(10*sizeof(char*));
    // size_t no_entries = 10;

    // int ret6 = list(fd, "folder1/", entries, &no_entries);
    // printf("list returned %d \n", ret6);
    // printf("Size of list returned %ld \n", no_entries);
    // for (int i = 0; i < no_entries; i++) {
    //     printf("list returned %s \n", entries[i]);
    // }

    char** entries2 = malloc(10*sizeof(char*));
    size_t no_entries2 = 10;

    int ret7 = list(fd, "symlink_folder", entries2, &no_entries2);
    printf("list returned %d \n", ret7);
    printf("Size of list returned %ld \n", no_entries2);
    for (int i = 0; i < no_entries2; i++) {
        printf("list returned %s \n", entries2[i]);
    }

    // uint8_t* buff = malloc

    return 0;
}
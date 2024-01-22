#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 1;
    }

    if (strcmp(argv[1], "load") == 0) {
        // Load the kernel module
        if (system("insmod chkr-module.ko") == -1) {
            perror("insmod");
            return 1;
        }

        printf("Kernel module loaded.\n");
    } else if (strcmp(argv[1], "unload") == 0) {
        // Unload the kernel module
        if (system("rmmod chkr-module") == -1) {
            perror("rmmod");
            return 1;
        }

        printf("Kernel module unloaded.\n");
        system("dmesg | grep '!!!Sys call'");  // for intercepted system calls
        system("dmesg | grep '***Sys call'");  // for new system calls
    } else {
        fprintf(stderr, "Usage: %s <load|unload>\n", argv[0]);
        return 1;
    }

    return 0;
}

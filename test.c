#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "barcode.h"

#define BARCODE_MAXLEN  1023

int main(int argc, char *argv[])
{
    barcode_dev    dev;
    unsigned long  ms;
    int            status, exitcode;

    if (argc != 3 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: %s [ -h | --help ]\n", argv[0]);
        fprintf(stderr, "       %s INPUT-EVENT-DEVICE IDLE-TIMEOUT\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, "This program reads barcodes from INPUT-EVENT-DEVICE,\n");
        fprintf(stderr, "waiting at most IDLE-TIMEOUT seconds for a new barcode.\n");
        fprintf(stderr, "The INPUT-EVENT-DEVICE is grabbed, the digits do not appear as\n");
        fprintf(stderr, "inputs in the machine.\n");
        fprintf(stderr, "You can at any time end the program by sending it a\n");
        fprintf(stderr, "SIGINT (Ctrl+C), SIGHUP, or SIGTERM signal.\n");
        fprintf(stderr, "\n");
        return EXIT_FAILURE;
    }

    if (install_done(SIGINT) ||
        install_done(SIGHUP) ||
        install_done(SIGTERM)) {
        fprintf(stderr, "Cannot install signal handlers: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    {
        double value, check;
        char dummy;
        if (sscanf(argv[2], " %lf %c", &value, &dummy) != 1 || value < 0.001) {
            fprintf(stderr, "%s: Invalid idle timeout value (in seconds).\n", argv[2]);
            return EXIT_FAILURE;
        }
        ms = (unsigned long)(value * 1000.0);
        check = (double)ms / 1000.0;

        if (value < check - 0.001 || value > check + 0.001 || ms < 1UL) {
            fprintf(stderr, "%s: Idle timeout is too long.\n", argv[2]);
            return EXIT_FAILURE;
        }
    }

    if (barcode_open(&dev, argv[1])) {
        fprintf(stderr, "%s: Cannot open barcode input event device: %s.\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    while (1) {
        char    code[BARCODE_MAXLEN + 1];
        size_t  len;

        if (done) {
            status = EINTR;
            break;
        }

        len = barcode_read(&dev, code, sizeof code, ms);
        if (errno) {
            status = errno;
            break;
        }
        if (len < (size_t)1) {
            status = ETIMEDOUT;
            break;
        }

        printf("%zu-digit barcode: %s\n", len, code);
        fflush(stdout);
    }

    if (status == EINTR) {
        fprintf(stderr, "Signaled to exit. Complying.\n");
        fflush(stderr);
        exitcode = EXIT_SUCCESS;
    } else
    if (status == ETIMEDOUT) {
        fprintf(stderr, "Timed out, no more barcodes.\n");
        fflush(stderr);
        exitcode = EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Error reading input event device %s: %s.\n", argv[1], strerror(status));
        fflush(stderr);
        exitcode = EXIT_FAILURE;
    }

    if (barcode_close(&dev)) {
        fprintf(stderr, "Warning: Error closing input event device %s: %s.\n", argv[1], strerror(errno));
        fflush(stderr);
        exitcode = EXIT_FAILURE;
    }

    return exitcode;
}

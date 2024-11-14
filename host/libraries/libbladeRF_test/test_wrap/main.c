#include <fcntl.h>
#include <getopt.h>
#include <libbladeRF.h>
#include <stdio.h>
#include <stdlib.h>

#include "conversions.h"

static const struct option long_options[] = {
    { "path", required_argument, NULL, 'p' },
    { "reset", no_argument, NULL, 'r' },
    { "no-reset", no_argument, NULL, 'n' },
    { "help", no_argument, NULL, 'h' },
    { NULL, 0, NULL, 0 },
};

void log_cb(const char* msg, size_t len)
{
    printf("%s", msg);
}

int main(int argc, char *argv[])
{
    int opt     = 0;
    int opt_ind = 0;
    int status  = 0;
    char *path  = NULL;
    struct bladerf *dev;
    bool reset_on_open = true;
    bool ok;
    bladerf_log_level log_level = BLADERF_LOG_LEVEL_INFO;

    opt = 0;

    while (opt != -1) {
        opt = getopt_long(argc, argv, "p:rnv:h", long_options, &opt_ind);

        switch (opt) {
            case 'p':
                path = optarg;
                break;


            case 'r':
                reset_on_open = true;
                break;

            case 'n':
                reset_on_open = false;
                break;

            case 'v':
                log_level = str2loglevel(optarg, &ok);
                if (!ok) {
                    fprintf(stderr, "Invalid log level: %s\n", optarg);
                    return -1;
                }
                break;

            case 'h':
                printf("Usage: %s [options]\n", argv[0]);
                printf("  -p, --path <str>      Specify path to device to "
                       "open.\n");
                printf("  -r, --reset           Enable USB reset on open.\n");
                printf("  -n, --no-reset        Disable USB reset on open.\n");
                printf("  -v, --verbosity <l>   Set libbladeRF verbosity "
                       "level.\n");
                printf("  -h, --help            Show this text.\n");
                printf("\n");
                return 0;

            default:
                break;
        }
    }

    if (path == NULL) {
        printf("ERROR: Path argument is required\n");
        return 1;
    }

    printf("Got device str %s\n", path);

    bladerf_log_set_verbosity(log_level);
    bladerf_set_log_callback(log_cb);

    bladerf_set_usb_reset_on_open(reset_on_open);

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        printf("Failed to open bladerf %s: %s\n", path, strerror(errno));
        return 1;
    }

    status = bladerf_wrap(&dev, (void *)((size_t) fd), BLADERF_BACKEND_LIBUSB);
    if (status != 0) {
        close(fd);
        fprintf(stderr, "Failed to open device: %s\n",
                bladerf_strerror(status));
        return EXIT_FAILURE;
    } else {
        close(fd);
        bladerf_close(dev);
    }

    return EXIT_SUCCESS;
}

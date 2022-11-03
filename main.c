/*
 * IPS format specifications found at:
 * https://zerosoft.zophar.net/ips.php
 */

#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define die(fmt, ...) \
    do { \
        fprintf(stderr, fmt "\n",##__VA_ARGS__); \
        exit(1); \
    } while (0)

#define die_errno(errnum, fmt, ...) \
    do { \
        fprintf(stderr, fmt ": %s\n",##__VA_ARGS__, strerror(errnum)); \
        exit(1); \
    } while (0)

#define PATCH_EOF 0x454f46
#define SIZE_HEADER 5
#define SIZE_OFFSET 3
#define SIZE_SIZE 2

#define USAGE  \
    "Usage: ips [-h] [-c MODIFIED] PATCH ROM\n" \
    "\n" \
    "Patches a game ROM with a specified IPS patch.\n" \
    "\n" \
    "Optional arguments\n" \
    "   -h, --help\n" \
    "       Show usage statement and exit.\n" \
    "   -c --create\n" \
    "       Create a patch with name PATCH which patches ROM to create MODIFIED.\n"

/**
 *  uint_size - convert the two-byte size buffer to a uint.
 */
unsigned int uint_size(uint8_t size[2]){
    return (((unsigned int)size[0] << 8) & 0xFF00) |
           (((unsigned int)size[1]) & 0x00FF);
}

/**
 *  uint_offset - convert the three-byte offset buffer to a uint.
 */
unsigned int uint_offset(uint8_t offset[3]){
    return (((unsigned int)(offset)[0] << 16) & 0x00FF0000) |
           (((unsigned int)(offset)[1] << 8) & 0x0000FF00) |
           ((unsigned int)(offset)[2] & 0x000000FF);
}

/**
 *  ips_read - wrapper function to handle fread error checking. De-clutters main program code.
 */
void ips_read(uint8_t *buffer, size_t read_bytes, FILE *stream){
    size_t bytes_read = 0;
    bytes_read = fread(buffer, sizeof(uint8_t), read_bytes, stream);

    if (bytes_read != read_bytes){
        if (feof(stream))
            die("EOF");

        die_errno(errno, "fread");
    }
}

/**
 *  ips_write - Wrapper for writing a normal data buffer to op file.
 */
void ips_write(uint8_t *buffer, size_t write_bytes, uint32_t offset, FILE *stream){
    size_t bytes_written = 0;

    if (fseek(stream, offset, SEEK_SET) == -1)  // Seek to the correct file position
        die_errno(errno, "fseek");

    bytes_written = fwrite(buffer, sizeof(uint8_t), write_bytes, stream);
    if (bytes_written != write_bytes)
        die_errno(errno, "fwrite");

}

/**
 *  ips_rle - Repeatedly writes rle_byte to op file.
 */
void ips_rle(uint8_t rle_byte, size_t write_bytes, uint32_t offset, FILE *stream){
    size_t bytes_written = 0;

    if (fseek(stream, offset, SEEK_SET) == -1)  // Seek to the correct file position
        die_errno(errno, "fseek");

    while (bytes_written < write_bytes){
        bytes_written += fwrite(&rle_byte, sizeof(uint8_t), 1, stream);
    }

    printf("Bytes written: %zu\n", bytes_written);
}


int main(int argc, char **argv){
    /* Parsing commandline arguments */
    char *patch_filename, *rom_filename;

    int opt;
    const char *short_opts = ":hic:";
    struct option long_opts[] = {
            {"help", no_argument, NULL, 'h'},
            {"create", required_argument, NULL, 'c'},
            {NULL, 0, NULL, 0}
    };

    while (1) {
        opt = getopt_long(argc, argv, short_opts, long_opts, NULL);
        if (opt == -1)
            break;

        switch (opt) {
            case 'h': /* Usage */
                printf(USAGE);
                exit(0);
                break;

            case 'c': /* Create patch */
                printf("Create patch not implemented at this time");
                exit(0);
                break;

            case '?': /* Unknown option */
                die("unknown option '%c'", optopt);
                break;

            case ':': /* Missing required argument */
                die("missing option argument for option %c", optopt);
                break;

            default:
                die("unexpected getopt_long return value: %c\n", (char)opt);
        }
    }

    if (argc - optind != 2)
        die("expected two positional arguments, but found %d", argc - optind);

    patch_filename = argv[optind];
    rom_filename = argv[optind + 1];

    /* Patching the ROM file */
    uint8_t header[5];
    uint8_t *offset = header; // Offset of the file to place patch bytes
    uint8_t *size = &header[3];   // Number of patch bytes to read

    /* Allocates max patch buffer since it isn't too large */
    uint8_t *data = calloc(0xffff, sizeof(uint8_t));    // Use calloc so we don't write random memory to op file
    if (data == NULL)
        die("Out of memory");

    uint8_t rle_byte = 0;
    FILE *patch;
    FILE *rom;       // File we will patch

    patch = fopen(patch_filename, "rb");
    if (patch == NULL)
        die_errno(errno, "fopen");

    rom = fopen(rom_filename, "ab");
    if (rom == NULL)
        die_errno(errno, "fopen");

    /* Verify the patch header. The first 5 bytes of the file must be "PATCH" or 50 41 54 43 48 (hex) */
    memset(header, 0, sizeof(header));
    ips_read(header, SIZE_HEADER, patch);

    // If patch header not found, exit
    if (uint_offset(offset) != 0x504154 || uint_size(size) != 0x4348)
        die("Invalid file header");

    /* Read patch loop */
    ips_read(offset, SIZE_OFFSET, patch);    // Only read 3 bytes to check for EOF

    while (uint_offset(offset) != PATCH_EOF){
        ips_read(size, SIZE_SIZE, patch);  // Read in the rest of the header

        /* Normal patch */
        if (uint_size(size) > 0){
            ips_read(data, uint_size(size), patch);   // Read in data

            ips_write(data, uint_size(size), uint_offset(offset), rom);

        /* RLE patch */
        } else {
            ips_read(size, SIZE_SIZE, patch);  // Read in repeat number, reuse size for readability
            ips_read(&rle_byte, 1, patch);  // Repeat byte

            ips_rle(rle_byte, uint_size(size), uint_offset(offset), rom);
        }
        ips_read(offset, SIZE_OFFSET, patch);    // Like before, read offset to determine EOF
    }

    free(data);
    return 0;
}
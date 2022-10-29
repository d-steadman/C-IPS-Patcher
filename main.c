/*
 * IPS format specifications found at:
 * https://zerosoft.zophar.net/ips.php
 */

#include <stdint.h>
#include <stdio.h>

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

// Hardcoded files for testing
#define PATCH_FILE "test/patch.ips"
#define OP_FILE "test/op.gba"
#define EOF 0x454f46

/**
 *  uint_size - convert the two-byte size buffer to a uint
 */
unsigned int uint_size(uint8_t size[2]){
    return (((unsigned int)size[0] << 8) & 0xFF00) |
           (((unsigned int)size[1]) & 0x00FF);
}

/**
 *  uint_offset - convert the three-byte offset buffer to a uint
 */
unsigned int uint_offset(uint8_t offset[3]){
    return (((unsigned int)(offset)[0] << 16) & 0x00FF0000) |
           (((unsigned int)(offset)[1] << 8) & 0x0000FF00) |
           ((unsigned int)(offset)[2] & 0x000000FF);
}


int main(){
    uint8_t buffer[5];
    uint8_t *offset = buffer; // Offset of the file to place patch bytes
    uint8_t *size = &buffer[3];   // Number of patch bytes to read
    FILE *patch;
    FILE *op;       // File we will patch

    patch = fopen(PATCH_FILE, "rb");
    op = fopen(OP_FILE, "wb");

    /* Verify the patch header. The first 5 bytes of the file must be "PATCH" */
    fread(buffer, sizeof(uint8_t), 5, patch);

    // If patch header not found, exit
    if (uint_offset(offset) != 0x504154 || uint_size(size) != 0x4348)
        die("Invalid file header");

    return 0;
}
#include <stdio.h>

// Hardcoded files for testing
#define PATCH_FILE "test/patch.ips"
#define OP_FILE "test/op.gba"

int main(){
    u_int8_t offset[3]; // Offset of the file to place patch bytes
    u_int8_t size[2];   // Number of patch bytes to read
    FILE *patch;
    FILE *op;       // File we will patch
    u_int8_t buffer[5];

    patch = fopen(PATCH_FILE, "rb");
    op = fopen(OP_FILE, "wb");

    /* Verify the patch header. The first 5 bytes of the file must be "PATCH" */
    fread(buffer, 3, u_int8_t, patch);
    printf("%d\n", (u_int32_t)buffer);

    return 0;
}
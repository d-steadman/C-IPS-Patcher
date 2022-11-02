# C-IPS-Patcher
A reimplemented version of my previous IPS patcher, but now written in C.

# Quick IPS Overview
IPS is a patching format primarily used for small game ROMs, such as those for the Game Boy (Color & Advance), NES, etc.

## Patch Header
Each IPS patch begins with the below bytes, which are ASCII decoded to "PATCH."

`50 41 54 43 48 -> "PATCH"`

## Patch Blocks
A patch file is broken down into blocks which contain fields with offset, size of patch, and data. These blocks can take two forms: standard and RLE.

### Standard Block
Below is an example standard block. The first 3 bytes of the block specify the offset of the file where we should write the patch data. The following 2 bytes specify the length of the patch data. After than is the patch block.

`12 34 56 00 04 DE AD BE EF`

In this example, we are writing to the offset 0x123456. The patch block is 0x0004 bytes long. Reading the next 4 bytes, we can find that the patch block is 0xDEADBEEF.

### RLE Block
The next type of block is the RLE (Run Length Encoding) block. It is very similar to the standard block, but instead of patching a block of text, it instead writes the same character repeatedly a specified number of times. The first 3 bytes of this block are still the offset, but the size of an RLE block will be zero. After the 2 zero bytes, the next 2 bytes will specify how many times to repeat the RLE byte, and the final byte will specify the repeat byte.

`12 34 56 00 00 00 00 04 FF`

In this example, we are writing to the offset 0x123456. We encounter the zero size and know that we have an RLE block. The next 2 bytes specify a repeat count of 4, and the final byte is the repeat byte. So we will end up writing 0xFFFFFFFF.

### EOF
To specify the end of the patch file, in the place where an offset should be, will be bytes that decode to "EOF."

`45 4f 46 -> "EOF"`
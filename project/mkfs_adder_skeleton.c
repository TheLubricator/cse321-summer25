#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>

#define BS 4096u
#define INODE_SIZE 128u
#define ROOT_INO 1u
#define DIRECT_MAX 12
#pragma pack(push, 1)

typedef struct {
    // CREATE YOUR SUPERBLOCK HERE
    // ADD ALL FIELDS AS PROVIDED BY THE SPECIFICATION
    uint32_t magic;
    uint32_t version;   
    uint32_t block_size;
    uint64_t total_blocks;  
    uint64_t inode_count;
    uint64_t inode_bitmap_start;
    uint64_t inode_bitmap_blocks;
    uint64_t data_bitmap_start;
    uint64_t data_bitmap_blocks;
    uint64_t inode_table_start;
    uint64_t inode_table_blocks;
    uint64_t data_region_start;
    uint64_t data_region_blocks;
    uint64_t root_inode ;
    uint64_t mtime_epoch; 
    uint32_t flags;  
    // THIS FIELD SHOULD STAY AT THE END
    // ALL OTHER FIELDS SHOULD BE ABOVE THIS
    uint32_t checksum;            // crc32(superblock[0..4091])
} superblock_t;
#pragma pack(pop)
_Static_assert(sizeof(superblock_t) == 116, "superblock must fit in one block");

#pragma pack(push,1)
typedef struct {
    // CREATE YOUR INODE HERE
    // IF CREATED CORRECTLY, THE STATIC_ASSERT ERROR SHOULD BE GONE
    uint16_t mode;       // file type and permissions
    uint16_t links;
    uint32_t uid;        
    uint32_t gid;
    uint64_t size_bytes;       
    uint64_t atime;      
    uint64_t ctime;      
    uint64_t mtime;          
    uint32_t direct[12];
    uint32_t reserved_0;
    uint32_t reserved_1;
    uint32_t reserved_2;
    uint32_t proj_id;
    uint32_t uid16_gid16;
    uint64_t xattr_ptr;
    // THIS FIELD SHOULD STAY AT THE END
    // ALL OTHER FIELDS SHOULD BE ABOVE THIS
    uint64_t inode_crc;   // low 4 bytes store crc32 of bytes [0..119]; high 4 bytes 0

} inode_t;
#pragma pack(pop)
_Static_assert(sizeof(inode_t)==INODE_SIZE, "inode size mismatch");

#pragma pack(push,1)
typedef struct {
    // CREATE YOUR DIRECTORY ENTRY STRUCTURE HERE
    uint32_t inode_no;
    uint8_t  type;      // 1=file, 2=directory
    char     name[58]; 
    // IF CREATED CORRECTLY, THE STATIC_ASSERT ERROR SHOULD BE GONE

    uint8_t  checksum; // XOR of bytes 0..62
} dirent64_t;
#pragma pack(pop)
_Static_assert(sizeof(dirent64_t)==64, "dirent size mismatch");


// ==========================DO NOT CHANGE THIS PORTION=========================
// These functions are there for your help. You should refer to the specifications to see how you can use them.
// ====================================CRC32====================================
uint32_t CRC32_TAB[256];
void crc32_init(void){
    for (uint32_t i=0;i<256;i++){
        uint32_t c=i;
        for(int j=0;j<8;j++) c = (c&1)?(0xEDB88320u^(c>>1)):(c>>1);
        CRC32_TAB[i]=c;
    }
}
uint32_t crc32(const void* data, size_t n){
    const uint8_t* p=(const uint8_t*)data; uint32_t c=0xFFFFFFFFu;
    for(size_t i=0;i<n;i++) c = CRC32_TAB[(c^p[i])&0xFF] ^ (c>>8);
    return c ^ 0xFFFFFFFFu;
}
// ====================================CRC32====================================

// WARNING: CALL THIS ONLY AFTER ALL OTHER SUPERBLOCK ELEMENTS HAVE BEEN FINALIZED
static uint32_t superblock_crc_finalize(superblock_t *sb) {
    sb->checksum = 0;
    uint32_t s = crc32((void *) sb, BS - 4);
    sb->checksum = s;
    return s;
}

// WARNING: CALL THIS ONLY AFTER ALL OTHER SUPERBLOCK ELEMENTS HAVE BEEN FINALIZED
void inode_crc_finalize(inode_t* ino){
    uint8_t tmp[INODE_SIZE]; memcpy(tmp, ino, INODE_SIZE);
    // zero crc area before computing
    memset(&tmp[120], 0, 8);
    uint32_t c = crc32(tmp, 120);
    ino->inode_crc = (uint64_t)c; // low 4 bytes carry the crc
}

// WARNING: CALL THIS ONLY AFTER ALL OTHER SUPERBLOCK ELEMENTS HAVE BEEN FINALIZED
void dirent_checksum_finalize(dirent64_t* de) {
    const uint8_t* p = (const uint8_t*)de;
    uint8_t x = 0;
    for (int i = 0; i < 63; i++) x ^= p[i];   // covers ino(4) + type(1) + name(58)
    de->checksum = x;
}


int CLI_validate(int argc, char *argv[]) {
    if (argc != 7) {
        printf("invalid CLI format, correct: %s --input <input image> --output <output image> --file <filename txt>\n");
        return 5;
    }
    else if (strstr(argv[4],".img")==NULL) {
        printf("invalid output image format. Must be in .img format.\n");
        return 5;
    }
    else if (strcmp(argv[1],"--input")!=0 || strcmp(argv[3],"--output")!=0 || strcmp(argv[5],"--file")!=0) {
        printf("invalid CLI format, correct: %s --input <input image> --output <output image> --file <filename txt>\n");
        return 5;
    }
    else if (access(argv[2], F_OK) != 0) {
        printf("input image file does not exist: %s\n", argv[2]);
        return 6;
    }
    else if (access(argv[6], F_OK) != 0) {
        printf("file to add does not exist: %s\n", argv[6]);
        return 7;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    crc32_init();
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s %d\n", i, argv[i],argc);
    }
    int ret = CLI_validate(argc, argv);
    if (ret != 0) {
        return ret;
    }
    FILE *file_size = fopen(argv[2], "r");
    if (!file_size) {
        fprintf(stderr, "Error: Cannot open input image for reading\n");
        return 8;
    }
    fseeko(file_size, 0, SEEK_END);
    size_t image_size = ftell(file_size);
    fclose(file_size);
    printf("Input image size: %zu bytes\n", image_size);
    uint8_t *image_buffer = malloc(image_size); // Allocate memory for the entire image
    if (!image_buffer) {
        fprintf(stderr, "Error: Failed to allocate memory for image of size %zu bytes\n", image_size);
        return 9;
    }
    memset(image_buffer, 0, image_size); // Zero-initialize the buffer
    FILE *read_input_img = fopen(argv[2], "rb");
    if (!read_input_img) {
        fprintf(stderr, "Error: Cannot open input image for reading\n");
        free(image_buffer);
        return 10;
    }
    size_t bytes_read = fread(image_buffer, 1, image_size, read_input_img);
    if (bytes_read != image_size) {
        fprintf(stderr, "Error: Read %zu bytes, expected %zu\n", bytes_read, image_size);
        fclose(read_input_img);
        free(image_buffer);
        return 11;
    }
    fclose(read_input_img);


    superblock_t *read_sb = (superblock_t*)image_buffer ;
    uint8_t *read_inode_bitmap = image_buffer  + read_sb->inode_bitmap_start * BS;
    uint8_t *read_data_bitmap = image_buffer  + read_sb->data_bitmap_start * BS;
    inode_t *read_inode = (inode_t*)(image_buffer + read_sb->inode_table_start * BS);
    dirent64_t *read_dir = (dirent64_t*)(image_buffer  + read_sb->data_region_start * BS);

printf("=== Superblock Verification ===\n");
if (read_sb->magic != 0x4D565346) {
    printf("Error: Invalid magic number: 0x%X\n", read_sb->magic);
    free(image_buffer);
    return 12;
}
if (read_sb->version != 1) {
    printf("Error: Unsupported version: %u\n", read_sb->version);
    free(image_buffer);
    return 13;
}
if (read_sb->block_size != BS) {
    printf("Error: Unexpected block size: %u\n", read_sb->block_size);
    free(image_buffer);
    return 14;
}
printf("Magic: 0x%X (expected: 0x4D565346)\n", read_sb->magic);
printf("Version: %u (expected: 1)\n", read_sb->version);
printf("Block Size: %u (expected: %u)\n", read_sb->block_size, BS);
printf("Total Blocks: %" PRIu64 "\n", read_sb->total_blocks);
printf("Inode Count: %" PRIu64 "\n", read_sb->inode_count);
superblock_crc_finalize(read_sb);
printf("Checksum: 0x%X\n", read_sb->checksum);

// Verify bitmaps

printf("=== Bitmap Verification ===\n");
printf("Inode bitmap first byte: 0b%08b\n", read_inode_bitmap[0]);
printf("Data bitmap first byte: 0b%08b\n", read_data_bitmap[0]);

// // Verify root inode

// printf("=== Root Inode Verification ===\n");
// printf("Mode: 0%o\n", read_inode[0].mode);
// printf("Links: %u\n", read_inode[0].links);
// printf("Size: %" PRIu64 " bytes\n", read_inode[0].size_bytes);
// printf("Direct Block[0]: %u\n", read_inode[0].direct[0]);
// inode_crc_finalize(&read_inode[0]);

// printf("CRC: 0x%" PRIx64 "\n", read_inode[0].inode_crc);

// // Verify directory entries

// printf("=== Directory Entries Verification ===\n");
// printf("Entry 0: inode=%u, type=%u, name='%s', checksum=0x%X\n",
//        read_dir[0].inode_no, read_dir[0].type, read_dir[0].name, read_dir[0].checksum);
// printf("Entry 1: inode=%u, type=%u, name='%s', checksum=0x%X\n",
//        read_dir[1].inode_no, read_dir[1].type, read_dir[1].name, read_dir[1].checksum);

uint8_t first_free_inode, first_free_data, i, found_inode=0, found_data=0;
size_t bitmap_bytes = read_sb->inode_bitmap_blocks * BS;
for (i = 0; i < bitmap_bytes&&!found_inode ; i++) {
    if (read_inode_bitmap[0]!=0b11111111){
        if (i==0){ // to skip root inode
            for (int bit=1; bit<8;bit++){
                if ((read_inode_bitmap[i] & (1 << bit))==0){
                    first_free_inode= bit+1;//say 1 indexed
                    if (first_free_inode <= read_sb->inode_count) {
                    found_inode = 1;
                    read_inode_bitmap[i] |= (1 << bit); // Mark as used
                     printf("First free inode found at position: %d index=0\n", first_free_inode);
                    break;
                   
                   
                   
                }
            }

        }
    }
        else{ //rest are assumed empty
            for (int bit=0; bit<8;bit++){
                if ((read_inode_bitmap[i] & (1 << bit))==0){
                    first_free_inode= i*8 + bit+1;//say 1 indexed
                    if (first_free_inode <= read_sb->inode_count) {
                    found_inode = 1;
                    read_inode_bitmap[i] |= (1 << bit); // Mark as used
                     printf("First free inode found at position: %d index=0\n", first_free_inode);
                    break;
                    
                }
            }
        }
    }

    }
}    
printf("current inode bit map 0 valeue %d\n",read_inode_bitmap[0]);
FILE *txt_file=
    // WRITE YOUR DRIVER CODE HERE
    // PARSE YOUR CLI PARAMETERS
    // THEN ADD THE SPECIFIED FILE TO YOUR FILE SYSTEM
    // UPDATE THE .IMG FILE ON DISK
    return 0;
}

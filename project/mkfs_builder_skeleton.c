// Build: gcc -O2 -std=c17 -Wall -Wextra mkfs_minivsfs.c -o mkfs_builder
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
#include <fcntl.h>

#define BS 4096u               // block size
#define INODE_SIZE 128u
#define ROOT_INO 1u

uint64_t g_random_seed = srand(time(NULL)); // This should be replaced by seed value from the CLI.

// below contains some basic structures you need for your project
// you are free to create more structures as you require

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
    // IF CREATED CORRECTLY, THE STATIC_ASSERT ERROR SHOULD BE GONE
    uint32_t inode_no;
    uint8_t  type;      // 1=file, 2=directory
    char     name[58];  
    // THIS FIELD SHOULD STAY AT THE END
    // ALL OTHER FIELDS SHOULD BE ABOVE THIS

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
int convert_to_int(char *str, int *result) {
    char *endptr;
    errno = 0;
    
    long val = strtol(str, &endptr, 10);
    
    // Check for various error conditions
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        return 0; // Out of range
    }
    
    if (endptr == str || *endptr != '\0') {
        return 0; // No digits found or extra characters
    }
    
    *result = (int)val;
    return 1; // Success
}

int CLI_validate(int argc, char *argv[]) {
    if (argc != 7) {
        printf("invalid format, correct: %s --image <output image> --inode <inode size> --size--kib <block size>\n", argv[0]);
        return 5;
    }
    else if (strstr(argv[2],".img")==NULL) {
        printf("invalid output file name. Must be named out.img.\n");
        return 5;
    }
    else if (strcmp(argv[1],"--image")!=0 || strcmp(argv[3],"--inode")!=0 || strcmp(argv[5],"--size-kib")!=0) {
        printf("invalid format, correct: %s --image <output image> --inode <inode size> --size-kib <block size>\n", argv[0]);
        return 5;
    }
    return 0;
}
int Check_Valid_Values(int inode_size, int block_size) {
    if  (inode_size<128 || inode_size>512) {
        printf("Invalid inode count: %d\n", inode_size);
        return 1;
    }
    if  (block_size<180 || block_size>4096 ||block_size%4!=0) {
        printf("Invalid total size: %d\n", block_size);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    crc32_init();
    // for (int i = 0; i < argc; i++) {
    //     printf("argv[%d] = %s %d\n", i, argv[i],argc);
    // }
    
    int ret = CLI_validate(argc, argv);
    if (ret != 0) {
        return ret;
    }

    int input_inode_size,input_total_size;
    if (!convert_to_int(argv[4], &input_inode_size)) {
        printf("Invalid input as inode count: %s, must be a number\n", argv[4]);
        return 2;
    }
    if (!convert_to_int(argv[6], &input_total_size)) {
        printf("Invalid input as total size %s, must be a number\n", argv[6]);
        return 3;
    }
    ret = Check_Valid_Values(input_inode_size, input_total_size);
    if (ret != 0) {
        return ret;
    }
    printf("Inode count: %d, total img size: %d\n", input_inode_size, input_total_size);
   superblock_t superblock;
dirent64_t dirent;

/* proper configuration of the structures */
uint64_t total_blocks = (input_total_size * 1024) / BS; 
size_t image_size = input_total_size * 1024; // img size  in byetts
uint8_t *image_buffer = malloc(image_size); // Allocate memory for the entire image
if (!image_buffer) {
    fprintf(stderr, "Error: Failed to allocate memory for image of size %zu bytes\n", image_size);
    exit(EXIT_FAILURE);
}
memset(image_buffer, 0, image_size); // Zero-initialize the buffer
// ... (subsequent steps will populate image_buffer)

// Don't forget to free(image_buffer) after writing to file
superblock.magic=0x4D565346;
superblock.version=1;
superblock.block_size=BS;
superblock.total_blocks=total_blocks;
superblock.inode_count=input_inode_size;

superblock.inode_bitmap_start=1;
superblock.inode_bitmap_blocks=1;
superblock.data_bitmap_start=2;
superblock.data_bitmap_blocks=1;
superblock.inode_table_start=3;
superblock.inode_table_blocks=(input_inode_size*INODE_SIZE)/BS;
superblock.data_region_start=superblock.inode_table_start+superblock.inode_table_blocks;
superblock.data_region_blocks=total_blocks-superblock.data_region_start;
superblock.root_inode=ROOT_INO;
superblock.mtime_epoch=(uint64_t)time(NULL);
superblock.flags=0;
superblock.checksum=0;
superblock_crc_finalize(&superblock);
memcpy(image_buffer+0*BS, &superblock, sizeof(superblock_t));
//test during coding to find errors
// superblock_t *retrieved1 = (superblock_t*)((char*)image_buffer + 0);
// int j=retrieved1->block_size;
// printf("Superblock Magic: 0x%X\n", retrieved1->magic);
// printf("Superblock Version: %u\n", retrieved1->version);
// printf("Superblock Block Size: %d\n", j);
// printf("Superblock Total Blocks: %" PRIu64 "\n", retrieved1->total_blocks);
// printf("Superblock Inode Count: %" PRIu64 "\n", retrieved1->inode_count);
// printf("Superblock Checksum: 0x%X\n", retrieved1->checksum);
//Allocate inode bitmap  block 1
uint8_t inode_bitmap[BS];
memset(inode_bitmap, 0, BS);
inode_bitmap[0] = 0b00000001; 
memcpy(image_buffer + superblock.inode_bitmap_start * BS, inode_bitmap, BS);
//Allocate data bitmap block 2
uint8_t data_bitmap[BS];
memset(data_bitmap, 0, BS);
data_bitmap[0] = 0b00000001; 
memcpy(image_buffer + superblock.data_bitmap_start * BS, data_bitmap, BS);

//root  indoe time
// inode.mode = 0x4000 | 0x1FF; // directory with 0777 permissions



// uint8_t *inode_bitmap_retrieve = (uint8_t*)((char*)image_buffer + superblock.inode_bitmap_start * BS);
// uint8_t *data_bitmap_array_test = (uint8_t*)((char*)image_buffer+superblock.data_bitmap_start * BS);
// printf("Inode bitmap first byte: 0b%08b\n", inode_bitmap_retrieve[0]);
// printf("Data bitmap first byte: 0b%08b\n", data_bitmap_array_test[0]);

inode_t inode_table[superblock.inode_count];
memset(inode_table, 0, superblock.inode_count*INODE_SIZE);
inode_table[0].mode = (0040000); 
inode_table[0].links = 2;
inode_table[0].uid = rand();
inode_table[0].gid = rand();
inode_table[0].size_bytes = 128;
inode_table[0].atime = (uint64_t)time(NULL);
inode_table[0].ctime = (uint64_t)time(NULL);
inode_table[0].mtime = (uint64_t)time(NULL);
inode_table[0].direct[0] = superblock.data_region_start;
inode_table[0].proj_id = 2;
inode_table[0].uid16_gid16 = 0;
inode_table[0].xattr_ptr = 0;
inode_table[0].reserved_0 = 0;
inode_table[0].reserved_1 = 0;
inode_table[0].reserved_2 = 0;
inode_crc_finalize(&inode_table[0]);
memcpy(image_buffer + superblock.inode_table_start * BS, inode_table, superblock.inode_count * INODE_SIZE);


// inode_t *retrieved_inode = (inode_t*)((char*)image_buffer + superblock.inode_table_start * BS);
// printf("Root Inode Mode: 0%o\n", retrieved_inode[0].mode);
// printf("Root Inode Size: %" PRIu64 " bytes\n", retrieved_inode[0].size_bytes);
// printf("Root Inode Direct Block[0]: %u\n", retrieved_inode[0].direct[0]);
// printf("Root Inode CRC: 0x%" PRIx64 "\n", retrieved_inode[0].inode_crc);



// Create root directory entry
dirent64_t dir_entries[2]; // "." and ".."
memset(dir_entries, 0, sizeof(dir_entries));

//"."
dir_entries[0].inode_no = ROOT_INO;
dir_entries[0].type = 2; 
strncpy(dir_entries[0].name, ".", sizeof(dir_entries[0].name) - 1);
dirent_checksum_finalize(&dir_entries[0]);
//".."
dir_entries[1].inode_no = ROOT_INO; 
dir_entries[1].type = 2; 
strncpy(dir_entries[1].name, "..", sizeof(dir_entries[1].name) - 1);
dirent_checksum_finalize(&dir_entries[1]);
memcpy(image_buffer + superblock.data_region_start * BS, dir_entries, sizeof(dir_entries));

dirent64_t *retrieved_dir_entries = (dirent64_t*)((char*)image_buffer + superblock.data_region_start * BS);
printf("Directory Entry 0: inode=%u, type=%u, name=%s, checksum=0x%X\n",
       retrieved_dir_entries[0].inode_no, retrieved_dir_entries[0].type,
       retrieved_dir_entries[0].name, retrieved_dir_entries[0].checksum);
printf("Directory Entry 1: inode=%u, type=%u, name=%s, checksum=0x%X\n",
       retrieved_dir_entries[1].inode_no, retrieved_dir_entries[1].type,
       retrieved_dir_entries[1].name, retrieved_dir_entries[1].checksum);
    // WRITE YOUR DRIVER CODE HERE
    // PARSE YOUR CLI PARAMETERS
if (access(argv[2], F_OK) == 0) {
    printf("Error: out.img already exists\n");
    free(image_buffer);
    exit(1);
}

// FILE *file = fopen(argv[2], "wb");
int fd = open(argv[2], O_CREAT | O_EXCL | O_WRONLY, 0644);
// if (file == NULL) {
//     printf("Error: Cannot create out.img\n");
//     free(image_buffer);
//     exit(1);
// }

// size_t bytes_written = fwrite(image_buffer, 1, total_blocks * BS, file);

write(fd, image_buffer, image_size);
// printf("File system image 'out.img' created successfully with size %d kilobytes\n", input_total_size);
// if (fclose(file) != 0) {
//     printf("Error: Failed to close out.img\n");
//     free(image_buffer);
//     exit(1);
// }

close(fd);

// =========================== DIAGNOSTICS: READ BACK AND VERIFY ===========================
printf("\n=== RUNNING DIAGNOSTICS: Reading back from file ===\n");

// Allocate buffer for reading back
uint8_t *read_buffer = malloc(image_size);
if (!read_buffer) {
    fprintf(stderr, "Error: Failed to allocate read buffer\n");
    free(image_buffer);
    exit(1);
}

// Read the file back
FILE *read_file = fopen(argv[2], "rb");
if (!read_file) {
    fprintf(stderr, "Error: Cannot open out.img for reading\n");
    free(image_buffer);
    free(read_buffer);
    exit(1);
}

size_t bytes_read = fread(read_buffer, 1, image_size, read_file);
if (bytes_read != image_size) {
    fprintf(stderr, "Error: Read %zu bytes, expected %zu\n", bytes_read, image_size);
    fclose(read_file);
    free(image_buffer);
    free(read_buffer);
    exit(1);
}
fclose(read_file);

// Verify superblock from file
superblock_t *read_sb = (superblock_t*)read_buffer;
printf("=== Superblock Verification ===\n");
printf("Magic: 0x%X (expected: 0x4D565346)\n", read_sb->magic);
printf("Version: %u (expected: 1)\n", read_sb->version);
printf("Block Size: %u (expected: %u)\n", read_sb->block_size, BS);
printf("Total Blocks: %" PRIu64 "\n", read_sb->total_blocks);
printf("Inode Count: %" PRIu64 "\n", read_sb->inode_count);
printf("Checksum: 0x%X\n", read_sb->checksum);

// Verify bitmaps
uint8_t *read_inode_bitmap = read_buffer + read_sb->inode_bitmap_start * BS;
uint8_t *read_data_bitmap = read_buffer + read_sb->data_bitmap_start * BS;
printf("=== Bitmap Verification ===\n");
printf("Inode bitmap first byte: 0b%08b\n", read_inode_bitmap[0]);
printf("Data bitmap first byte: 0b%08b\n", read_data_bitmap[0]);

// Verify root inode
inode_t *read_inode = (inode_t*)(read_buffer + read_sb->inode_table_start * BS);
printf("=== Root Inode Verification ===\n");
printf("Mode: 0%o\n", read_inode[0].mode);
printf("Links: %u\n", read_inode[0].links);
printf("Size: %" PRIu64 " bytes\n", read_inode[0].size_bytes);
printf("Direct Block[0]: %u\n", read_inode[0].direct[0]);
printf("CRC: 0x%" PRIx64 "\n", read_inode[0].inode_crc);

// Verify directory entries
dirent64_t *read_dir = (dirent64_t*)(read_buffer + read_sb->data_region_start * BS);
printf("=== Directory Entries Verification ===\n");
printf("Entry 0: inode=%u, type=%u, name='%s', checksum=0x%X\n",
       read_dir[0].inode_no, read_dir[0].type, read_dir[0].name, read_dir[0].checksum);
printf("Entry 1: inode=%u, type=%u, name='%s', checksum=0x%X\n",
       read_dir[1].inode_no, read_dir[1].type, read_dir[1].name, read_dir[1].checksum);

// Compare original vs read data
printf("=== Memory vs File Comparison ===\n");
if (memcmp(image_buffer, read_buffer, image_size) == 0) {
    printf("✅ SUCCESS: File data matches original memory buffer!\n");
} else {
    printf("❌ ERROR: File data differs from original memory buffer!\n");
    // Find first difference
    for (size_t i = 0; i < image_size; i++) {
        if (image_buffer[i] != read_buffer[i]) {
            printf("First difference at byte %zu: memory=0x%02X, file=0x%02X\n", 
                   i, image_buffer[i], read_buffer[i]);
            break;
        }
    }
}

free(read_buffer);
printf("=== DIAGNOSTICS COMPLETE ===\n\n");


free(image_buffer);
    // THEN CREATE YOUR FILE SYSTEM WITH A ROOT DIRECTORY
    // THEN SAVE THE DATA INSIDE THE OUTPUT IMAGE
    return 0;
}

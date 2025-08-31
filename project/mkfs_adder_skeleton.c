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
#include<math.h>

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
    
    else if (strcmp(argv[2], argv[4]) == 0) {
        printf("Error: Input and output image files must be different.\n");
        return 8;   

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
    FILE *file_size = fopen(argv[2], "r");
    if (!file_size) {
        fprintf(stderr, "Error: Cannot open input image for reading\n");
        return 8;
    }
    fseek(file_size, 0, SEEK_END);
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


    superblock_t *read_superblock = (superblock_t*)image_buffer ;
    uint8_t *read_inode_bitmap = image_buffer  + read_superblock->inode_bitmap_start * BS;
    uint8_t *read_data_bitmap = image_buffer  + read_superblock->data_bitmap_start * BS;
    inode_t *read_inode_table = (inode_t*)(image_buffer + read_superblock->inode_table_start * BS);
    dirent64_t *read_directory_entries = (dirent64_t*)(image_buffer  + read_superblock->data_region_start * BS); //root  starts from here ie 0  idx and on wards
    

printf("=== Superblock Verification ===\n");
if (read_superblock->magic != 0x4D565346) {
    printf("Error: Invalid magic number: 0x%X\n", read_superblock->magic);
    free(image_buffer);
    return 12;
}
if (read_superblock->version != 1) {
    printf("Error: Unsupported version: %u\n", read_superblock->version);
    free(image_buffer);
    return 13;
}
if (read_superblock->block_size != BS) {
    printf("Error: Unexpected block size: %u\n", read_superblock->block_size);
    free(image_buffer);
    return 14;
}
// printf("Magic: 0x%X (expected: 0x4D565346)\n", read_superblock->magic);
// printf("Version: %u (expected: 1)\n", read_superblock->version);
// printf("Block Size: %u (expected: %u)\n", read_superblock->block_size, BS);
// printf("Total Blocks: %" PRIu64 "\n", read_superblock->total_blocks);
// printf("Inode Count: %" PRIu64 "\n", read_superblock->inode_count);
// superblock_crc_finalize(read_superblock);
// printf("Checksum: 0x%X\n", read_superblock->checksum);

// Verify bitmaps

// printf("=== Bitmap Verification ===\n");
// printf("Inode bitmap first byte: 0b%08b\n", read_inode_bitmap[0]);
// printf("Data bitmap first byte: 0b%08b\n", read_data_bitmap[0]);

// // Verify root inode

// printf("=== Root Inode Verification ===\n");
// printf("Mode: 0%o\n", read_inode_table[0].mode);
// printf("Links: %u\n", read_inode_table[0].links);
// printf("Size: %" PRIu64 " bytes\n", read_inode_table[0].size_bytes);
// printf("Direct Block[0]: %u\n", read_inode_table[0].direct[0]);
// inode_crc_finalize(&read_inode_table[0]);

// printf("CRC: 0x%" PRIx64 "\n", read_inode_table[0].inode_crc);

// // Verify directory entries

// printf("=== Directory Entries Verification ===\n");
// printf("Entry 0: inode=%u, type=%u, name='%s', checksum=0x%X\n",
//        read_dir[0].inode_no, read_dir[0].type, read_dir[0].name, read_dir[0].checksum);
// printf("Entry 1: inode=%u, type=%u, name='%s', checksum=0x%X\n",
//        read_dir[1].inode_no, read_dir[1].type, read_dir[1].name, read_dir[1].checksum);

uint8_t first_free_inode, first_free_data, i, found_inode=0, found_data=0;
size_t bitmap_bytes = read_superblock->inode_bitmap_blocks * BS;
for (i = 0; i < bitmap_bytes&&!found_inode ; i++) {
    if (read_inode_bitmap[0]!=0b11111111){
        if (i==0){ // to skip root inode
            for (int bit=1; bit<8;bit++){
                if ((read_inode_bitmap[i] & (1 << bit))==0){
                    first_free_inode= bit+1;//say 1 indexed
                    if (first_free_inode <= read_superblock->inode_count) {
                    found_inode = 1;
                    read_inode_bitmap[i] |= (1 << bit); // Mark as used
                     printf("First free inode found at position: %d index=0\n", first_free_inode);
                    break;
                   
                   
                   
                }
            }

        }
    } //skipping root's   fixed  pos, it iterates from 1 to 7
        else{ //rest are assumed empty
            for (int bit=0; bit<8;bit++){
                if ((read_inode_bitmap[i] & (1 << bit))==0){
                    first_free_inode= i*8 + bit+1;//say 1 indexed, since next  bitt  in bitmap is array type, so  i*8+0+1 indicates 8th position and this always returns  correct value
                    if (first_free_inode <= read_superblock->inode_count) {
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
FILE *txt_file=fopen(argv[6],"r");
fseek(txt_file,0,SEEK_END);
size_t txt_size=ftell(txt_file);
fclose(txt_file);
printf("File size is %ld bytes\n",txt_size);
// Integer ceiling division: (a + b - 1) / b
int required_blocks = (txt_size + BS - 1) / BS;
printf("Required blocks: %d\n", required_blocks);
if (required_blocks > DIRECT_MAX) {
    printf("Error: File too large to add, requires more than %d direct blocks\n", DIRECT_MAX);
    free(image_buffer);
    
    return 15;
}
uint32_t data_block_indices[DIRECT_MAX] = {0};
size_t data_bitmap_bytes = read_superblock->data_bitmap_blocks * BS;
for (i = 0; i < data_bitmap_bytes && !found_data; i++) {
    if (read_data_bitmap[i] != 0b11111111) { 
        if  (i == 0) {// to skip root data block
            for (int bit = 1; bit < 8; bit++) {
                if ((read_data_bitmap[i] & (1 << bit)) == 0) {
                    data_block_indices[found_data] = i * 8 + bit; // 0-indexed
                    found_data++;
                    read_data_bitmap[i] |= (1 << bit); // Mark as used
                    printf("Data block allocated at index: %d\n", data_block_indices[found_data - 1]);
                    if (found_data == required_blocks) {
                        break;
                    }
                }
            }
        } 
        else { 
            for (int bit = 0; bit < 8; bit++) {
                if ((read_data_bitmap[i] & (1 << bit)) == 0) {
                    data_block_indices[found_data] = i * 8 + bit; // 0-indexed
                    found_data++;
                    read_data_bitmap[i] |= (1 << bit); // Mark as used
                    printf("Data block allocated at index: %d\n", data_block_indices[found_data - 1]);
                    if (found_data == required_blocks) {
                        break;
                    }
                }
            }
        }
    }

}
if (found_data < required_blocks) {
    printf("Error: Not enough free data blocks available\n");
    free(image_buffer); 
    return 16;
}
inode_t *new_file_inode = &read_inode_table[first_free_inode - 1]; //1 indexing so minus 1
new_file_inode->mode = (0100000); // file mode
new_file_inode->links = 1;
new_file_inode->uid = 0;
new_file_inode->gid = 0;
new_file_inode->size_bytes = txt_size;
new_file_inode->atime = (uint64_t)time(NULL);
new_file_inode->ctime = (uint64_t)time(NULL);
new_file_inode->mtime = (uint64_t)time(NULL);
for (i = 0; i < required_blocks; i++) {
    new_file_inode->direct[i] = data_block_indices[i];
}
new_file_inode->reserved_0 = 0;
new_file_inode->reserved_1 = 0;
new_file_inode->reserved_2 = 0;
new_file_inode->proj_id = 0;
new_file_inode->uid16_gid16 = 0;
new_file_inode->xattr_ptr = 0;
inode_crc_finalize(new_file_inode);
printf("New file inode created at index %d (1 index) havving size %ld bytes\n", first_free_inode, new_file_inode->size_bytes);
FILE *txt_file_write = fopen(argv[6], "rb");
uint8_t block_buffer[BS];
for (i = 0; i < required_blocks; i++) {
    memset(block_buffer, 0, BS); // make suring the buffer  is empty
    //every block usually  will hold full value, but  as we  knnow according to
    //math , if  is over even by a bit we  have to ceil it, so last block wwill not hold full value,  so every other blocck equal to block  size last  block
    // will hold remaining bytes
    size_t read_size_per_block;
    
    if (i == required_blocks - 1 && (txt_size % BS) != 0) { 
        //also added one check if by chance last block is also needed fully
        read_size_per_block = txt_size % BS;
    }
    else{
        read_size_per_block = BS;
    }
    printf("Reading %zu bytes for block %d\n", read_size_per_block, i);
    // nnow reading  file block by block to to see if its same  as  read_perblock else we cannot write
    size_t read_bytes_from_file = fread(block_buffer, 1, read_size_per_block, txt_file_write);
    if (read_bytes_from_file != read_size_per_block) {
        printf("Error: Read %zu bytes from text file, expected %zu\n", read_bytes_from_file, read_size_per_block);
        fclose(txt_file_write);
        free(image_buffer);
        return 17;
    }

    uint8_t *data_block_position = image_buffer + (read_superblock->data_region_start + data_block_indices[i]) * BS; //exact position to place dblock, sum of buffer start of dataregion a nd finally dblock index*block size
    memcpy(data_block_position, block_buffer, BS);
    printf("Written %zu bytes to data block index %d\n", read_size_per_block, data_block_indices[i]);
}
fclose(txt_file_write); // done writing
// dirrectory entry, with dupoplicate name check

printf("Filename length: %zu\n", strlen(argv[6]));
if (strlen(argv[6]) > 58) {
    printf("Error: Filename too long, max 58 characters\n");
    free(image_buffer);
    return 18;
}
//dup  check
for (i = 0; i < BS / sizeof(dirent64_t); i++) {
    if(read_directory_entries[i].inode_no !=0  && (strcmp(read_directory_entries[i].name, argv[6])) == 0) {
        printf("Error: Duplicate filename found in root directory: %s\n", argv[6]);
        free(image_buffer);
        return 19;
    }
}
int first_free_directory_index = -1; //if unchanged then  non space
for (i = 0; i < BS / sizeof(dirent64_t); i++) {
    if (read_directory_entries[i].inode_no == 0) { // free entry found
        first_free_directory_index = i;
        break;
    }
}
if (first_free_directory_index == -1) {
    printf("Error: No free directory entries available in root directory\n");
    free(image_buffer);
    return 20;
}
//finally new entrey
dirent64_t *new_file_entry = &read_directory_entries[first_free_directory_index];
new_file_entry->inode_no = first_free_inode;
new_file_entry->type = 1; // file so
strncpy(new_file_entry->name, argv[6], 58); //name[58]
new_file_entry->name[57] = '\0'; //null tterminate
dirent_checksum_finalize(new_file_entry);
//root  directory inode update
read_inode_table[0].links += 1; // lniks mean count so count up
read_inode_table[0].mtime = (uint64_t)time(NULL); // cchange modificcation t ime
read_inode_table[0].size_bytes += sizeof(dirent64_t); //sizoe up by direcotry entry size
inode_crc_finalize(&read_inode_table[0]);

//superblock  modification timem and crc
read_superblock->mtime_epoch = (uint64_t)time(NULL);
superblock_crc_finalize(read_superblock);


// write to  output image
FILE *write_output_img = fopen(argv[4], "wb");
if (!write_output_img) {
    printf("Error: Cannot open output image for writing\n");
    free(image_buffer);
    return 21;
}
size_t bytes_tobe_written = fwrite(image_buffer, 1, image_size, write_output_img);
if (bytes_tobe_written != image_size) {
    printf("Error: Wrote %zu bytes to output image, expected %zu\n", bytes_tobe_written, image_size);
    fclose(write_output_img);
    free(image_buffer);
    return 22;
}
fclose(write_output_img);
free(image_buffer);
printf("File '%s' added successfully to image '%s'\n", argv[6], argv[4]);
    // WRITE YOUR DRIVER CODE HERE
    // PARSE YOUR CLI PARAMETERS
    // THEN ADD THE SPECIFIED FILE TO YOUR FILE SYSTEM
    // UPDATE THE .IMG FILE ON DISK
    return 0;
}


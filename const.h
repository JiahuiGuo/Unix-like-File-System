#ifndef FS_const_h
#define FS_const_h

#define FS_IMG_NAME "fs.img"

// Const size
#define FS_SIZE   1024*1024*100	/* 100MB */
#define NR_INODES 1024          /* 1KB */
#define NAME_LEN_MAX 128        /* Max name length for a file or directory */
#define BLOCK_SIZE  2048        /* 2KB */
#define BUF_SIZE 2048
#define PARA_SIZE  128
#define ROOT_INODE 0            /* inode number for root directory */
#define NDIR_BLOCKS 12          /* number of direct block */
#define IND_BLOCK   1           /* number of indirected block */
#define DIND_BLOCK  1           /* number of double indirected block */
#define N_BLOCK     16
#define MAX_OPEN_FILE 	128
#define SIZE_SYNC 2048

// Inode mode
#define UNDEFINED_TYPE 0        // undefined
#define FILE_TYPE 1             // file
#define DIRECTORY_TYPE 2        // directory

// Open file flags
#define O_READ 0
#define O_WRITE 1
#define O_READWRITE 2

// Calculated const
#define NR_BLOCKS (FS_SIZE / BLOCK_SIZE)
#define SIZE_INODE sizeof(struct inode)
#define MAX_NUM_INODE_PER_BLOCK (BLOCK_SIZE / SIZE_INODE)
#define MAX_NUM_DIR_PER_BLOCK (BLOCK_SIZE / sizeof(struct directory))
#define SIZE_SUPER_BLOCK sizeof(struct super_block)
#define SIZE_DESCRIPTOR sizeof(struct descriptor)
#define SIZE_FILE sizeof(struct file)
#define SIZE_IND_DIR_BLOCK (BLOCK_SIZE / sizeof(u32_t))
#define SIZE_DIND_DIR_BLOCK ((BLOCK_SIZE / sizeof(u32_t)) * SIZE_IND_DIR_BLOCK)

/*
 The file system is 100MB, with each block size 2KB.
    1) The direct pointer block: NDIR_BLOCKS = 12 -> 12 * 2KB = 24KB
    2) The indirect pointer block: IND_BLOCK = 1
    each block could contain direct pointer = BLOCK_SIZE / sizeof(u32_t)
    total size = 1 * (1024 * 2 / 4) * 2KB = 1024 KB
    3) The double indirect pointer block: DIND_BLOCK = 1
    total size = 1 * 512 * 512 * 2KB = 524288 KB
    4) The total size = 24 + 1024 + 524,288 KB > FS_SIZE = 100,000KB
    Thus for this parameter setting, no need for trible indirect pointer.
*/

#define STAR "****************************"

/* stdout type */
#define TOSCREEN 0
#define TOFILE 1

#endif

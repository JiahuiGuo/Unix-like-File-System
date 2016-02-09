#ifndef FS_type_h
#define FS_type_h

#include "const.h"

typedef unsigned char  u8_t;    /* 1 byte */
typedef unsigned short u16_t;   /* 2 bytes */
typedef unsigned int   u32_t;   /* 4 bytes */
typedef unsigned long  u64_t;   /* 8 bytes */

// NOTICE: THE STRUCTURE SHOULD BE CACHE ALIGNED!!!

/* Inode Structure */
typedef struct inode{
    u32_t i_mode;                /* File or directory */
    u32_t i_size;                /* File size */
    time_t i_mtime;              /* Last modified time */
    time_t i_atime;              /* Last accessed time */
    time_t i_ctime;              /* Create time */
    char i_filename[NAME_LEN_MAX];
    u32_t i_parent_ino;          /* Parent inode */
    u32_t i_ino;                 /* inode number */
    u32_t i_link_num;            /* link number */
    u32_t i_direct_blocks[NDIR_BLOCKS];   /* Number of direct data block */
    u32_t i_indirect_blocks;              /* Number of indirected data block */
    u32_t i_double_indirect_blocks;       /* Number of double indirected data block */
} inode;

/* Super Block Structure */
typedef struct super_block{
    u16_t s_inodes_count;       /* Inodes count */
    u16_t s_free_inodes_count;  /* Free Inode count */
    u32_t s_free_blocks_count;	/* Free Block count */
    u32_t s_data_blocks_count;	/* Data blocks count */
    u32_t s_inode_map_position; /* First available inode position */
    u32_t s_inode_position;     /* Inode table position */
    u32_t s_data_map_position;  /* First available data position */
    u32_t s_data_position;      /* Data table position */
    u32_t s_max_open_file;      /* Maximum opened files */
} super_block;

/* Directory Structure */
typedef struct directory{
    u32_t d_inode;              /* Inode number */
    char d_filename[NAME_LEN_MAX];  /* Directory name */
} directory;

/* File Structure */
typedef struct file{
    inode * f_ino;              /* Inode Number */
    off_t f_offset;
    char *f_buffer;
    u16_t f_mode;
    u32_t f_usedBuf;
} file;

/* Descriptor Structure */
typedef struct descriptor{
    super_block * d_sb;
    char * d_data_bitmap_ptr;
    char * d_inode_bitmap_ptr;
    inode * d_root;
    inode * d_curInode;
    u32_t d_curDir;
    file * d_files;
    u32_t d_file_counts;
} descriptor;

#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "type.h"
#include "const.h"
#include "fs.h"
#include <stdarg.h>
#include <string.h>

descriptor * dptr;
char full_dir[NAME_LEN_MAX];
extern int stdout_type;
extern FILE *fd_out;

// allocate an inode
int fs_ialloc(){
    int ino;
    if (dptr->d_sb->s_free_inodes_count == 0) {
        fs_print_std("No inode available!\n");
        return -1;
    }
    
	for (ino = 1; ino < NR_INODES; ++ino)
	{
		if (dptr->d_inode_bitmap_ptr[ino] == 0)
		{
            dptr->d_inode_bitmap_ptr[ino] = 1;
            dptr->d_sb->s_free_inodes_count--;
			return ino;
		}
	}
	return -1;
}

int fs_balloc(){
    int bno;
    // All the blocks are allocated
    if (dptr->d_sb->s_free_blocks_count == 0) {
        fs_print_std("No block available!\n");
        return -1;
    }
    for (bno = 1; bno < NR_BLOCKS; ++bno) {
        if (dptr->d_data_bitmap_ptr[bno] == 0) {
            dptr->d_data_bitmap_ptr[bno] = 1;
            dptr->d_sb->s_free_blocks_count--;
            return bno;
        }
    }
    return -1;
}

// Caching initial information
int fs_init(){

    // print out the system detailed information
    // fs_print_std("Initializing the File System!\n");
    
    int fd;
    fd = open(FS_IMG_NAME, O_RDWR);
    
    dptr = (descriptor *)malloc(SIZE_DESCRIPTOR);
	dptr->d_sb = (super_block *)malloc(SIZE_SUPER_BLOCK);
    dptr->d_data_bitmap_ptr = (char *)malloc(NR_BLOCKS);
    dptr->d_inode_bitmap_ptr = (char *)malloc(NR_INODES);
    dptr->d_root = (inode *)malloc(SIZE_INODE * NR_INODES);
    dptr->d_files = (file *)malloc(SIZE_FILE * MAX_OPEN_FILE);
    
    // Read all the sb information into cache
    if (read(fd, dptr->d_sb, SIZE_SUPER_BLOCK) <= 0) {
        close(fd);
        return -1;
    }
	if (read(fd, dptr->d_data_bitmap_ptr, NR_BLOCKS) <= 0) {
        close(fd);
        return -1;
    }
    if (read(fd, dptr->d_inode_bitmap_ptr, NR_INODES) <= 0) {
        close(fd);
        return -1;
    }
    if (read(fd, dptr->d_root, SIZE_INODE * NR_INODES) <= 0){
        close(fd);
        return -1;
    }
    
    dptr->d_curDir = dptr->d_root->i_ino;
    dptr->d_curInode = dptr->d_root + dptr->d_curDir;
    dptr->d_file_counts = 0;
    memset(dptr->d_files, 0, SIZE_FILE * MAX_OPEN_FILE);
    
//
//    fs_print_std("Initialization finished! The FS is ready to use!\n");
//    fs_print_std("The current inode #:%d\n", dptr->d_root->i_ino);
    
	close(fd);
    return 0;
}

// mkfs
int fs_mkfs(){
    
    int fd;
    time_t curTime;
    curTime = time(0);
    
    // Get the file system descriptor
	fd = open(FS_IMG_NAME, O_RDWR|O_CREAT|O_TRUNC);
    
    // Allocate the blocks in memory
    super_block * sb;
    char * blockBitMap;
    char * inodeBitMap;
    inode * ino;

	sb = (super_block*) malloc (SIZE_SUPER_BLOCK);
	blockBitMap = (char*) malloc (NR_BLOCKS);
	memset(blockBitMap, 0, NR_BLOCKS);
    inodeBitMap = (char*) malloc (NR_INODES);
	memset(inodeBitMap, 0, NR_INODES);
    ino = (inode*) malloc(SIZE_INODE * NR_INODES);
	memset(ino, 0, SIZE_INODE * NR_INODES);
	
    // Configure the super block
    sb->s_inodes_count = NR_INODES;
    sb->s_free_inodes_count = NR_INODES;
    sb->s_free_blocks_count = NR_BLOCKS;
    sb->s_data_blocks_count = NR_BLOCKS;
    sb->s_data_map_position = SIZE_SUPER_BLOCK; /* Start right after the superblock */
    sb->s_inode_map_position = sb->s_data_map_position + sizeof(blockBitMap);   /* Start right after the block bitmap */
    sb->s_inode_position = sb->s_inode_map_position + sizeof(inodeBitMap);      /* Start right after the inode bitmap */
    sb->s_data_position = sb->s_inode_position + SIZE_INODE * NR_INODES;
    sb->s_max_open_file = MAX_OPEN_FILE;
    
	// Allocate the root Directory
	ino->i_mode = DIRECTORY_TYPE;
    ino->i_ino = ROOT_INODE;
    ino->i_parent_ino = ROOT_INODE;
    ino->i_ctime = curTime;
    ino->i_atime = curTime;
    ino->i_mtime = curTime;
    ino->i_link_num = 1;
    ino->i_size = 0;
    strcpy(ino->i_filename, "/");
    sb->s_free_inodes_count -= 1;
    inodeBitMap[0] = 1;
    
//    // Validate the sizeof on struct
//    fs_print_std("super block size : %lu bytes.\n", SIZE_SUPER_BLOCK);
//    fs_print_std("inode size : %lu bytes.\n", SIZE_INODE);
//    fs_print_std("descriptor size: %lu bytes.\n", SIZE_DESCRIPTOR);
    
    // Write these information into disk
	write(fd, sb, SIZE_SUPER_BLOCK);
	write(fd, blockBitMap, NR_BLOCKS);
	write(fd, inodeBitMap, NR_INODES);
	write(fd, ino, SIZE_INODE * NR_INODES);
	
	// Create a 100MB emulate File System
	lseek(fd, FS_SIZE, SEEK_SET);
	write(fd, "\n", 1);
	close(fd);
    
    // Print out information for checking
//    fs_print_std("mkfs: Formating the Disk...\n");
//    fs_print_std("Disk statistics:\n");
//    fs_print_std("# Super block: %d\n",1);
//    fs_print_std("# Inodes: %d\n", sb->s_inodes_count);
//    fs_print_std("# Data block: %d\n",sb->s_data_blocks_count);
//    fs_print_std("Creation time: %ld\n",ino->i_ctime);
//    fs_print_std("The ROOT inode #%d\n", ino->i_ino);
//    fs_print_std("The ROOT directory '%s'\n", ino->i_filename);
//	fs_print_std("Successfully create a %d Bytes file system!\n", FS_SIZE);
    close(fd);
    
    // Caching all the initial information into memory
	fs_init();
    
    return 0;
}

// create a file
int fs_create(char * filename){
    
    time_t curTime;
    curTime = time(0);
    
    int newNode;
	inode *ino, *newIno;
	int bno;
    
	ino = dptr->d_curInode;
	
	if (strncmp(filename, "./", 2) == 0)
	{
		return fs_create(filename + 2);
	}
	else if (strncmp(filename, "../", 3) == 0)
	{
        fs_chdir(dptr->d_root + ino->i_parent_ino);
        fs_create(filename + 3);
        fs_chdir(dptr->d_root + ino->i_ino);
		return 0;
	}
	
	newNode = fs_ialloc();
	if (newNode < 0)
	{
		fs_print_std("Error in allocating new inode!\n");
		return -1;
	}
	
    newIno = dptr->d_root + newNode;
    newIno->i_mode = FILE_TYPE;
    newIno->i_ctime = curTime;
    newIno->i_atime = curTime;
    newIno->i_mtime = curTime;
    newIno->i_parent_ino = ino->i_ino;
    newIno->i_size = 0;
    newIno->i_link_num = 1;
    strcpy(newIno->i_filename, filename);
	
	bno = fs_balloc();
	if (bno < 0)
	{
		fs_print_std("Error in allocating new data block!\n");
		return -1;
	}
    
	newIno->i_direct_blocks[0] = bno;
    
    if(fs_add_dir(ino, newNode) < 0){
        fs_print_std("Error in adding new dir!\n");
        return -1;
    }
    return 0;
}

//
int fs_open_atomic(inode * ino, int flag){
    
    int i;
    for (i = 0; i < dptr->d_sb->s_max_open_file; ++i){
        if (dptr->d_files[i].f_ino == NULL) {
            dptr->d_files[i].f_ino = ino;
            dptr->d_files[i].f_offset = 0;
            dptr->d_files[i].f_buffer = (char*)malloc(BUF_SIZE);
            dptr->d_files[i].f_usedBuf = 0;
            dptr->d_files[i].f_mode = flag;
            dptr->d_file_counts++;
            return i;
        }
    }
    return -1;
}

// open
int fs_open(char * filename, int flag){
	
	inode *ino;
    inode *newIno;;
	int fd = -1;

	ino = dptr->d_curInode;
	newIno = fs_get_inode(filename, ino);
	if (newIno == NULL)
	{
        if (flag == O_READ) {
            fs_print_std("Error in opening file!\n");
            fs_print_std("The file does not exist for read only!\n");
            return -1;
        }
        else{
            if(fs_create(filename) < 0){
                fs_print_std("Error creating a new file to write!\n");
                return -1;
            }
            newIno = fs_get_inode(filename, ino);
            fd = fs_open_atomic(newIno, flag);
        }
    }
	else
		fd = fs_open_atomic(newIno, flag);
    
    return fd;
}

//
int fs_get_block_idx_atomic(off_t offset, int index){
    u32_t idx;
    int fd;
    fd = open(FS_IMG_NAME, O_RDONLY);
    lseek(fd, dptr->d_sb->s_data_position + index * BLOCK_SIZE + offset * sizeof(u32_t), SEEK_SET);
    read(fd, &idx, sizeof(u32_t));
    close(fd);
    return idx;
}

//
int fs_get_block_idx(int bno, inode* ino){
    // block is in the direct block zone
    if (bno < NDIR_BLOCKS)
        return ino->i_direct_blocks[bno];
    // block is in indirected block zone
    else if (bno < SIZE_IND_DIR_BLOCK + NDIR_BLOCKS)
        return fs_get_block_idx_atomic(bno - NDIR_BLOCKS, ino->i_indirect_blocks);
    // block is in double indirected block zone
    else if (bno < SIZE_DIND_DIR_BLOCK + SIZE_IND_DIR_BLOCK + NDIR_BLOCKS){
        int b_idx_offset, b_idx;
        off_t offset;
        b_idx_offset = (bno - SIZE_IND_DIR_BLOCK - NDIR_BLOCKS) / SIZE_IND_DIR_BLOCK;
        offset = bno - SIZE_IND_DIR_BLOCK - NDIR_BLOCKS - b_idx_offset * NDIR_BLOCKS;
        b_idx = fs_get_block_idx_atomic(b_idx_offset, ino->i_indirect_blocks);
        return fs_get_block_idx_atomic(offset, b_idx);
    }
    // block is "outside" the file system
    else{
        fs_print_std("Error in getting block index!\n");
        return -1;
    }
    return -1;
}

//
int fs_get_disk_block(char* buffer, int bno, int offset, int size, file* file_ptr){
    int fd;
    int read_size;
    char tmp[BLOCK_SIZE + 1];
    bzero(tmp, BLOCK_SIZE + 1);
    fd = open(FS_IMG_NAME, O_RDONLY);
    lseek(fd, dptr->d_sb->s_data_position + BLOCK_SIZE * bno + offset, SEEK_SET);
    read_size = (int)read(fd, tmp, size);
    strncat(buffer, tmp, read_size);
    file_ptr->f_offset += size;
    close(fd);
    return 0;
}

//
int fs_read_atomic(int fd, char * buf, int size){
    file * file_ptr;
    int bno, bno_disk;
    int offset;
    if (dptr->d_files[fd].f_ino == NULL) {
        fs_print_std("No such file exists!\n");
        return -1;
    }
    else
        file_ptr = dptr->d_files + fd;
    
    if (file_ptr->f_mode == O_WRITE) {
        fs_print_std("Reading a write only file!\n");
        return -1;
    }
    
    if (file_ptr->f_offset > file_ptr->f_ino->i_size) {
        fs_print_std("Read out of file range!\n");
        return -1;
    }
    
    while (size > 0) {
        bno = (int)(file_ptr->f_offset / BLOCK_SIZE);
        offset = file_ptr->f_offset % BLOCK_SIZE;
        bno_disk = fs_get_block_idx(bno, file_ptr->f_ino);
        if (size < BLOCK_SIZE - offset)
            fs_get_disk_block(buf, bno_disk, offset, size, file_ptr);
        else
            fs_get_disk_block(buf, bno_disk, offset, BLOCK_SIZE - offset, file_ptr);
        size = size - (BLOCK_SIZE - offset);
    }
    
    return 0;
}

//read(<fd>,<size>)
int fs_read(int fd, int size){
    char* buf;
	buf = (char*)malloc(size + 1);
	memset(buf, 0, size + 1);

	if(fs_read_atomic(fd, buf, size) < 0){
        free(buf);
        return -1;
    }
	fs_print_std("%s\n", buf);
	free(buf);
    return 0;
}

//write(<fd>,<string>)
int fs_write(int fd, char *str){
    file* file_ptr;
    if (dptr->d_files[fd].f_ino == NULL){
        fs_print_std("No opened file exist!\n");
        return -1;
    }
	else {
		file_ptr = dptr->d_files + fd;
	}
	
	if (file_ptr->f_mode == O_READ){
		fs_print_std("The file is read only!\n");
		return -1;
	}
    
	strcat(file_ptr->f_buffer, str);
    file_ptr->f_usedBuf += strlen(str);
	
	if (file_ptr->f_usedBuf >= SIZE_SYNC){
		fs_sync_data(file_ptr);
		fs_sync();
	}
    return 0;
}

//seek(<fd>,<offset>)
int fs_seek(int fd, off_t offset){
    
    file* file_ptr;
    if (dptr->d_files[fd].f_ino == NULL)
    {
        fs_print_std("No opened file!\n");
        return -1;
    }
    else
        file_ptr = dptr->d_files + fd;
    file_ptr->f_offset = offset;
    return 0;
}

//close(<fd>)
int fs_close(int fd){
    
    if (dptr->d_files[fd].f_ino == NULL){
		fs_print_std("No opened file!\n");
		return -1;
	}
	else{
		file* file_ptr;
		file_ptr = dptr->d_files + fd;
		fs_sync_data(file_ptr);
		fs_sync();
        free(file_ptr->f_buffer);
        memset(file_ptr, 0, sizeof(file));
        dptr->d_file_counts--;
	}
    return 0;
}

//
int fs_get_full_dir(inode * ino){
    // Check whether it is the root node
    if (ino->i_ino == ROOT_INODE)
        strcpy(full_dir, dptr->d_root->i_filename);
    else
    {
        inode* parent = dptr->d_root + ino->i_parent_ino;
        fs_get_full_dir(parent);
        strcat(full_dir, ino->i_filename);
        strcat(full_dir, "/");
    }
    return 0;
}

//
int fs_get_subDir(int* array, inode* ino)
{
	inode* newIno;
	int index, newIno_size;
    int i;
	index = 0;
	newIno_size = ino->i_size;
	
	for (i = 0; i < NDIR_BLOCKS; ++i){
		newIno = dptr->d_root + ino->i_direct_blocks[i];
		if (ino->i_direct_blocks[i] == 0 || newIno->i_mode == UNDEFINED_TYPE){
			continue;
		}
		else{
			array[index++] = ino->i_direct_blocks[i];
			newIno_size--;
		}
		
		if (newIno_size <= 0){
			break;
		}
	}
	
	if (ino->i_indirect_blocks != 0)
	{
		int fd;
		fd = open(FS_IMG_NAME, O_RDONLY);
		lseek(fd, dptr->d_sb->s_data_position + BLOCK_SIZE * ino->i_indirect_blocks, SEEK_SET);
        
		int idx[BLOCK_SIZE / sizeof(u32_t)] = {0};
		read(fd, idx, BLOCK_SIZE);
		for (i = 0; i < BLOCK_SIZE / sizeof(u32_t); ++i){
			if (idx[i] == 0)
				continue;
			else{
				array[index++] = idx[i];
				ino->i_size--;
			}
			if (ino->i_size <= 0)
				break;
		}
        close(fd);
		
	}
    return 0;
}

//
inode * fs_get_inode(char * dirname, inode * ino){
    int ino_size, i;
    inode* newIno;
    int * idx;
	
	if (strcmp(dirname, ".") == 0)
	{
		return dptr->d_root + ino->i_ino;
	}
	else if (strcmp(dirname, "..") == 0)
	{
		return dptr->d_root + ino->i_parent_ino;
	}
	else if (strncmp(dirname, "./", 2) == 0)
	{
		return fs_get_inode(dirname + 2, ino);
	}
	else if (strncmp(dirname, "../", 3) == 0)
	{
		return fs_get_inode(dirname + 3, dptr->d_root + ino->i_parent_ino);
	}
	
	ino_size = ino->i_size;
    
	idx = (int *)malloc(ino_size * sizeof(int));
	
	fs_get_subDir(idx, ino);
	
	for (i = 0; i < ino_size; i++)
	{
		newIno = dptr->d_root + idx[i];
		if (strcmp(newIno->i_filename, dirname) == 0)
		{
			return dptr->d_root + idx[i];
		}
	}
    return NULL;
}


//change dir
int fs_chdir(inode * ino){
    
    // Track the inode
    dptr->d_curDir = ino->i_ino;
    dptr->d_curInode = dptr->d_root + dptr->d_curDir;
	memset(full_dir, 0, NAME_LEN_MAX);
	fs_get_full_dir(ino);
    return 0;
}

//cd(<dirname>)
int fs_cd(char * dirname){
    
    inode *curInode;
    curInode = dptr->d_curInode;
    
    inode *newInode;
    char* subDir;
	   
    // Check whether changing to root dir
    if (strcmp(dirname, "/") == 0 || strcmp(dirname, "") == 0) {
        fs_chdir(dptr->d_root);
    }
	else if ((subDir = strchr(dirname, '/')) == NULL)
	{
		newInode = fs_get_inode(dirname, curInode);
		if (newInode == NULL)
		{
			fs_print_std("cd: no such file or directory: %s\n", dirname);
			return -1;
		}
		else if (newInode->i_mode != DIRECTORY_TYPE)
		{
			fs_print_std("cd: no such file or directory: %s\n", newInode->i_filename);
			return -1;
		}
		fs_chdir(newInode);
        return 0;
	}
	else
	{
		*subDir = '\0';
		fs_cd(dirname);
		fs_cd(subDir + 1);
	}
    return 0;
    
}

int fs_get_free_block(inode * ino){
    int bno;
    for (bno = 0; bno < NDIR_BLOCKS; ++bno)
        if (ino->i_direct_blocks[bno] == 0)
            return bno;
    return -1;
}

int fs_add_dir(inode * curIno, int ino){
    
    int ino_size;
    int bno;
	int idx;
    int fd;
    int i;
    int idxBuf[BLOCK_SIZE / sizeof(u32_t)] = {0};

    ino_size = curIno->i_size;
    if (ino_size == 0)
		bno = 0;
	else if (0 < ino_size && ino_size < NDIR_BLOCKS)
	{
		bno = fs_get_free_block(curIno);
		if (bno < 0) return -1;
	}
	else
	{
		if (curIno->i_indirect_blocks != 0)
		{
			idx = curIno->i_indirect_blocks;
			bno = -1;
		}
		else
		{
			idx = fs_balloc();
			curIno->i_indirect_blocks = idx;
			bno = 0;
		}
	}
	
	if (curIno->i_indirect_blocks  == 0)
		curIno->i_direct_blocks[bno] = ino;
	else{
        fd = open(FS_IMG_NAME, O_RDWR);
        lseek(fd, dptr->d_sb->s_data_position + BLOCK_SIZE * curIno->i_indirect_blocks, SEEK_SET);
        if (bno == 0)
            idxBuf[0] = ino;
        else {
            read(fd, idxBuf, BLOCK_SIZE);
            for (i = 1; i < BLOCK_SIZE /sizeof(u32_t) ; i++){
                if (idxBuf[i] == 0){
                    idxBuf[i] = ino;
                    break;
                }
            }
            if (i == BLOCK_SIZE /sizeof(u32_t)){
                fs_print_std("Error in creating directory!\n");
                close(fd);
                return -1;
            }
        }
        lseek(fd, dptr->d_sb->s_data_position + BLOCK_SIZE * curIno->i_indirect_blocks, SEEK_SET);
        write(fd, idxBuf, BLOCK_SIZE);
        close(fd);
    }
    curIno->i_size++;
    
	if(fs_sync() < 0){
        fs_print_std("Unable to sync information to disk!\n");
        return -1;
    }
    return 0;
}

// Sync all the caching with disk
int fs_sync(){
    
    int fd;
    fd = open(FS_IMG_NAME, O_RDWR);
    if (fd < 0) {
        fs_print_std("Unable to open IMAGE file!\n");
        return -1;
    }
	lseek(fd, 0, SEEK_SET);

	// Overwrite all the information
    write(fd, dptr->d_sb, SIZE_SUPER_BLOCK);
	write(fd, dptr->d_data_bitmap_ptr, NR_BLOCKS);
	write(fd, dptr->d_inode_bitmap_ptr, NR_INODES);
	write(fd, dptr->d_root, SIZE_INODE * NR_INODES);
    
	close(fd);
    return 0;
}

//
int fs_put_block(int idx, off_t offset, file* file_ptr, int size){
    int fd;
	fd = open(FS_IMG_NAME, O_WRONLY);
	lseek(fd, dptr->d_sb->s_data_position + idx * BLOCK_SIZE + offset, SEEK_SET);
	write(fd, file_ptr->f_buffer, size);
	file_ptr->f_offset += size;
    file_ptr->f_ino->i_size += size;
	close(fd);
    return 0;
}

// Sync all the data block with disk (Not suitable to keep in cache)
int fs_sync_data(file* file_ptr){
    
    int bno, offset, b_idx, newbno;
	bno = (int)(file_ptr->f_offset / BLOCK_SIZE);
	offset = (int)(file_ptr->f_offset - BLOCK_SIZE * bno);
	
    b_idx = fs_get_block_idx(bno, file_ptr->f_ino);
    
	if (file_ptr->f_usedBuf < BLOCK_SIZE - offset)
		fs_put_block(b_idx, offset, file_ptr, file_ptr->f_usedBuf);
	else if (file_ptr->f_usedBuf == BLOCK_SIZE - offset){
		fs_put_block(b_idx, offset, file_ptr, file_ptr->f_usedBuf);
		newbno = fs_balloc_atomic(file_ptr->f_ino);
	}
	else {
		fs_put_block(b_idx, offset, file_ptr, BLOCK_SIZE - offset);
		strcpy(file_ptr->f_buffer, file_ptr->f_buffer + BLOCK_SIZE - offset);
		newbno = fs_balloc_atomic(file_ptr->f_ino);
		fs_put_block(newbno, 0, file_ptr, file_ptr->f_usedBuf - (BLOCK_SIZE - offset));
	}
    
	memset(file_ptr->f_buffer, 0, BUF_SIZE);
	file_ptr->f_usedBuf = 0;

    return 0;
}

//
int fs_mkdir_atomic(char * dirname){
    
    time_t curTime;
    curTime = time(0);
    
    directory curDir;
    memset(curDir.d_filename, 0, sizeof(curDir.d_filename));
	strcpy(curDir.d_filename, dirname);
	
	if (strcmp(curDir.d_filename, "/") == 0) return -1;
	else if (curDir.d_filename[strlen(curDir.d_filename) - 1] == '/')
	{
		curDir.d_filename[strlen(curDir.d_filename) - 1] = '\0';
	}
	
	inode* curDirNode = dptr->d_curInode;
    int newNode = fs_ialloc();
	if (newNode < 0) return -1;
	
	inode* newDir = dptr->d_root + newNode;
    newDir->i_ctime = curTime;
    newDir->i_atime = curTime;
    newDir->i_mtime = curTime;
    newDir->i_ino = newNode;
    newDir->i_mode = DIRECTORY_TYPE;
    newDir->i_link_num = 1;
    newDir->i_parent_ino = dptr->d_curDir;
    strcpy(newDir->i_filename, curDir.d_filename);
	
	fs_add_dir(curDirNode, newNode);
    
    return 0;
}

//mkdir(<dirname>)
int fs_mkdir(char * dirname){

    char * subDir;
    directory curDir;
    memset(curDir.d_filename, 0, sizeof(curDir.d_filename));
	
    if(strlen(dirname) > NAME_LEN_MAX)
    {
        fs_print_std("The directory name is too long!\n");
        return -1;
    }
    
	else if (strcmp(dirname, "") == 0){
        fs_print_std("Empty directory name!\n");
		return -1;
	}
    
    else if (strcmp(dirname, "/") == 0){
        fs_print_std("Root directory is already there!\n");
        return -1;
    }
    
	else if ((subDir = strchr(dirname, '/')) == NULL)
	{
        if(fs_check_exist(dptr->d_curInode, dirname) < 0){
            fs_print_std("Directory already exists!\n");
            return -1;
        }
		fs_mkdir_atomic(dirname);
	}
	else if (subDir == dirname)
	{
		fs_mkdir(subDir + 1);
	}
	else
	{
		strncpy(curDir.d_filename, dirname, subDir - dirname);
        //fs_print_std("%s",curDir.d_filename);
        if (!fs_check_exist(dptr->d_curInode,curDir.d_filename))
            fs_mkdir_atomic(curDir.d_filename);
		fs_cd(curDir.d_filename);
		fs_mkdir(subDir + 1);
		fs_cd("..");
	}
	
	return 0;
}

//
int fs_free_block(int bno){
    dptr->d_data_bitmap_ptr[bno] = 0;
    dptr->d_sb->s_free_blocks_count++;
    return 0;
}
//
int fs_rmdir_atomic(inode* ino){
    
    inode * parent;
    parent = dptr->d_root + ino->i_parent_ino;
    int ino_size;
    int dir_size = 0;
    int i;
	ino_size = parent->i_size;
	
	for (i = 0; i < NDIR_BLOCKS; ++i){
		if (parent->i_direct_blocks[i] == ino->i_ino){
            // Reset relative information
            parent->i_direct_blocks[i] = 0;
            parent->i_size--;
            dptr->d_inode_bitmap_ptr[ino->i_ino] = 0;
            dptr->d_sb->s_free_inodes_count++;
            return 0;
		}
		else if (parent->i_direct_blocks[i] == 0)
			continue;
		else {
			ino_size--;
			dir_size++;
		}
		if (ino_size <= 0) return -1;
	}
	
	if (parent->i_indirect_blocks != 0)
	{
		int fd;
		fd = open(FS_IMG_NAME, O_RDWR);
		lseek(fd, dptr->d_sb->s_data_position + BLOCK_SIZE * parent->i_indirect_blocks, SEEK_SET);
        
		int idx[BLOCK_SIZE / sizeof(u32_t)] = {0};
        read(fd, idx, BLOCK_SIZE);
		for (i = 0; i < BLOCK_SIZE / sizeof(u32_t); ++i){
			if (idx[i] == 0)
				continue;
			else if (idx[i] == ino->i_ino){
				idx[i] = 0;
				parent->i_size--;
				if (parent->i_size == dir_size){
					fs_free_block(parent->i_indirect_blocks);
					parent->i_indirect_blocks = 0;
				}
				
                dptr->d_inode_bitmap_ptr[ino->i_ino] = 0;
                dptr->d_sb->s_free_inodes_count++;
				
				lseek(fd, dptr->d_sb->s_data_position + BLOCK_SIZE * parent->i_indirect_blocks, SEEK_SET);
				write(fd, idx, BLOCK_SIZE);
				close(fd);
				return 0;
			}
			else
				ino_size--;
			
			if (ino_size <= 0) return -1;
		}
	}
	return -1;
}

//rmdir(<dirname>)
int fs_rmdir(char *dirname){
    
    inode* ino;
	ino = fs_get_inode(dirname, dptr->d_curInode);
	
    if (ino == NULL){
		fs_print_std("No such file or directory!\n");
		return -1;
	}
	else if (ino->i_mode == FILE_TYPE){
		fs_print_std("Not a directory!\n");
		return -1;
	}
	else if (ino->i_size > 0){
		fs_print_std("Not an empty directory!\n");
		return -1;
	}
	else {
		if (fs_rmdir_atomic(ino) < 0){
			fs_print_std("Error in rmdir!\n");
			return -1;
		}
		fs_sync();
	}
    return 0;
}

//
int fs_cp_block(int fd, int src, int dst){
    
	char buffer[BLOCK_SIZE];
    bzero(buffer, BLOCK_SIZE);
	lseek(fd, dptr->d_sb->s_data_position + src * BLOCK_SIZE, SEEK_SET);
	read(fd, buffer, BLOCK_SIZE);
	lseek(fd, dptr->d_sb->s_data_position + dst * BLOCK_SIZE, SEEK_SET);
	write(fd, buffer, BLOCK_SIZE);
	return 0;
}

//
int fs_write_block(int idx, off_t offset, int bno){
    int fd;
	fd = open(FS_IMG_NAME, O_WRONLY);
	lseek(fd, dptr->d_sb->s_data_position + idx * BLOCK_SIZE + offset * sizeof(u32_t), SEEK_SET);
	write(fd, &bno, sizeof(u32_t));
	close(fd);
	return 0;
}

//
int fs_balloc_atomic(inode* ino){
    int cur_bno;
	int bno = fs_balloc();
	if (bno < 0){
		fs_print_std("Erorr in allocation a block!\n");
		return -1;
	}
	cur_bno = ino->i_size / BLOCK_SIZE;
    // Fit within the direct block
	if (cur_bno < NDIR_BLOCKS)
        ino->i_direct_blocks[cur_bno] = bno;
    // Direct block is right used up
	else if (cur_bno == NDIR_BLOCKS){
        ino->i_indirect_blocks = fs_balloc();
		fs_write_block(ino->i_indirect_blocks, 0, bno);
	}
    // Direct block is used up, but indirect block is not used up yet
	else if (cur_bno < NDIR_BLOCKS + SIZE_IND_DIR_BLOCK)
		fs_write_block(ino->i_indirect_blocks, cur_bno - NDIR_BLOCKS, bno);
    // Both Direct block and indirect block are right used up
	else if (cur_bno == NDIR_BLOCKS + SIZE_IND_DIR_BLOCK){
        // Allocate a new direct pointer in double-indirect pointer
		fs_write_block(fs_balloc(), 0, bno);
        // Allocate a new double-indirect pointer
		ino->i_double_indirect_blocks = fs_balloc();
		fs_write_block(ino->i_double_indirect_blocks, 0, ino->i_double_indirect_blocks);
	}
	else if (cur_bno <  SIZE_DIND_DIR_BLOCK + SIZE_IND_DIR_BLOCK + NDIR_BLOCKS){
		int idx = (cur_bno - (NDIR_BLOCKS + SIZE_IND_DIR_BLOCK)) / SIZE_IND_DIR_BLOCK;
		int offset = (cur_bno - (NDIR_BLOCKS + SIZE_IND_DIR_BLOCK)) % SIZE_IND_DIR_BLOCK;
		if (offset == 0){
            int new_bno = fs_balloc();
			fs_write_block(ino->i_double_indirect_blocks, idx, new_bno);
			fs_write_block(new_bno, 0, bno);
		}
		else{
			int new_bno = fs_get_block_idx_atomic(idx, ino->i_double_indirect_blocks);
			fs_write_block(new_bno, offset + 1, bno);
		}
	}
	return bno;
}

//
int fs_link_atomic(inode * src, inode * dst){
    
    int fd;
	fd = open(FS_IMG_NAME, O_RDWR);
    int bno;
    int b_idx = 0;
    int src_bno, dst_bno;
    int i;
    bno = src->i_size / BLOCK_SIZE;
	if (src->i_size % BLOCK_SIZE != 0)
        b_idx++;
	
    fs_cp_block(fd, src->i_direct_blocks[0], dst->i_direct_blocks[0]);
	
	if (b_idx == 1){
        dst->i_size = src->i_size;
		close(fd);
		return 0;
	}
	else
        dst->i_size = BLOCK_SIZE;

	for (i = 1; i < bno; ++i){
		src_bno = fs_get_block_idx(i,src);
		dst_bno = fs_balloc_atomic(dst);
		fs_cp_block(fd, src_bno, dst_bno);
	}
    
    dst->i_size = src->i_size;
    close(fd);
    return 0;
}

//link(<src>,<dst>)
int fs_link(char* src, char* dst){
    inode *curIno, *srcIno, *dstIno;
    curIno = dptr->d_curInode;
    srcIno = fs_get_inode(src, curIno);
    if (srcIno == NULL) {
        fs_print_std("No such file!\n");
        return -1;
    }
    else if (srcIno->i_mode == DIRECTORY_TYPE){
        fs_print_std("No such file!\n");
        return -1;
    }
	dstIno = fs_get_inode(dst, curIno);
    if (dstIno != NULL) {
        fs_print_std("File exists!\n");
        return -1;
    }
    fs_create(dst);
    dstIno = fs_get_inode(dst, curIno);
	fs_link_atomic(srcIno, dstIno);
	return 0;
}

int fs_free_data_block(inode* ino, int bsize){
    int i, bno_idx;
	for (i = 0; i < bsize; i++){
		bno_idx = fs_get_block_idx(i, ino);
        if(bno_idx < 0){
            fs_print_std("Error in find the block!\n");
            return -1;
        }
        dptr->d_data_bitmap_ptr[bno_idx] = 0;
        dptr->d_sb->s_free_blocks_count++;
	}
    return 0;
}

//unlink(<name>)
int fs_unlink(char *name){
    
    inode *ino;
    inode *curDir;
    int bsize;

    curDir = dptr->d_curInode;
    ino = fs_get_inode(name, curDir);
	if (ino == NULL){
		fs_print_std("No such file or directory!\n");
		return -1;
	}
    else if (ino->i_mode == DIRECTORY_TYPE){
		fs_print_std("Not a file!\n");
		return -1;
	}
    bsize = ino->i_size / BLOCK_SIZE + 1;
	fs_free_data_block(ino, bsize);
	fs_rmdir_atomic(ino);
	fs_sync();
    return 0;
}

//stat(<name>)
int fs_stat(char *name){
    inode *ino;
    inode *curDir;
    curDir = dptr->d_curInode;
    char* c_time_str;
    char* a_time_str;
    char* m_time_str;
    ino = fs_get_inode(name, curDir);
	if (ino == NULL)
        fs_print_std("Error in stating!\n");
    else {
        // file type
        if (ino->i_mode == DIRECTORY_TYPE)
            fs_print_std("File Type: Directory\n");
        else if (ino->i_mode == FILE_TYPE)
            fs_print_std("File Type: File\n");
        else
            fs_print_std("File Type: Undefined type\n");
        // file name
        fs_print_std("File name: %s\n", ino->i_filename);
        // file size
        fs_print_std("Size: %d\n", ino->i_size);
        // file link
        fs_print_std("Link: %d\n", ino->i_link_num);
        // file time
        c_time_str = ctime(&ino->i_ctime);
        a_time_str = ctime(&ino->i_atime);
        m_time_str = ctime(&ino->i_mtime);
        fs_print_std("Creation time: %s", c_time_str);
        fs_print_std("Access time: %s", a_time_str);
        fs_print_std("Modification time: %s", m_time_str);
        // file inode
        fs_print_std("Inode #%d\n", ino->i_ino);
    }
	return 0;
}

//TODO DEBUG
int fs_list_dir(inode* ino){
    if (ino->i_mode == DIRECTORY_TYPE)
        fs_print_std("%s/\t", ino->i_filename);
    else if (ino->i_mode == FILE_TYPE)
        fs_print_std("%s\t", ino->i_filename);
    else if (ino->i_mode == UNDEFINED_TYPE){
        fs_print_std("Error in listing directories!\n");
        return -1;
    }
    return 0;
}

//ls
int fs_ls(){
    // Fake the current dir and parent dir
    fs_print_std(".\t..\t");

    inode *ino, *newIno;
    int* newInoIdx;
    int newIno_size;
	int i;
    
    ino = dptr->d_curInode;
	newIno_size = ino->i_size;
	newInoIdx = (int*)malloc(sizeof(int) * newIno_size);
	fs_get_subDir(newInoIdx, ino);
	
	for (i = 0; i < newIno_size; i++){
		newIno = dptr->d_root + newInoIdx[i];
		fs_list_dir(newIno);
	}
    
	free(newInoIdx);
	
    fs_print_std("\n");
	return 0;
}

//cat(<filename>)
int fs_cat(char* filename){
    inode* curIno;
    inode* ino;
    curIno = dptr->d_curInode;
	ino = fs_get_inode(filename, curIno);
	if (ino == NULL){
		fs_print_std("No such file exists!\n");
		return -1;
	}
	else if (ino->i_mode == DIRECTORY_TYPE){
		fs_print_std("Error in cat a directory\n");
		return -1;
	}
	
	if (ino->i_size > 0){
		int fd = fs_open(filename, O_READ);
		fs_read(fd, ino->i_size);
		fs_close(fd);
	}
    return 0;
}

//cp(<src>,<dst>)
int fs_cp(char* src, char* dst) {
    inode *curIno, *srcIno, *dstIno;
    curIno = dptr->d_curInode;

    srcIno = fs_get_inode(src, curIno);
	if (srcIno == NULL){
		fs_print_std("No such file exists!\n");
		return -1;
	}
	else if (srcIno->i_mode == DIRECTORY_TYPE){
		fs_print_std("No such file exists!\n");
		return -1;
	}
	
	dstIno = fs_get_inode(dst, curIno);
	if (dstIno != NULL){
		if (dstIno->i_mode == DIRECTORY_TYPE){
            fs_print_std("Destination is a directory!\n");
		}
		else if (dstIno->i_mode == FILE_TYPE){
			fs_unlink(dst);
			fs_link_atomic(srcIno, dstIno);
		}
	}
	else
        fs_link_atomic(srcIno, dstIno);
	    
	return 0;
}


int fs_print_tree(int height){
    int i;
    for (i = 0; i < height; ++i) {
        fs_print_std("\t");
    }
    fs_print_std("----");
    return 0;
}

//
int fs_list_tree(inode* ino, int height){
	inode* child;
	int* idx;
    int i;
    idx = (int*)malloc(sizeof(int) * ino->i_size);
	fs_get_subDir(idx, ino);
	for (i = 0; i < ino->i_size; ++i){
		child = dptr->d_root + idx[i];
        if (child->i_mode == FILE_TYPE){
            fs_print_tree(height);
			fs_print_std("%s (%d bytes)\n", child->i_filename, child->i_size);
		}
		else if (child->i_mode == DIRECTORY_TYPE && child->i_size == 0){
            fs_print_tree(height);
			fs_print_std("%s\n", child->i_filename);
		}
		else {
            fs_print_tree(height);
			fs_print_std("%s\n", child->i_filename);
			fs_list_tree(child, height + 1);
		}
	}
	free(idx);
    return 0;
}

//tree
int fs_tree() {
    fs_list_tree(dptr->d_curInode, 0);
    return 0;
}

//import(<srcname>,<dstname>)
int fs_import(char *srcname, char *dstname) {
    
    int src_fd, dst_fd;
    inode *curIno, *dstIno;
    char buffer[BUF_SIZE];
    bzero(buffer, BUF_SIZE);
    curIno = dptr->d_curInode;
    dstIno = fs_get_inode(dstname, curIno);
	if (dstIno != NULL){
		fs_print_std("File already exists!\n");
		return -1;
	}
	
	src_fd = open(srcname, O_RDONLY);
	if (src_fd == -1){
		fs_print_std("Fail to open src file!\n");
		return -1;
	}
	
    dst_fd = fs_open(dstname, O_WRITE);
	while (read(src_fd, buffer, BUF_SIZE - 1) > 0){
		fs_write(dst_fd, buffer);
        bzero(buffer, BUF_SIZE);
	}
	
	close(src_fd);
	fs_close(dst_fd);
    return 0;
}

//export(<srcname>,<dstname>)
int fs_export(char *srcname, char *dstname) {
    int src_fd, dst_fd;
    inode *curIno, *srcIno;
    curIno = dptr->d_curInode;
    char* buffer;
	srcIno = fs_get_inode(srcname, curIno);
	
	if (srcIno == NULL){
		fs_print_std("No such file exists!\n");
		return -1;
	}
	
	dst_fd = open(dstname, O_WRONLY | O_TRUNC | O_CREAT);
	if (dst_fd == -1){
		fs_print_std("Fail to open dst file!\n");
		return -1;
	}
	
	src_fd = fs_open(srcname, O_READ);
	
	buffer = (char*) malloc(srcIno->i_size);
	memset(buffer, 0, srcIno->i_size);
    fs_read_atomic(src_fd, buffer, srcIno->i_size);
	write(dst_fd, buffer, srcIno->i_size);
	
	close(dst_fd);
	fs_close(src_fd);
	
    return 0;
}

//
int fs_check_exist(inode* ino, char * dirname){
	int i;
	inode* newIno;
    int newIno_size;
    int * idx;
    newIno_size = ino->i_size;
    idx = (int*)malloc(sizeof(int)*newIno_size);

    fs_get_subDir(idx, ino);
	
	for (i = 0; i < newIno_size; ++i){
        if (newIno_size <= 0){
			free(idx);
			return 0;
		}
		newIno = dptr->d_root + idx[i];
		
		if (strcmp(newIno->i_filename, dirname) == 0){
			free(idx);
			return -1;
		}
	}
    free(idx);
    return 0;
}

//
void fs_print_std(const char *format, ...)
{
    va_list args;
    va_start(args, format);
	stdout_type == TOSCREEN ? vprintf(format, args) : vfprintf(fd_out, format, args);
	va_end(args);
}




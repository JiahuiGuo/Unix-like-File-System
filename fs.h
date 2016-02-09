#ifndef FS_fs_h
#define FS_fs_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "const.h"
#include "type.h"

// Implementated high level shell commands
int fs_mkfs();

int fs_open(char*, int);
int fs_read(int, int);
int fs_write(int, char*);
int fs_seek(int, off_t);
int fs_close(int);
int fs_mkdir(char*);
int fs_rmdir(char*);
int fs_cd(char*);
int fs_link(char*, char*);
int fs_unlink(char*);
int fs_stat(char*);
int fs_ls();
int fs_cat(char*);
int fs_cp(char*, char*);
int fs_tree();
int fs_import(char*, char*);
int fs_export(char*, char*);

// Implementated low level function wrapper
int fs_mkdir_atomic(char*);
int fs_open_atomic(inode*, int);
int fs_read_atomic(int, char*, int);
int fs_link_atomic(inode*, inode*);
int fs_balloc_atomic(inode*);

// Low level utility functions
int fs_init();
int fs_ialloc();
int fs_balloc();
int fs_sync();
int fs_sync_data(file*);
int fs_free_block(int);
int fs_free_data_block(inode*, int);
int fs_create(char*);
int fs_get_full_dir(inode*);
int fs_add_dir(inode*, int);
int fs_list_dir(inode*);
int fs_chdir(inode*);
int fs_put_block(int, off_t, file*, int);
int fs_get_disk_block(char*, int, int, int, file*);
int fs_get_block_idx(int, inode*);
int fs_get_block_idx_atomic(off_t, int);
int fs_check_exist(inode*, char*);
inode * fs_get_inode(char*, inode*);
int fs_print_tree(int);
int fs_list_tree(inode*, int);
void fs_print_std(const char *format, ...);

#endif

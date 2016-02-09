#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "const.h"
#include "type.h"
#include "shell.h"
#include "fs.h"

//static char buffer[CHUNK_SIZE];
static char *m_argv[PARA_SIZE];
FILE *fd_out;
int stdout_type;

/* Function wrapper, do the arguments validation */

//mkfs
int m_mkfs(){
    if(fs_mkfs() < 0){
        fs_print_std("Error in mkfs!\n");
        return -1;
    }
    return 0;
}

// open
int m_open(int m_argc, char *m_argv[]){
    int fd = 0;
    if(m_argc < 3)
    {
        fs_print_std("Usage: open <filename> <flag>");
        return -1;
    }
    else
    {
        char *filename = m_argv[1];
        char *flag = m_argv[2];
        if(strcmp(flag, "r") == 0){
            fd = fs_open(filename, O_READ);
            if(fd < 0){
                fs_print_std("Open file to read fails!\n");
                return -1;
            }
        }
        else if(strcmp(flag, "w") == 0){
            fd = fs_open(filename, O_WRITE);
            if(fd < 0){
                fs_print_std("Open file to write fails!\n");
                return -1;
            }
        }
        else if(strcmp(flag, "rw") == 0){
            fd = fs_open(filename, O_READWRITE);
            if(fd < 0){
                fs_print_std("Open file to read and write fails!\n");
                return -1;
            }
        }
        else{
            fs_print_std("<flag> should be in set ('r', 'w', 'rw')!\n");
            return -1;
        }
    }
    fs_print_std("SUCCESS, fd=%d\n",fd);
    return fd;
}

//read(<fd>,<size>)
int m_read(int m_argc, char *m_argv[]){
    int fd;
    int size;
    if(m_argc < 3){
        fs_print_std("Usage: read <fd> <size>\n");
        return -1;
    }
    else{
        fd = atoi(m_argv[1]);
        size = atoi(m_argv[2]);
        if (fd < 0 || size < 0) {
            fs_print_std("Illegal file descriptor or size to read!\n");
            return -1;
        }
        if(fs_read(fd, size) < 0){
            fs_print_std("Unable to read the file!\n");
            return -1;
        }
    }
    return 0;
}

//write(<fd>,<string>)
int m_write(int m_argc, char *m_argv[]){
    int fd;
    if(m_argc < 3){
        fs_print_std("Usage: write <fd> <string>\n");
        return -1;
    }
    else{
        fd = atoi(m_argv[1]);
        if(fs_write(fd, m_argv[2]) < 0){
            fs_print_std("Error in write!\n");
            return -1;
        };
    }
    return 0;
}

//seek(<fd>,<offset>)
int m_seek(int m_argc, char *m_argv[]){
    int fd;
    int offset;
    if(m_argc < 3){
        fs_print_std("Usage: write <fd> <offset>\n");
        return -1;
    }
    else{
        fd = atoi(m_argv[1]);
        offset = atoi(m_argv[2]);
        if (fd < 0)
            fs_print_std("Invalid file descriptor!\n");
        if (offset < 0)
            fs_print_std("Invalid offset!\n");
        int ret = fs_seek(fd,offset);
        if(ret < 0){
            fs_print_std("Error in seek!\n");
            return -1;
        }
    }
    return 0;
}

//close(<fd>)
int m_close(int m_argc, char *m_argv[]){
    int fd;
    if(m_argc < 2){
        fs_print_std("Usage: close <fd>\n");
        return -1;
    }
    else{
        fd = atoi(m_argv[1]);
        int ret = fs_close(fd);
        if(ret < 0){
            fs_print_std("Error in close!\n");
            return -1;
        }
    }
    return 0;
}


//mkdir(<dirname>)
int m_mkdir(int m_argc, char *m_argv[]){
    char *path;
    if(m_argc < 2){
        fs_print_std("Usage: mkdir <dirname>\n");
        return -1;
    }
    else{
        path = m_argv[1];
        fs_print_std("%s\n",path);
        int ret = fs_mkdir(path);
        if(ret < 0){
            fs_print_std("Error in creating directory!\n");
            return -1;
        }
    }
    return 0;
}

//rmdir(<dirname>)
int m_rmdir(int m_argc, char *m_argv[]){
    char *path;
    if(m_argc < 2){
        fs_print_std("Usage: rmdir <dirname>\n");
        return -1;
    }
    else{
        path = m_argv[1];
        int ret = fs_rmdir(path);
        if(ret < 0){
            fs_print_std("Error in deleting directory!\n");
            return -1;
        }
    }
    return 0;
}

//cd(<dirname>)
int m_cd(int m_argc, char *m_argv[]){
    char *path;
    if(m_argc < 2){
        fs_print_std("Usage: cd <dirname>\n");
        return -1;
    }
    else{
        path = m_argv[1];
        int ret = fs_cd(path);
        if(ret < 0){
            fs_print_std("Error in changing directory!\n");
            return -1;
        }
    }
    return 0;
}


//link(<src>,<dst>)
int m_link(int m_argc, char *m_argv[]){
    char *src;
	char *dst;
	if (m_argc < 3) {
	    fs_print_std("Usage: link <src> <dst>\n");
		return -1;
	}
	else {
	    src = m_argv[1];
		dst = m_argv[2];
		int ret = fs_link(src, dst);
		if (ret < 0) {
		    fs_print_std("Link error!\n");
            return -1;
		}
	}
	return 0;
}

//unlink(<name>)
int m_unlink(int m_argc, char *m_argv[]){
    char *name;
	if (m_argc < 3) {
	    fs_print_std("Usage: link <src> <dst>\n");
		return -1;
	}
	else {
	    name = m_argv[1];
		int ret = fs_unlink(name);
		if (ret < 0) {
		    fs_print_std("Unlink error!\n");
            return -1;
		}
	}
	return 0;
}

//stat(<name>)
int m_stat(int m_argc, char *m_argv[]){
    if (m_argc < 2) {
	    fs_print_std("Usage: stat <name>\n");
		return -1;
	}
	else if(fs_stat(m_argv[1]) < 0) {
	    fs_print_std("Error in stat!\n");
		return -1;
	}
    return 0;
}

//
//ls
int m_ls(int m_argc, char *m_argv[]){
    int ret;
    if (m_argc < 1) {
	    fs_print_std("Usage: ls\n");
		return -1;
	}
    ret = fs_ls();
	return ret;
}
//
//cat
int m_cat(int m_argc, char *m_argv[]){
    
    return 0;
}

//cp(<src>,<dst>)
int m_cp(int m_argc, char *m_argv[]) {
    char *src;
	char *dst;
	if (m_argc < 3) {
	    fs_print_std("Usage: cp <src> <dst>\n");
		return -1;
	}
	else {
	    src = m_argv[1];
		dst = m_argv[2];
    }
    fs_cp(src, dst);
	return 0;
}


//tree
int m_tree(int m_argc, char *m_argv[]){
    if (m_argc < 1) {
        fs_print_std("Usage: tree\n");
        return -1;
    }
    if(fs_tree() < 0){
        fs_print_std("Tree failed!\n");
        return -1;
    }
    return 0;
}

//import(<srcname>,<dstname>)
int m_import(int m_argc, char *m_argv[]) {
    char *src;
	char *dst;
	if (m_argc < 3) {
	    fs_print_std("Usage: import <src> <dst>\n");
		return -1;
	}
	else {
	    src = m_argv[1];
		dst = m_argv[2];
    }
    fs_import(src, dst);
    return 0;
}

//export(<srcname>,<dstname>)
int m_export(int m_argc, char *m_argv[]) {
    char *src;
	char *dst;
	if (m_argc < 3) {
	    fs_print_std("Usage: export <src> <dst>\n");
		return -1;
	}
	else {
	    src = m_argv[1];
		dst = m_argv[2];
    }
    fs_export(src, dst);
    return 0;
}


int m_sh(int m_argc, char *m_argv[]){
    char *m_stdin, *m_stdout;
    if (m_argc < 5) {
        fs_print_std("Usage: sh < input > output\n");
        return -1;
    }
    else {
        m_stdin = m_argv[2];
        m_stdout = m_argv[4];
    }
    m_batch(m_stdin, m_stdout);
    return 0;
};

void m_cmd(int m_argc, char* cmd)
{
    // mkfs
    if(strcmp(cmd, "mkfs") == 0){
        m_mkfs();
    }
    //open(<filename>, <flag>)
    else if(strcmp(cmd, "open") == 0){
        int fd;
        fd = m_open(m_argc, m_argv);
        fs_print_std("fd: %d\n", fd);
    }
        
    //read(<fd>,<size>)
    else if(strcmp(cmd, "read") == 0){
        m_read(m_argc, m_argv);
    }
    
    //write(<fd>,<string>)
    else if(strcmp(cmd, "write") == 0){
        m_write(m_argc, m_argv);
    }
    
    //seek(<fd>,<offset>)
    else if(strcmp(cmd, "seek") == 0){
        m_seek(m_argc, m_argv);
    }
    
    //close(<fd>)
    else if(strcmp(cmd, "close") == 0){
        m_close(m_argc, m_argv);
    }
    
    //mkdir(<dirname>)
    else if(strcmp(cmd, "mkdir") == 0){
        m_mkdir(m_argc, m_argv);
    }

    //rmdir(<dirname>)
    else if(strcmp(cmd, "rmdir") == 0){
        m_rmdir(m_argc, m_argv);
    }
    
    //cd(<dirname>)
    else if(strcmp(cmd, "cd") == 0){
        m_cd(m_argc, m_argv);
    }

    //link(<src>,<dst>)
    else if(strcmp(cmd, "link") == 0){
        m_link(m_argc, m_argv);
    }
    
    //unlink(<name>)
    else if(strcmp(cmd, "unlink") == 0){
        m_unlink(m_argc, m_argv);
    }
    
    //stat(<name>)
    else if(strcmp(cmd, "stat") == 0){
        m_stat(m_argc, m_argv);
    }
    
    //ls
    else if(strcmp(cmd, "ls") == 0){
        m_ls(m_argc, m_argv);
    }
    
    //cat(<filename>)
    else if(strcmp(cmd, "cat") == 0){
        m_cat(m_argc, m_argv);
    }
    
    //cp(<src>,<dst>)
    else if(strcmp(cmd, "cp") == 0){
        m_cp(m_argc, m_argv);
    }
    
    //tree
    else if(strcmp(cmd, "tree") == 0){
        m_tree(m_argc, m_argv);
    }
    
    //import(<srcname>,<dstname>)
    else if(strcmp(cmd, "import") == 0){
        m_import(m_argc, m_argv);
    }
    
    //export(<srcname>,<dstname>)
    else if(strcmp(cmd, "export") == 0){
        m_export(m_argc, m_argv);
    }
    
    //exit
    else if(strcmp(cmd, "exit") == 0){
        exit(0);
    }
    
    else if (strcmp(cmd, "sh") == 0){
        m_sh(m_argc, m_argv);
    }
    
    else
        fs_print_std("cmd not found!\n");
}

//free m_argv
void free_m_argv()
{
	int index;
	for(index = 0; m_argv[index] != NULL; index++) {
		bzero(m_argv[index], strlen(m_argv[index])+1);
		free(m_argv[index]);
		m_argv[index] = NULL;
	}
}

//parse the arguments and store them in char *m_argv
int to_argv(char *tmp) {
	int index = 0;
	char ret[PARA_SIZE];
	bzero(ret, PARA_SIZE);
    if (strlen(tmp) >= PARA_SIZE){
        fs_print_std("Arguments length is too long!\n");
        return -1;
    }
	while(*tmp != '\0' && *tmp != '\n') {
		if (*tmp == ' ') {
			if (m_argv[index] == NULL) {
				m_argv[index] = (char *)malloc(sizeof(char)*strlen(ret)+1);
			}
        
			else {
				bzero(m_argv[index], strlen(m_argv[index]));
			}
			memcpy(m_argv[index], ret, strlen(ret)+1);
			bzero(ret, PARA_SIZE);
			index++;
		}
		else
            strncat(ret, tmp, 1);
		tmp++;
	}
	m_argv[index] = (char *)malloc(sizeof(char)*strlen(ret)+1);
	memcpy(m_argv[index], ret, strlen(ret)+1);
	return index + 1;
}

// sh batch process
int m_batch(char* input, char* output){
    FILE *fd_in;
    fd_in = fopen(input, "r");
    fd_out = fopen(output, "a");
    stdout_type = TOFILE;
    char *tmp = (char *)malloc(PARA_SIZE * sizeof(char));
    char *cmd = (char *)malloc(PARA_SIZE *  sizeof(char));
    while (fgets(tmp, sizeof(tmp)*PARA_SIZE, fd_in) != NULL){
        free_m_argv();
        int m_argc = to_argv(tmp);
        strncpy(cmd, m_argv[0], strlen(m_argv[0]));
        strncat(cmd, "\0", 1);
        m_cmd(m_argc, cmd);
        free_m_argv();
        bzero(cmd, PARA_SIZE);
    }
    fclose(fd_in);
    fclose(fd_out);
    stdout_type = TOSCREEN;
    return 0;
}

//
int main(int argc, char *argv[]){
    char c;
    char *tmp = (char *)malloc(PARA_SIZE * sizeof(char));
    char *cmd = (char *)malloc(PARA_SIZE *  sizeof(char));
    bzero(cmd, PARA_SIZE);
    bzero(tmp, PARA_SIZE);
    stdout_type = TOSCREEN;
    // NOTICE!!! Command "mkfs" MUST be called first
    fs_print_std("[FS Shell] ~> ");
    fflush(stdout);
    while(c != EOF){
        c = getchar();
        switch (c) {
            case '\n':
                if(tmp[0] == '\0'){
                    fs_print_std("[FS Shell] ~> ");
                }
                else{
                    int m_argc = to_argv(tmp);
                    strncpy(cmd, m_argv[0], strlen(m_argv[0]));
                    strncat(cmd, "\0", 1);
//                    fs_print_std(STAR);
//                    fs_print_std(" cmd : %s ", cmd);
//                    fs_print_std(STAR);
//                    fs_print_std("\n");
                    m_cmd(m_argc, cmd);
                    free_m_argv();
                    fs_print_std("[FS Shell] ~> ");
                    bzero(cmd, PARA_SIZE);
                }
                bzero(tmp, PARA_SIZE);
                break;
            default:
                strncat(tmp, &c, 1);
                break;
        }
    }
    free(tmp);
    free(cmd);
    fs_print_std("\n");
    fs_print_std("ALL THE COMMANDS ARE SUCCESSFULLY EXECUTED!\n");
    return 0;
}







#ifndef FS_shell_h
#define FS_shell_h

int m_mkfs();
int m_open(int, char**);
int m_read(int, char**);
int m_write(int, char**);
int m_seek(int, char**);
int m_close(int, char**);
int m_mkdir(int, char**);
int m_rmdir(int, char**);
int m_cd(int, char**);
int m_link(int, char**);
int m_unlink(int, char**);
int m_stat(int, char**);
int m_ls(int, char**);
int m_cat(int, char**);
int m_cp(int, char**);
int m_tree(int, char**);
int m_import(int, char**);
int m_export(int, char**);
int m_sh(int, char**);
int m_batch(char*, char*);
int to_argv(char *);
void m_cmd(int, char*);
void free_m_argv();

#endif

#ifndef __FS_H__
#define __FS_H__
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FSSIZE 10000000

unsigned char* fs;
unsigned char* fs_offset;
unsigned char* block_ptrs[10000];
unsigned char* cur_block;
struct inode* nodes;
struct inode* cur_node;
struct free_blocks* FBL;
struct superblock* super;
//struct superblock omniblock;
//struct free_blocks FL;
//struct inode inodes[100];

void mapfs(int fd);
void unmapfs();
void formatfs();
void loadfs();
void lsfs();
void printfs(struct inode* node, int rec_count);
void addfilefs(char* fname);
void add(struct inode* cnode, char* path);
void removefilefs(char* fname);
void rmv(struct inode* cnode, char* path, int last);
void extractfilefs(char* fname);
void extract(struct inode* cnode, char* path, int found_block);

struct superblock {
	int num_inode;
	int inode_size;
	int num_blocks;
	int block_size;
};

struct inode {
	//int inum;
	int is_dir;
	int blocks[100];
	int free;
	int size;
	//int parent_dir;
	//int parent_node;
};

struct free_blocks {
	int free_list[10000];
};

struct dir_entry {
	char* filename;
	int inode_num;
};

#endif

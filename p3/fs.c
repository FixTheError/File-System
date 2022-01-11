#include "fs.h"

void mapfs(int fd){
  if ((fs = mmap(NULL, FSSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL){
      perror("mmap failed");
      exit(EXIT_FAILURE);
  }
}


void unmapfs(){
  munmap(fs, FSSIZE);
}


void formatfs(){
	fs_offset = fs;
	super = (struct superblock*) fs;
	super[0].num_inode = 100;
	super[0].inode_size = sizeof(struct inode);
	super[0].num_blocks = 100 * (super->num_inode);
	super[0].block_size = 512;
	fs_offset += sizeof(struct superblock);
	FBL = (struct free_blocks*) fs_offset;
	for (int i = 0; i < super->num_blocks; i++) {
		FBL->free_list[i] = 1;
	}
	fs_offset += sizeof(struct free_blocks);
	nodes = (struct inode*) fs_offset;
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			nodes[i].blocks[j] = -1;

		}
		if (i == 0) {
			nodes[i].is_dir = 1;
			nodes[i].free = 0;
		}
		else {
			nodes[i].is_dir = 0;
			nodes[i].free = 1;
		}
		nodes[i].size = 0;
	}
	fs_offset += sizeof(struct inode) * 100;
}
//strtok

void loadfs(){
	fs_offset = fs;
	super = (struct superblock*) fs_offset;
	fs_offset += sizeof(struct superblock);
	FBL = (struct free_blocks*) fs_offset;
	fs_offset += sizeof(struct free_blocks);
	nodes = (struct inode*) fs_offset;
	fs_offset += sizeof(struct inode) * 100;
	//block_ptrs = fs_offset;
	for (int i = 0; i < super->num_blocks; i++) {
		block_ptrs[i] = fs_offset;
		//cur_block = fs_offset;
		fs_offset += super->block_size;
	}
	
}


void lsfs(){
	cur_node = &nodes[0];
	printfs(cur_node, 0);
}

void printfs(struct inode* node, int rec_count) {
	struct dir_entry* dir_ptr;
	struct inode* temp_node;
	for (int i = 0; i < 100; i++) {
		if (node->blocks[i] != -1) {
			if (FBL->free_list[node->blocks[i]] != 1) {
				dir_ptr = (struct dir_entry*) block_ptrs[node->blocks[i]];
				for (int j = 0; j < rec_count; j++) {
					printf("\t");
				}
				printf("%s\n", dir_ptr->filename);
				temp_node = &nodes[dir_ptr->inode_num];
				if (temp_node->is_dir == 1) {
					printfs(temp_node, (rec_count + 1));
				}
			}
		}
	}
}

void addfilefs(char* fname){
	cur_node = &nodes[0];
	add(cur_node, fname);
}

void add(struct inode* cnode, char* path) {
	char* tok = strtok(path, "/");
	/*if (tok == NULL) {
		return;
	}*/
	char* tok_next = path + 1 + (strlen(tok) * sizeof(char));
	int CBI;
	struct dir_entry new_entry;
	new_entry.filename = tok;
	int found = 0;
	int fd;
	struct stat* statbuf = malloc(sizeof(struct stat));

	unsigned char* buf = malloc(super->block_size);

	for (int i = 0; i < 100; i++) {	//find the file/directory with the same name as tok
		//cur_block = block_ptrs[cnode->blocks[i]];
		//struct dir_entry* ptr = (struct dir_entry*) cur_block;
		//char* fn = ptr->filename;
		if (cnode->blocks[i] != -1) {
			cur_block = block_ptrs[cnode->blocks[i]];
			struct dir_entry* ptr = (struct dir_entry*) cur_block;
			char* fn = ptr->filename;
			if (strcmp(fn, tok) == 0) {
				cnode = &nodes[ptr->inode_num];
				found = 1;
				break;
			}
		}
	}
	if (found == 0) {
		int j = 0;
		for (int i = 0; i < 100; i++) {
			if (cnode->blocks[i] == -1) {
				while(FBL->free_list[j] != 1) {
					j++;
				}
				CBI = j;
				cnode->blocks[i] = j;
				cur_block = block_ptrs[j];
				FBL->free_list[j] = 0;
				break;
			}
			else if (FBL->free_list[cnode->blocks[i]] == 1) {
				cur_block = block_ptrs[cnode->blocks[i]];
				break;
			}
		}
		for (int i = 0; i < 100; i++) {
			if (nodes[i].free == 1) {
				new_entry.inode_num = i;
				new_entry.filename = tok;
				strcpy((unsigned char*)block_ptrs[CBI], "");
				struct dir_entry* dir_ptr = (struct dir_entry*) block_ptrs[CBI];
				dir_ptr->inode_num = i;
				dir_ptr->filename = tok;
				cnode->size += super->block_size;
				cnode = &nodes[i]; 
				cnode->free = 0;
					if (strcmp(tok_next, "") != 0) {
						cnode->is_dir = 1;
					}
				break;
			}
		}
	}
	if (strcmp(tok_next, "") == 0) {
		if ((fd = open(tok, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
			perror("in addfs: open failed");
			exit(EXIT_FAILURE);
		}
		fstat(fd, statbuf);
		cnode->size = statbuf->st_size;
		int dumb = (cnode->size / super->block_size)+1;
		for (int i = 0; i < dumb; i++) {
			int j = 0;
			while (FBL->free_list[j] != 1) {
				j++;
			}
			cnode->blocks[i] = j;
			cur_block = block_ptrs[j];
			read(fd, buf, super->block_size);
			strcpy(block_ptrs[j], buf);
			FBL->free_list[j] = 0;
			//block_ptrs[j] = buf;
			if ((strlen(block_ptrs[j]) * sizeof(char) + 1) < super->block_size) {
				break;
			}
		}
	}
	else {
		add(cnode, tok_next);
	}
}

void removefilefs(char* fname){
	cur_node = &nodes[0];
	rmv(cur_node, fname, 0);
}

void rmv(struct inode* cnode, char* path, int last) {
	int found_block;
	int found_index;
	int found = 0;
	char* tok = strtok(path, "/");
	char* tok_next = path + 1 + (strlen(tok) * sizeof(char));
	for (int i = 0; i < 100; i++) {	//find the file/directory with the same name as tok
		if (cnode->blocks[i] != -1) {
			cur_block = block_ptrs[cnode->blocks[i]];
			struct dir_entry* ptr = (struct dir_entry*) cur_block;
			char* fn = ptr->filename;
			if (strcmp(fn, tok) == 0) {
				found_index = i;
				found_block = cnode->blocks[i];
				cnode->blocks[i] = -1;
				cnode = &nodes[ptr->inode_num];
				found = 1;
				break;
			}
		}
	}
	if (found == 1) {
		if (strcmp(tok_next, "") == 0) {
			last = 1;
			int temp = 0;
			while (temp < 100) {
				if (cnode->blocks[temp] != -1) {
					FBL->free_list[cnode->blocks[temp]] = 1;
					block_ptrs[cnode->blocks[temp]] = "";
					cnode->blocks[temp] = -1;
				}
				temp++;
			}
			cnode->free = 1;
			cnode->is_dir = 0;
			//cnode->blocks[found_index] = -1;
		}
		else {
			char* stemp = path + sizeof(tok);
			rmv(cnode, tok_next, (last + 1));
		}
		int tmp = 0;
		FBL->free_list[found_block] = 1;
		cnode->blocks[found_index] = -1;
		for (int i = 0; i < 100; i++) {
			if (cnode->blocks[i] == -1) {
				tmp++;
			}
			else if (FBL->free_list[cnode->blocks[i]] == 1) {
				tmp++;
			}
		}
		if (tmp > 99) {
			int temp = 0;
			while (temp < 100) {
				FBL->free_list[cnode->blocks[temp]] = 1;
				/*if (cnode->blocks[temp] != -1) {
					strcpy(block_ptrs[cnode->blocks[temp]], "");
				}*/
				cnode->blocks[temp] = -1;
				temp++;
			}
			cnode->free = 1;
			cnode->is_dir = 0;
			if (last == 0) {
				FBL->free_list[nodes[0].blocks[found_index]] = 1;
				nodes[0].blocks[found_index] = -1;
			}
		}
	}
	else {
		printf(" in rmv: %s was not found", tok);
	}
}

void extractfilefs(char* fname){
	cur_node = &nodes[0];
	extract(cur_node, fname, 0);
}

void extract(struct inode* cnode, char* path, int found_block) {
	int found = 0;
	char* tok = strtok(path, "/");
	char* tok_next = path + 1 + (strlen(tok) * sizeof(char));
	int fd;
	for (int i = 0; i < 100; i++) {	//find the file/directory with the same name as tok
		if (cnode->blocks[i] != -1) {
			if ((FBL->free_list[cnode->blocks[i]]) == 0) {
				cur_block = block_ptrs[cnode->blocks[i]];
				struct dir_entry* ptr = (struct dir_entry*) cur_block;
				char* fn = ptr->filename;
				if (strcmp(fn, tok) == 0) {
					found_block = cnode->blocks[i];
					cnode = &nodes[ptr->inode_num];
					found = 1;
					break;
				}
			}
		}
	}
	if (found == 1) {
		cur_block = block_ptrs[found_block];
		struct dir_entry* ptr = (struct dir_entry*) cur_block;
		char* fn = ptr->filename;
		if (strcmp(fn, tok) == 0 && strcmp(tok_next, "") == 0) {
			
			/*if ((fd = open(fn, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
				perror("in extract: open failed");
				exit(EXIT_FAILURE);
			}*/
			int dumb = (cnode->size / super->block_size) + 1;
			for (int i = 0; i < dumb; i++) {
				char* buf;
				strcpy(buf, block_ptrs[cnode->blocks[i]]);
				/*cur_block = block_ptrs[cnode->blocks[i]];
				buf = &cur_block;*/
				//write(fd, buf, super->block_size);
				printf("%s", buf);
				if ((strlen(buf) * sizeof(char) + 1) < super->block_size) {
					break;
				}
			}
		}
		else {
			char* stemp = path + sizeof(tok);
			extract(cnode, tok_next, found_block);
		}
	}
	else {
		printf("in extract: %s not found", tok);
	}
}
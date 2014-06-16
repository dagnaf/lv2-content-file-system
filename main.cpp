// main.cpp
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "disk.h"

FILE* fd;
char cmd[25];
char line[100];
char *tokens[10];
int num_tokens;
char cwd[50];
int uid;
int cuid;
char sample[BLOCK_SZ] = "sample text\n";
char buf[BLOCK_SZ];
int getCMD() {
	num_tokens = 1;
	fgets(line, 100, stdin);
	if ((tokens[0] = strtok(line, " \n\t")) == NULL) {
		return 1;
	}
	while ((tokens[num_tokens] = strtok(NULL, " \t\n")) != NULL) {
		++num_tokens;
	}
	return 0;
}
void install() {
	int i;
	printf("installing ...\n");
	fd = fopen(VDISK, "wb");
	if (fd == NULL) {
		printf("cannot install file system, restart program\n");
		exit(1);
	}
	superblock.init();
	fseek(fd, 0, SEEK_SET);
	fwrite(&superblock, BLOCK_SZ, 1, fd);
	mfd.init();
	fseek(fd, superblock.mfd, SEEK_SET);
	fwrite(&mfd, MFD_BLOCKS * BLOCK_SZ, 1, fd);
	ufd.init();
	fseek(fd, superblock.ufd, SEEK_SET);
	fwrite(&ufd, UFD_SZ, 1, fd);
	for (i = 0; i < INODE_BITMAP_WORD; ++i) bitmap_inode[i] = 0;
	fseek(fd, superblock.inode_bitmap, SEEK_SET);
	fwrite(bitmap_inode, INODE_BITMAP_BLOCKS * BLOCK_SZ, 1, fd);
	for (i = 0; i < BLOCK_BITMAP_WORD; ++i) bitmap_block[i] = 0;
	fseek(fd, superblock.block_bitmap, SEEK_SET);
	fwrite(bitmap_block, BLOCK_BITMAP_BLOCKS * BLOCK_SZ, 1, fd);
	fclose(fd);
}
void init() {
	printf("setting up ...\n");
	fd = fopen(VDISK, "rb+");
	if (fd == NULL) {
		printf("file system not installed, ready to install? <y/n> ");
		if (getCMD() == 0 && strcmp(tokens[0], "y") == 0) install();
		else {
			printf("file system must be installed first\n");
			exit(1);
		}
		fd = fopen(VDISK, "rb");
		if (fd == NULL) {
			printf("fail to open disk, restart file system\n");
			exit(1);
		}
	}
	fseek(fd, 0, SEEK_SET);
	fread(&superblock, sizeof(superblock), 1, fd);
	fseek(fd, superblock.mfd, SEEK_SET);
	fread(&mfd, MFD_SZ, 1, fd);
	fseek(fd, superblock.inode_bitmap, SEEK_SET);
	fread(bitmap_inode, INODE_BITMAP_SZ, 1, fd);
	fseek(fd, superblock.block_bitmap, SEEK_SET);
	fread(bitmap_block, BLOCK_BITMAP_SZ, 1, fd);
	fclose(fd);
	strcpy(cwd, "/root");
	printf("welcome\n");
	uid = -1; cuid = -1;
}
void infoSys() {
    int used;
    used = BLOCKS - superblock.block_cap;
    printf("system reserved blocks: %d\n", superblock.data/BLOCK_SZ);
    printf("blocks: %d/%d\n", used, BLOCKS - superblock.data/BLOCK_SZ);
    printf("block size: %d\n", BLOCK_SZ);
    used = MAX_USER*MAX_FILE - superblock.inode_cap;
    printf("inodes: %d/%d\n", used, MAX_USER*MAX_FILE);
    printf("users: %d\n", mfd.n);
}
void prompt() {
    if (uid != -1) printf("%s@local ", mfd.users[uid].name);
	printf("%s $ ", cwd);
}
void showDir() {
	int flag = 0;
	if (strcmp(cwd, "/root") == 0) {
		for (int i = 0; i < MAX_USER; ++i) {
			if (mfd.users[i].pt != -1) {
				flag = 1;
				printf("\t<dir>\t%s\n", mfd.users[i].name);
			}
		}
	} else {
		fd = fopen(VDISK, "rb");
		fseek(fd, superblock.ufd+UFD_SZ*cuid, SEEK_SET);
		fread(&ufd, UFD_SZ, 1, fd);
		for (int i = 0; i < MAX_FILE; ++i) {
			if (ufd.files[i].pt != -1) {
				flag = 1;
				printf("\t<file>\t%s - (inode %d)\n", ufd.files[i].name, ufd.files[i].pt);
			}
		}
	}
	if (!flag) {
		printf("empty\n");
	}
}
void login() {
	if (tokens[1] != NULL) {
		int new_uid = mfd.login(tokens[1]);
		if (new_uid >= 0) {
				uid = new_uid; cuid = uid;
				strcpy(cwd, "/root/");
				strcat(cwd, tokens[1]);
		} else {
			printf("login: username not found\n");
		}
	} else {
		printf("login: lonin username\n");
	}
}
int newInode() {
	for (int i = 0; i < MAX_USER*MAX_FILE; ++i) {
		int r = i / WORD_LEN;
		int c = i % WORD_LEN;
		if (((bitmap_inode[r]) & (1 << c)) == 0) {
			bitmap_inode[r] |= (1 << c);
			fd = fopen(VDISK, "rb+");
			// write superblock
			superblock.inode_cap--;
			fseek(fd, superblock.sb, SEEK_SET);
			fwrite(&superblock, BLOCK_SZ, 1, fd);
			// write inode_bitmap
			fseek(fd, superblock.inode_bitmap+r*sizeof(int), SEEK_SET);
			fwrite(&bitmap_inode[r], sizeof(int), 1, fd);
			// write inode
			Inode new_inode; new_inode.init(cuid, 0, 0);
			fseek(fd, superblock.inode+i*INODE_SZ, SEEK_SET);
			fwrite(&new_inode, INODE_SZ, 1, fd);
			fclose(fd);
			return i;
		}
	}
	return -1;
}
void createUser() {
	int new_uid;
	int e = mfd.add(&new_uid, tokens[2]);
	if (e == 0) {
		// write mfd
		fd = fopen(VDISK, "rb+");
		fseek(fd, superblock.mfd, SEEK_SET);
		fwrite(&mfd, MFD_BLOCKS * BLOCK_SZ, 1, fd);
		// write ufd
		UFD new_ufd; new_ufd.init();
		fseek(fd, superblock.ufd+new_uid*UFD_SZ, SEEK_SET);
		fwrite(&new_ufd, UFD_SZ, 1, fd);
		fclose(fd);
	} else if (e == e_dup) printf("create: username exists\n");
	else if (e == e_max) printf("create: user max\n");
}
void createFile() {
	UFD new_ufd;
	fd = fopen(VDISK, "rb+");
	fseek(fd, superblock.ufd+UFD_SZ*cuid, SEEK_SET);
	fread(&new_ufd, UFD_SZ, 1, fd);
	int fid;
	int e = new_ufd.add(&fid, tokens[1]);
	if (e == 0) {
		// get inode and write inode and inode bitmap
		new_ufd.files[fid].init((const char *)tokens[1], newInode());
		// write ufd
		fd = fopen(VDISK, "rb+");
		fseek(fd, superblock.ufd+cuid*UFD_SZ, SEEK_SET);
		fwrite(&new_ufd, UFD_SZ, 1, fd);
		fclose(fd);
	} else if (e == e_dup) printf("create: filename exists\n");
	else if (e == e_max) printf("create: file max\n");
}
void create() {
	if (tokens[1] != NULL) {
		if (strcmp(tokens[1], "-u") == 0) {
			if (tokens[2] == NULL) {
				printf("create: create -u username | filename\n");
			} else {
				if (uid != 0) printf("no permission\n");
					else createUser();
			}
		} else {
		    if (cuid == -1) printf("create: cannot create file at root\n");
			else createFile();
		}
	} else {
		printf("create: create -u username | filename\n");
	}
}
void change() {
	if (uid != 0) {
        printf("cd: no such directory\n");
	} else if (tokens[1] == NULL) {
		strcpy(cwd, "/root"); cuid = -1;
	} else {
		if (strcmp(tokens[1], "..") == 0) {
			strcpy(cwd, "/root"); cuid = -1;
		} else if (strcmp(tokens[1], ".") == 0) {
		} else {
			if (strcmp(cwd, "/root") == 0) {
				int new_uid = mfd.find(tokens[1]);
				if (new_uid >= 0) {
                    cuid = new_uid;
                    strcat(cwd, "/"); strcat(cwd, tokens[1]);
				}
				else printf("cd: no such directory\n");
			} else printf("cd: no such directory\n");
		}
	}
}
void deleteInode(int x) {
	int r = x / WORD_LEN;
	int c = x % WORD_LEN;
	bitmap_inode[r] &= ~(1 << c);
	fd = fopen(VDISK, "rb+");
	// write superblock
    superblock.inode_cap++;
    fseek(fd, superblock.sb, SEEK_SET);
    fwrite(&superblock, BLOCK_SZ, 1, fd);
    // write inode_bitmap
	fseek(fd, superblock.inode_bitmap+r*sizeof(int), SEEK_SET);
	fwrite(&bitmap_inode[r], sizeof(int), 1, fd);
	fclose(fd);
}
void deleteBlock(Inode new_inode) {
	int i, r, c, flag = 0;
	for (i = 0; i < new_inode.blocks; ++i) {
		int x = new_inode.addr[i];
		if (x != -1) {
			flag = 1;
			superblock.block_cap++;
			r = x / WORD_LEN; c = x % WORD_LEN;
			bitmap_block[r] &=	~(1 << c);
		} else { i = -1; break; }
	}
	new_inode.update(0);
	if (!flag) return;
	fd = fopen(VDISK, "rb+");
	fseek(fd, superblock.block_bitmap, SEEK_SET);
	fwrite(bitmap_block, BLOCK_BITMAP_BLOCKS * BLOCK_SZ, 1, fd);
	fclose(fd);
}
void delUser() {
	int new_uid;
	new_uid = mfd.find(tokens[2]);
	if (new_uid >= 0) {
		// write mfd
		UFD new_ufd;
		fd = fopen(VDISK, "rb+");
		fseek(fd, superblock.ufd+new_uid*UFD_SZ, SEEK_SET);
		fread(&new_ufd, UFD_SZ, 1, fd);
		if (new_ufd.n > 0) printf("delete: user directory not empty\n");
		else {
			mfd.del(new_uid);
			fseek(fd, superblock.mfd, SEEK_SET);
			fwrite(&mfd, MFD_BLOCKS * BLOCK_SZ, 1, fd);
			fclose(fd);
		}
	} else printf("delete: user not found\n");
}
void delFile() {
	UFD new_ufd;
	// read ufd
	fd = fopen(VDISK, "rb+");
	fseek(fd, superblock.ufd+UFD_SZ*cuid, SEEK_SET);
	fread(&new_ufd, UFD_SZ, 1, fd);
	fclose(fd);
	int fid = new_ufd.find(tokens[1]);
	if (fid >= 0) {
		int inode_id = new_ufd.del(fid);
		Inode new_inode;
		// read inode
		fd = fopen(VDISK, "rb+");
		fseek(fd, superblock.inode+INODE_SZ*inode_id, SEEK_SET);
		fread(&new_inode, INODE_SZ, 1, fd);
		fseek(fd, superblock.ufd+cuid*UFD_SZ, SEEK_SET);
		// write ufd
		fwrite(&new_ufd, UFD_SZ, 1, fd);
		fclose(fd);
		// write inode_bitmap
		deleteInode(inode_id);
		// write block_bitmap
		deleteBlock(new_inode);
	} else printf("delete: file not found\n");
}
void del() {
	if (tokens[1] != NULL) {
		if (strcmp(tokens[1], "-u") == 0) {
			if (tokens[2] == NULL) {
				printf("delete: delete -u username | filename\n");
			} else {
				if (uid != 0)
					printf("no permission\n");
				else if (strcmp(tokens[2], "admin") == 0)
					printf("delete: admin cannot be deleted\n");
				else delUser();
			}
		} else {
		    if (cuid == -1) printf("delete: cannot delete file at root\n");
		    else delFile();
		}
	} else {
		printf("create: create -u username | filename\n");
	}
}
void info() {
    if (tokens[1] == NULL) infoSys();
    else {
        UFD new_ufd;
        fd = fopen(VDISK, "rb+");
        fseek(fd, superblock.ufd+cuid*UFD_SZ, SEEK_SET);
        fread(&new_ufd, UFD_SZ, 1, fd);
        fclose(fd);
        int fid = new_ufd.find(tokens[1]);
        if (fid >= 0) {
            int inode_id = new_ufd.files[fid].pt;
            Inode new_inode;
            fd = fopen(VDISK, "rb+");
            fseek(fd, superblock.inode+inode_id*INODE_SZ, SEEK_SET);
            fread(&new_inode, INODE_SZ, 1, fd);
            fclose(fd);
            new_inode.print();
        } else printf("info: file not found\n");
    }
}
int newBlock(int id) {
    int st = superblock.data/BLOCK_SZ;
    for (int i = (id+1) % BLOCKS; i != id; i = (i+1) % BLOCKS) {
        if (i < st) i = st;
        int r = i / WORD_LEN;
        int c = i % WORD_LEN;
        if ((bitmap_block[r] & (1 << c)) == 0) {
            bitmap_block[r] |= (1 << c);
            superblock.block_cap--;
            return i;
        }
    }
    return -1;
}
void cp() {
    if (cuid == -1) printf("copy: cannot copy file at root\n");
    else if (tokens[1] == NULL || tokens[2] == NULL) {
        printf("copy: copy copyfilename srcfilename eg copy 1.txt 2.txt\n");
    } else {
        // read directory
        UFD new_ufd;
        fd = fopen(VDISK, "rb+");
        fseek(fd, superblock.ufd+cuid*UFD_SZ, SEEK_SET);
        fread(&new_ufd, UFD_SZ, 1, fd);
        fclose(fd);
        int fid_dest = new_ufd.find(tokens[1]);
        if (fid_dest >= 0) {
            printf("copy: copy file:%s already exists\n", tokens[1]);
            return;
        }
		//		int inode_id_dest = new_ufd.files[fid_dest].pt;

        int fid_src = new_ufd.find(tokens[2]);
        if (fid_src < 0) {
            printf("copy: source file:%s not found\n", tokens[2]);
            return;
        }
		int inode_id_src = new_ufd.files[fid_src].pt;
        // read inode
        fd = fopen(VDISK, "rb+");
        Inode new_inode_src;
        fseek(fd, superblock.inode+inode_id_src*INODE_SZ, SEEK_SET);
        fread(&new_inode_src, INODE_SZ, 1, fd);
        fclose(fd);
        if (new_inode_src.sz > superblock.block_cap) {
            printf("copy: blocks not enough\n");
            return;
        }
        printf("creating copyfile ...\n");
        createFile();
        // update ufd
        fd = fopen(VDISK, "rb+");
        fseek(fd, superblock.ufd+cuid*UFD_SZ, SEEK_SET);
        fread(&new_ufd, UFD_SZ, 1, fd);
        // read inode_dest
        fid_dest = new_ufd.find(tokens[1]);
        int inode_id_dest = new_ufd.files[fid_dest].pt;
        Inode new_inode_dest;
        fseek(fd, superblock.inode+inode_id_dest*INODE_SZ, SEEK_SET);
        fread(&new_inode_dest, INODE_SZ, 1, fd);
        // read content
        fd = fopen(VDISK, "rb+");
        int i; int block_id = 0;
        for (i = 0; i < new_inode_src.blocks ; ++i) {
            block_id = newBlock(block_id);
            new_inode_dest.addr[i] = block_id;
            fseek(fd, new_inode_src.addr[i]*BLOCK_SZ, SEEK_SET);
            fread(buf, BLOCK_SZ, 1, fd);
            fseek(fd, new_inode_dest.addr[i]*BLOCK_SZ, SEEK_SET);
            fwrite(buf, BLOCK_SZ, 1, fd);
        }
        new_inode_dest.addr[i] = -1;
        new_inode_dest.update(new_inode_src.blocks);
        fseek(fd, superblock.block_bitmap, SEEK_SET);
        fwrite(bitmap_block, BLOCK_BITMAP_BLOCKS*BLOCK_SZ, 1, fd);
        fseek(fd, superblock.inode+inode_id_dest*INODE_SZ, SEEK_SET);
        fwrite(&new_inode_dest, INODE_SZ, 1, fd);
        fseek(fd, 0, SEEK_SET);
        fwrite(&superblock, BLOCK_SZ, 1, fd);
        fclose(fd);
    }
}
void test(int block_id) {
    fseek(fd, block_id*BLOCK_SZ, SEEK_SET);
    memset(buf, 0, sizeof(buf));
    fread(buf, BLOCK_SZ, 1, fd);
    puts(buf);
}
void write() {
    if (tokens[1] == NULL || tokens[2] == NULL) {
        printf("write: write filename blocks eg write 1.txt 10\n");
    } else {
        int sz = -1;
        sscanf(tokens[2], "%d", &sz);
        if (sz == -1 || (sz <= 10 && superblock.block_cap < sz) || sz > 10) {
            printf("write: blocks not enough or lv1+ allocation not implemented\n");
            return;
        }
        UFD new_ufd;
        fd = fopen(VDISK, "rb+");
        fseek(fd, superblock.ufd+cuid*UFD_SZ, SEEK_SET);
        fread(&new_ufd, UFD_SZ, 1, fd);
        fclose(fd);
        int fid = new_ufd.find(tokens[1]);
        if (fid < 0) {
            printf("write: file not found\n");
            return;
        }
        int inode_id = new_ufd.files[fid].pt;
        fd = fopen(VDISK, "rb+");
        int block_id = 0;
        int i; Inode new_inode;
        fseek(fd, superblock.inode+inode_id*INODE_SZ, SEEK_SET);
        fread(&new_inode, INODE_SZ, 1, fd);
        if (sz < new_inode.sz) deleteBlock(new_inode);
        for (i = 0; i < sz ; ++i) {
            block_id = newBlock(block_id);
            new_inode.addr[i] = block_id;
            fseek(fd, block_id*BLOCK_SZ, SEEK_SET);
            fwrite(sample, BLOCK_SZ, 1, fd);
        }
        new_inode.addr[i] = -1;
        new_inode.update(sz);
        fseek(fd, superblock.block_bitmap, SEEK_SET);
        fwrite(bitmap_block, BLOCK_BITMAP_BLOCKS*BLOCK_SZ, 1, fd);
        fseek(fd, superblock.inode+inode_id*INODE_SZ, SEEK_SET);
        fwrite(&new_inode, INODE_SZ, 1, fd);
        fseek(fd, 0, SEEK_SET);
        fwrite(&superblock, BLOCK_SZ, 1, fd);
        fclose(fd);
    }
}
void read() {
    if (tokens[1] == NULL) {
        printf("read: read filename\n");
    } else {
        UFD new_ufd;
        fd = fopen(VDISK, "rb+");
        fseek(fd, superblock.ufd+cuid*UFD_SZ, SEEK_SET);
        fread(&new_ufd, UFD_SZ, 1, fd);
        fclose(fd);
        int fid = new_ufd.find(tokens[1]);
        if (fid < 0) {
            printf("read: file not found\n");
            return;
        }
        int inode_id = new_ufd.files[fid].pt;
        fd = fopen(VDISK, "rb+");
        int i; Inode new_inode;
        fseek(fd, superblock.inode+inode_id*INODE_SZ, SEEK_SET);
        fread(&new_inode, INODE_SZ, 1, fd);
        for (i = 0; i < new_inode.blocks ; ++i) {
            memset(buf, 0, sizeof(buf));
            fseek(fd, new_inode.addr[i] * BLOCK_SZ, SEEK_SET);
            fread(buf, BLOCK_SZ, 1, fd);
            printf("%s", buf);
        }
        fclose(fd);
    }
}
void cmdHandler() {
	if (strcmp(tokens[0], "login") == 0) {
		login();
	} else if (uid == -1) {
		if (strcmp(tokens[0], "dir") == 0) showDir();
		else printf("no permission\n");
	} else if (strcmp(tokens[0], "dir") == 0) {
		showDir();
	} else if (strcmp(tokens[0], "create") == 0) {
		create();
	} else if (strcmp(tokens[0], "delete") == 0) {
		del();
	} else if (strcmp(tokens[0], "cd") == 0) {
		change();
	} else if (strcmp(tokens[0], "reinstall") == 0) {
		if (uid == 0) install();
		else printf("no permission\n");
	} else if (strcmp(tokens[0], "logout") == 0) {
		uid = -1; cuid = -1;
		strcpy(cwd, "/root");
	} else if (strcmp(tokens[0], "write") == 0) {
	    write();
	} else if (strcmp(tokens[0], "read") == 0) {
	    read();
	} else if (strcmp(tokens[0], "copy") == 0) {
	    cp();
	} else if (strcmp(tokens[0], "info") == 0) {
	    info();
	} else printf("cmd: no such command\n");
}
int main(int argc, char const *argv[]) {
	init();
	while (1) {
		prompt();
		if (getCMD() == 1) {
			continue;
		}
		if (strcmp(tokens[0], "exit") == 0) break;
		else {
			cmdHandler();
		}
	}
	printf("goodbye\n");
	return 0;
}

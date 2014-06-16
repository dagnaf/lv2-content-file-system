#ifndef DISK_H_INCLUDED
#define DISK_H_INCLUDED


#define BLOCKS 102400
#define BLOCK_SZ 1024
#define WORD_LEN 32
#define MAX_USER 8
#define MAX_FILE 256
#define VDISK "vdisk"
#define MFD_SZ sizeof(MFD)
#define UFD_SZ sizeof(UFD)
#define INODE_SZ sizeof(Inode)
// #define INODE_BITMAP_WORD (MAX_USER*MAX_FILE*INODE_SZ/BLOCK_SZ/WORD_LEN+1)
#define INODE_BITMAP_WORD (MAX_USER*MAX_FILE/WORD_LEN+1)
#define BLOCK_BITMAP_WORD (BLOCKS/WORD_LEN+1)
#define BLOCK_BITMAP_SZ (sizeof(bitmap_block))
#define INODE_BITMAP_SZ (sizeof(bitmap_inode))

#define MFD_BLOCKS (MFD_SZ/BLOCK_SZ+1)
#define UFD_BLOCKS (UFD_SZ*MAX_USER/BLOCK_SZ+1)
#define INODE_BLOCKS (INODE_SZ*MAX_USER*MAX_FILE/BLOCK_SZ+1)
#define BLOCK_BITMAP_BLOCKS (BLOCK_BITMAP_SZ/BLOCK_SZ+1)
#define INODE_BITMAP_BLOCKS (INODE_BITMAP_SZ/BLOCK_SZ+1)

#include <string.h>
#include <time.h>

enum err{e_dup = 1, e_len, e_io, e_max, e_not};
struct File {
	char name[14];
	int pt;
	void init(const char *s, int d) {
		strcpy(name, s);
		pt = d;
	}
};
int bitmap_block[BLOCK_BITMAP_WORD];
int bitmap_inode[INODE_BITMAP_WORD];

struct UFD
{
    int n;
	File files[MAX_FILE];
	void init() {
		int i;
		for (i = 0; i < MAX_FILE; ++i) files[i].init("", -1);
		n = 0;
	}
	int add(int* fid, char *s) {
        if (n == MAX_FILE) return e_max;
        if (find(s) >= 0) return e_dup;
        for (int i = 0; i < MAX_FILE; ++i) {
            if (files[i].pt == -1) {
                *fid = i; n++;
                return 0;
            }
        }
        return e_not;
	}
	int find(char *s) {
	    for (int i = 0; i < MAX_FILE; ++i) {
            if (files[i].pt != -1 && strcmp(files[i].name, s) == 0) {
                return i;
            }
	    }
	    return -1;
	}
	int del(int fid) {
	    int tmp = files[fid].pt;
        files[fid].pt = -1; n--;
        return tmp;
	}
} ufd;

struct MFD {
    int n;
	File users[MAX_USER];
	void init() {
		for (int i = 0; i < MAX_USER; ++i) users[i].pt = -1;
		users[0].init("admin", 0);
		n = 1;
	}
    int add(int *uid, char *s) {
        if (n == MAX_USER) return e_max;
        if (find(s) >= 0) return e_dup;
        for (int i = 0; i < MAX_USER; ++i) {
            if (users[i].pt == -1) {
                *uid = i; n++;
                users[i].init((const char *)s, i);
                return 0;
            }
        }
        return e_not;
    }
    int find(char *s) {
		for (int i = 0; i < MAX_USER; ++i) {
			if (users[i].pt != -1 && strcmp(s, users[i].name) == 0) {
				return i;
			}
		}
		return -1;
    }
    int login(char *s) { return find(s); }
    int del(int uid) {
        int tmp = users[uid].pt;
        users[uid].pt = -1; n--;
        return tmp;
    }
} mfd;

struct Inode
{
	int uid;
	int type;
	int permission;
	int blocks;
	int sz;
	int link;
	time_t create_time;
	time_t modify_time;
	int addr[13];
	void init(int i, int t, int p) {
	    uid = i; type = t; permission = p;
	    blocks = 0;
	    sz = 0;
	    link = 0;
	    time(&create_time);
	    time(&modify_time);
	    memset(addr, -1, sizeof(addr));
	}
	void print() {
	    printf("user id: %d\n", uid);
	    printf("type: %d\n", type);
	    printf("permission: %d\n", permission);
	    printf("blocks: %d\n", blocks);
	    printf("size: %d\n", sz);
	    printf("link: %d\n", link);
	    printf("create time: %s", ctime(&create_time));
	    printf("modify time: %s", ctime(&modify_time));
	}
	void update(int s) {
	    sz = s; blocks = s; time(&modify_time);
	    for (int i = s; i < 10; ++i) addr[i] = -1;
	}
};


struct SuperBlock {
    int block_cap;
    int inode_cap;
	int sb; // position for fseek
	int mfd;
	int ufd;
	int inode;
	int inode_bitmap;
	int block_bitmap;
	int data;
	void init() {
	    block_cap = BLOCKS;
	    inode_cap = MAX_FILE*MAX_USER;
		sb = 0;
		mfd = BLOCK_SZ;
		ufd = mfd + BLOCK_SZ * MFD_BLOCKS;
		inode = ufd + BLOCK_SZ * UFD_BLOCKS;
		inode_bitmap = inode + BLOCK_SZ * INODE_BLOCKS;
		block_bitmap = inode_bitmap + BLOCK_SZ * INODE_BITMAP_BLOCKS;
		data = block_bitmap + BLOCK_SZ * BLOCK_BITMAP_BLOCKS;
//		printf("test : %d %d\n", sizeof(block_bitmap), BLOCK);
	}
	void print() {
		printf("%d %d %d %d %d %d %d\n", sb, mfd, ufd, inode, inode_bitmap, block_bitmap, data);
	}
} superblock;



#endif // DISK_H_INCLUDED

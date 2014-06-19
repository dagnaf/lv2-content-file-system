#ifndef DISK_H_INCLUDED
#define DISK_H_INCLUDED


#define BLOCKS 102400       // �����ܵ��̿���
#define BLOCK_SZ 1024       // �̿��С1024kb
#define WORD_LEN 32         // λʾͼ�ֳ�32λ
#define MAX_USER 8          // ����û���
#define MAX_FILE 256        // ���û�����ļ���
#define VDISK "vdisk"       // ��������ļ���
#define MFD_SZ sizeof(MFD)  // mfd��ռkb��
#define UFD_SZ sizeof(UFD)  // һ��ufd��ռkb��
#define INODE_SZ sizeof(Inode)  // һ��i�ڵ���ռkb��
#define INODE_BITMAP_WORD (MAX_USER*MAX_FILE/WORD_LEN+1) // i�ڵ�λʾͼ���õ�������
#define BLOCK_BITMAP_WORD (BLOCKS/WORD_LEN+1) // �̿�λʾͼ���õ�������
#define BLOCK_BITMAP_SZ (sizeof(bitmap_block)) // �̿�λʾͼ��ռkb��
#define INODE_BITMAP_SZ (sizeof(bitmap_inode)) // i�ڵ�λʾͼ��ռkb��

#define MFD_BLOCKS (MFD_SZ/BLOCK_SZ+1)          // MFD��ռ�̿���
#define UFD_BLOCKS (UFD_SZ*MAX_USER/BLOCK_SZ+1) // ����UFD��ռ�̿���
#define INODE_BLOCKS (INODE_SZ*MAX_USER*MAX_FILE/BLOCK_SZ+1) // ����i�ڵ���ռ�̿���
#define BLOCK_BITMAP_BLOCKS (BLOCK_BITMAP_SZ/BLOCK_SZ+1)     // �̿�λʾͼ��ռ�̿���
#define INODE_BITMAP_BLOCKS (INODE_BITMAP_SZ/BLOCK_SZ+1)     // i�ڵ�λʾͼ��ռ�̿���

#include <cstdio> // ���� FILE ����
#include "superblock.h" // �ֱ�����������ݽṹ�Ķ��壬������
#include "mfd.h" // ��Ŀ¼��
#include "ufd.h" // �û�Ŀ¼��
#include "inode.h" // i�ڵ�

extern FILE* fd; // ȫ�ֱ������ļ���
extern SuperBlock superblock; // ������
extern MFD mfd; // ��Ŀ¼��
extern int bitmap_block[BLOCK_BITMAP_WORD]; // �̿�λʾͼ
extern int bitmap_inode[INODE_BITMAP_WORD]; // i�ڵ�λʾͼ
// �⼸�������ڳ����й̶�

// ������Ϣö�������ظ���ͻ�����ȣ���д������޶ȣ������ڴ��� ��������
enum err{e_dup = 1, e_len, e_io, e_max, e_not, e_invalid};

// ��ȡ������obj����ȡsz kb���Ӵ����ļ���offsetλ�ÿ�ʼ���������Ժ��Ƿ�ر�ȡ����cl��1Ϊ�ر�
// ��ȡ��ģʽĬ��Ϊ���Ƕ�������
void iread(void*obj, int sz, long offset, int cl = 1, const char *md = "rb+");
// ͬ��
void iwrite(void*, int, long, int cl = 1, const char *md = "rb+");
// �ر��ļ�����fd=NULL
void iclose();
// ��ȡһ��������ָ������ʷ���tokens
int getCMD();
// ������src��ʾ���ô���ĺ�������arg��ʾ���ܺ��еĲ���
void errHandler(int e_type, const char *src, char *arg);
// ��װϵͳ����Ҫ��������д������ļ�
void install();
// ��ʼ��ϵͳ����Ҫ��ȡ�����ļ��е�1�����飬2��Ŀ¼����3λʾͼ
void init();
// ��ʾϵͳ��Ϣ
void infoSys();
// ��ʾ��ǰ·��
void prompt();
// ��ʾ��ǰ�ļ����µ��ļ�
void showDir();
// ��½
void login();
// ����յ�i�ڵ�
int newInode();
// �����û�
void createUser();
// �����ļ�
void createFile();
// ��������
void create();
// cd����
void change();
// ɾ�����ͷž����i�ڵ㣬����Ϊi�ڵ���
void deleteInode(int x);
// ɾ�����ͷ�i�ڵ���ռ�õ��̿�
void deleteBlock(Inode inode);
// ɾ���û�
void delUser();
// ɾ���ļ�
void delFile();
// ɾ������
void del();
// ��ʾ��Ϣ
void info();
// ����յ��̿飬����Ϊ��һ���������õ����̿�id
int newBlock(int id);
// ��������
void cp();
// �����ã���Ч
void test(int block_id);
// д����
void write();
// ������
void read();
// �������
void cmdHandler();
// �뿪�˳�
int leave();

#endif // DISK_H_INCLUDED

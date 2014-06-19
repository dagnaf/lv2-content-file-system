#ifndef DISK_H_INCLUDED
#define DISK_H_INCLUDED


#define BLOCKS 102400       // 磁盘总的盘块数
#define BLOCK_SZ 1024       // 盘块大小1024kb
#define WORD_LEN 32         // 位示图字长32位
#define MAX_USER 8          // 最大用户数
#define MAX_FILE 256        // 单用户最大文件数
#define VDISK "vdisk"       // 虚拟磁盘文件名
#define MFD_SZ sizeof(MFD)  // mfd所占kb数
#define UFD_SZ sizeof(UFD)  // 一个ufd所占kb数
#define INODE_SZ sizeof(Inode)  // 一个i节点所占kb数
#define INODE_BITMAP_WORD (MAX_USER*MAX_FILE/WORD_LEN+1) // i节点位示图所用到的字数
#define BLOCK_BITMAP_WORD (BLOCKS/WORD_LEN+1) // 盘块位示图所用到的字数
#define BLOCK_BITMAP_SZ (sizeof(bitmap_block)) // 盘块位示图所占kb数
#define INODE_BITMAP_SZ (sizeof(bitmap_inode)) // i节点位示图所占kb数

#define MFD_BLOCKS (MFD_SZ/BLOCK_SZ+1)          // MFD所占盘块数
#define UFD_BLOCKS (UFD_SZ*MAX_USER/BLOCK_SZ+1) // 所有UFD所占盘块数
#define INODE_BLOCKS (INODE_SZ*MAX_USER*MAX_FILE/BLOCK_SZ+1) // 所有i节点所占盘块数
#define BLOCK_BITMAP_BLOCKS (BLOCK_BITMAP_SZ/BLOCK_SZ+1)     // 盘块位示图所占盘块数
#define INODE_BITMAP_BLOCKS (INODE_BITMAP_SZ/BLOCK_SZ+1)     // i节点位示图所占盘块数

#include <cstdio> // 包含 FILE 类型
#include "superblock.h" // 分别包含各种数据结构的定义，超级块
#include "mfd.h" // 根目录区
#include "ufd.h" // 用户目录区
#include "inode.h" // i节点

extern FILE* fd; // 全局变量，文件符
extern SuperBlock superblock; // 超级块
extern MFD mfd; // 根目录区
extern int bitmap_block[BLOCK_BITMAP_WORD]; // 盘块位示图
extern int bitmap_inode[INODE_BITMAP_WORD]; // i节点位示图
// 这几个变量在程序中固定

// 错误信息枚举量：重复冲突，长度，读写，最大限度，不存在错误， 其他错误
enum err{e_dup = 1, e_len, e_io, e_max, e_not, e_invalid};

// 读取磁盘至obj，读取sz kb，从磁盘文件的offset位置开始读，读完以后是否关闭取决于cl，1为关闭
// 读取的模式默认为覆盖读二进制
void iread(void*obj, int sz, long offset, int cl = 1, const char *md = "rb+");
// 同上
void iwrite(void*, int, long, int cl = 1, const char *md = "rb+");
// 关闭文件，置fd=NULL
void iclose();
// 获取一行命令，并分隔出单词放入tokens
int getCMD();
// 错误处理，src表示调用错误的函数名，arg表示可能含有的参数
void errHandler(int e_type, const char *src, char *arg);
// 安装系统，主要将超级块写入磁盘文件
void install();
// 初始化系统，主要读取磁盘文件中的1超级块，2根目录区，3位示图
void init();
// 显示系统信息
void infoSys();
// 显示当前路径
void prompt();
// 显示当前文件夹下的文件
void showDir();
// 登陆
void login();
// 申请空的i节点
int newInode();
// 创建用户
void createUser();
// 创建文件
void createFile();
// 创建命令
void create();
// cd命令
void change();
// 删除·释放具体的i节点，参数为i节点编号
void deleteInode(int x);
// 删除·释放i节点所占用的盘块
void deleteBlock(Inode inode);
// 删除用户
void delUser();
// 删除文件
void delFile();
// 删除命令
void del();
// 显示信息
void info();
// 申请空的盘块，参数为上一次申请所得到的盘块id
int newBlock(int id);
// 复制命令
void cp();
// 测试用，无效
void test(int block_id);
// 写命令
void write();
// 读命令
void read();
// 命令分析
void cmdHandler();
// 离开退出
int leave();

#endif // DISK_H_INCLUDED

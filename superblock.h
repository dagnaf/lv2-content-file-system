#ifndef SUPERBLOCK_H_INCLUDED
#define SUPERBLOCK_H_INCLUDED

struct SuperBlock {
  int block_cap; // 剩余盘块
  int inode_cap; // 剩余i节点
  int sb; // 各个区域在磁盘中的首地址
  int mfd;
  int ufd;
  int inode;
  int inode_bitmap;
  int block_bitmap;
  int data;
  void init(); // 初始化上述变量
};

#endif // SUPERBLOCK_H_INCLUDED

#ifndef SUPERBLOCK_H_INCLUDED
#define SUPERBLOCK_H_INCLUDED

struct SuperBlock {
  int block_cap; // ʣ���̿�
  int inode_cap; // ʣ��i�ڵ�
  int sb; // ���������ڴ����е��׵�ַ
  int mfd;
  int ufd;
  int inode;
  int inode_bitmap;
  int block_bitmap;
  int data;
  void init(); // ��ʼ����������
};

#endif // SUPERBLOCK_H_INCLUDED

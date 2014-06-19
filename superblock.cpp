#include "disk.h"

void SuperBlock::init() {
  // ʣ���̿�
  block_cap = BLOCKS;
  // ʣ��i�ڵ�
  inode_cap = MAX_FILE*MAX_USER;
  // ������ĵ�ַ
  sb = 0;
  // mfd�ĵ�ַ
  mfd = BLOCK_SZ;
  // ufd���׵�ַ
  ufd = mfd + BLOCK_SZ * MFD_BLOCKS;
  // i�ڵ������׵�ַ
  inode = ufd + BLOCK_SZ * UFD_BLOCKS;
  // λʾͼ�׵�ַ
  inode_bitmap = inode + BLOCK_SZ * INODE_BLOCKS;
  block_bitmap = inode_bitmap + BLOCK_SZ * INODE_BITMAP_BLOCKS;
  // �������׵�ַ
  data = block_bitmap + BLOCK_SZ * BLOCK_BITMAP_BLOCKS;
}

#include "disk.h"

void SuperBlock::init() {
  // 剩余盘块
  block_cap = BLOCKS;
  // 剩余i节点
  inode_cap = MAX_FILE*MAX_USER;
  // 超级块的地址
  sb = 0;
  // mfd的地址
  mfd = BLOCK_SZ;
  // ufd的首地址
  ufd = mfd + BLOCK_SZ * MFD_BLOCKS;
  // i节点区的首地址
  inode = ufd + BLOCK_SZ * UFD_BLOCKS;
  // 位示图首地址
  inode_bitmap = inode + BLOCK_SZ * INODE_BLOCKS;
  block_bitmap = inode_bitmap + BLOCK_SZ * INODE_BITMAP_BLOCKS;
  // 数据区首地址
  data = block_bitmap + BLOCK_SZ * BLOCK_BITMAP_BLOCKS;
}

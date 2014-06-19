#include <cstring>
#include "disk.h"

// 初始化当前文件目录，清空所有文件
void UFD::init() {
  n = 0;
  for (int i = 0; i < MAX_FILE; ++i) files[i].init("", -1);
}
// 添加文件：
// 参数1：文件编号指针，添加成功则设为新的编号
// 参数2：添加的文件名
// 返回：错误信息
int UFD::add(char *s, int type, int permission) {
  if (n == MAX_FILE) return e_max; // 超过最大文件数
  if (find(s) >= 0) return e_dup;  // 文件名重复
  int new_inode_id = newInode();
  files[newId()].init((const char *)s, new_inode_id);
  Inode new_inode(new_inode_id, type, permission);
  iwrite(&new_inode, INODE_SZ, superblock.inode+new_inode_id*INODE_SZ);
  n++;
  return 0;
}
// 申请一个空的文件id
int UFD::newId() {
  for (int i = 0; i < MAX_FILE; ++i)
    if (files[i].pt == -1)
      return i;
  return -1;
}
// 查找文件：
// 参数：文件名
// 返回： -1未找到，>0文件编号
int UFD::find(char *s) {
  for (int i = 0; i < MAX_FILE; ++i)
    if (files[i].pt != -1 && strcmp(files[i].name, s) == 0)
      return i;
  return -1;
}
// 删除文件：
// 参数：文件编号
// 同时删除对应的i节点，以及i节点中addr所指向的盘块
int UFD::del(char *s) {
  int fid = find(s);
  if (fid == -1) return e_not;
  int inode_id = files[fid].pt;
  files[fid].pt = -1;
  Inode inode;
  iread(&inode, INODE_SZ, superblock.inode+INODE_SZ*inode_id);
  deleteInode(inode_id);
  deleteBlock(inode);
  n--;
  return 0;
}

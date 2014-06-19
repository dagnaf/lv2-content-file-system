#ifndef INODE_H_INCLUDED
#define INODE_H_INCLUDED

#include <ctime>

struct Inode
{
  int uid; // 所属用户 id
  int type; // 文件类型
  int permission; // 文件许可
  int blocks; // 所占盘块
  int sz; // 文件大小
  int link; // 文件链接数
  time_t create_time; // 创建时间
  time_t modify_time; // 修改时间
  int addr[13]; // 所占盘块号，0-9为直接索引，10-12为间接索引（未实现）
  Inode(); // 空初始
  Inode(int i, int t, int p); // 初始化 用户id、类型、许可权
  void print(); // 打印
  void update(int s); // 更新大小
};

#endif // INODE_H_INCLUDED

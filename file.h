#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

// 文件结构：作为 用户空间文件 或 用户空间名 使用
// eg username,ufd0 或者 filename,inode0
struct File {
  char name[14]; // 文件名或用户名
  int pt; // 指向i节点或ufd
  // 初始化名字和指针
  void init(const char *s, int d);
};

#endif // FILE_H_INCLUDED

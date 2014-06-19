#ifndef FILEDIRECTORY_H_INCLUDED
#define FILEDIRECTORY_H_INCLUDED

// 目录接口
struct FileDirectory {
  virtual void init() = 0; // 初始化
  // virtual int add(char *) = 0; // 不提倡
  virtual int find(char *) = 0; // 查找
  virtual int del(char *) = 0; // 删除
  virtual int newId() = 0;
};

#endif // FILEDIRECTORY_H_INCLUDED

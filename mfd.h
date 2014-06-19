#ifndef MFD_H_INCLUDED
#define MFD_H_INCLUDED

#include "filedirectory.h"
#include "file.h"
// 实现文件目录功能
// 具体和UFD类似
struct MFD : FileDirectory
{
  int n; // 当前用户数
  File users[MAX_USER]; // 用户名和用户id
  void init();
  int add(char *s);
  int newId();
  int find(char *s);
  int login(char *s);
  int del(char *s);
};

#endif // MFD_H_INCLUDED

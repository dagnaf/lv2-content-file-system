#ifndef UFD_H_INCLUDED
#define UFD_H_INCLUDED

#include "filedirectory.h"
#include "file.h"

// 用户空间目录：目录记录该用户空间中的文件
struct UFD : FileDirectory
{
  int n; // 文件总数
  File files[MAX_FILE]; // 文件，一个用户最多MAX_FILE个文件，具体表示文件名和文件id
  // 初始化目录：清空
  void init();
  // 添加文件：
  // 参数1：添加的文件名，参数2：文件类型，参数3：文件权限
  // 返回：错误信息
  int add(char *s, int type = 0, int permission = 0);
  // 申请一个空的文件id
  int newId();
  // 查找文件：
  // 参数：文件名
  // 返回： -1未找到，>0文件编号
  int find(char *s);
  // 删除文件：
  // 参数：文件名
  int del(char *s);
};

#endif // UFD_H_INCLUDED

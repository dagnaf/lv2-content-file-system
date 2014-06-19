#ifndef MFD_H_INCLUDED
#define MFD_H_INCLUDED

#include "filedirectory.h"
#include "file.h"
// ʵ���ļ�Ŀ¼����
// �����UFD����
struct MFD : FileDirectory
{
  int n; // ��ǰ�û���
  File users[MAX_USER]; // �û������û�id
  void init();
  int add(char *s);
  int newId();
  int find(char *s);
  int login(char *s);
  int del(char *s);
};

#endif // MFD_H_INCLUDED

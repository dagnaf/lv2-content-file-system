#ifndef UFD_H_INCLUDED
#define UFD_H_INCLUDED

#include "filedirectory.h"
#include "file.h"

// �û��ռ�Ŀ¼��Ŀ¼��¼���û��ռ��е��ļ�
struct UFD : FileDirectory
{
  int n; // �ļ�����
  File files[MAX_FILE]; // �ļ���һ���û����MAX_FILE���ļ��������ʾ�ļ������ļ�id
  // ��ʼ��Ŀ¼�����
  void init();
  // ����ļ���
  // ����1����ӵ��ļ���������2���ļ����ͣ�����3���ļ�Ȩ��
  // ���أ�������Ϣ
  int add(char *s, int type = 0, int permission = 0);
  // ����һ���յ��ļ�id
  int newId();
  // �����ļ���
  // �������ļ���
  // ���أ� -1δ�ҵ���>0�ļ����
  int find(char *s);
  // ɾ���ļ���
  // �������ļ���
  int del(char *s);
};

#endif // UFD_H_INCLUDED

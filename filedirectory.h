#ifndef FILEDIRECTORY_H_INCLUDED
#define FILEDIRECTORY_H_INCLUDED

// Ŀ¼�ӿ�
struct FileDirectory {
  virtual void init() = 0; // ��ʼ��
  // virtual int add(char *) = 0; // ���ᳫ
  virtual int find(char *) = 0; // ����
  virtual int del(char *) = 0; // ɾ��
  virtual int newId() = 0;
};

#endif // FILEDIRECTORY_H_INCLUDED

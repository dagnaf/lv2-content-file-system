#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

// �ļ��ṹ����Ϊ �û��ռ��ļ� �� �û��ռ��� ʹ��
// eg username,ufd0 ���� filename,inode0
struct File {
  char name[14]; // �ļ������û���
  int pt; // ָ��i�ڵ��ufd
  // ��ʼ�����ֺ�ָ��
  void init(const char *s, int d);
};

#endif // FILE_H_INCLUDED

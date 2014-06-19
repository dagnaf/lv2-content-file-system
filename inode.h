#ifndef INODE_H_INCLUDED
#define INODE_H_INCLUDED

#include <ctime>

struct Inode
{
  int uid; // �����û� id
  int type; // �ļ�����
  int permission; // �ļ����
  int blocks; // ��ռ�̿�
  int sz; // �ļ���С
  int link; // �ļ�������
  time_t create_time; // ����ʱ��
  time_t modify_time; // �޸�ʱ��
  int addr[13]; // ��ռ�̿�ţ�0-9Ϊֱ��������10-12Ϊ���������δʵ�֣�
  Inode(); // �ճ�ʼ
  Inode(int i, int t, int p); // ��ʼ�� �û�id�����͡����Ȩ
  void print(); // ��ӡ
  void update(int s); // ���´�С
};

#endif // INODE_H_INCLUDED

#include <ctime>
#include <cstring>
#include <cstdio>
#include "inode.h"

// �ճ�ʼ��
Inode::Inode() {}
// һ���ʼ��
Inode::Inode(int i, int t, int p) {
  uid = i; type = t; permission = p;
  blocks = 0;
  sz = 0;
  link = 0;
  // ����Ϊ��ǰʱ�䣬������ʱ��
  time(&create_time);
  time(&modify_time);
  memset(addr, -1, sizeof(addr));
}
// ��ӡ��Ϣ
void Inode::print() {
  printf("user id: %d\n", uid);
  printf("type: %d\n", type);
  printf("permission: %d\n", permission);
  printf("blocks: %d\n", blocks);
  printf("size: %d\n", sz);
  printf("link: %d\n", link);
  printf("create time: %s", ctime(&create_time));
  printf("modify time: %s", ctime(&modify_time));
}
// ���´�С
void Inode::update(int s) {
  sz = s; blocks = s; time(&modify_time);
  for (int i = s; i < 10; ++i) addr[i] = -1;
}

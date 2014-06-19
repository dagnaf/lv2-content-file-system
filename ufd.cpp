#include <cstring>
#include "disk.h"

// ��ʼ����ǰ�ļ�Ŀ¼����������ļ�
void UFD::init() {
  n = 0;
  for (int i = 0; i < MAX_FILE; ++i) files[i].init("", -1);
}
// ����ļ���
// ����1���ļ����ָ�룬��ӳɹ�����Ϊ�µı��
// ����2����ӵ��ļ���
// ���أ�������Ϣ
int UFD::add(char *s, int type, int permission) {
  if (n == MAX_FILE) return e_max; // ��������ļ���
  if (find(s) >= 0) return e_dup;  // �ļ����ظ�
  int new_inode_id = newInode();
  files[newId()].init((const char *)s, new_inode_id);
  Inode new_inode(new_inode_id, type, permission);
  iwrite(&new_inode, INODE_SZ, superblock.inode+new_inode_id*INODE_SZ);
  n++;
  return 0;
}
// ����һ���յ��ļ�id
int UFD::newId() {
  for (int i = 0; i < MAX_FILE; ++i)
    if (files[i].pt == -1)
      return i;
  return -1;
}
// �����ļ���
// �������ļ���
// ���أ� -1δ�ҵ���>0�ļ����
int UFD::find(char *s) {
  for (int i = 0; i < MAX_FILE; ++i)
    if (files[i].pt != -1 && strcmp(files[i].name, s) == 0)
      return i;
  return -1;
}
// ɾ���ļ���
// �������ļ����
// ͬʱɾ����Ӧ��i�ڵ㣬�Լ�i�ڵ���addr��ָ����̿�
int UFD::del(char *s) {
  int fid = find(s);
  if (fid == -1) return e_not;
  int inode_id = files[fid].pt;
  files[fid].pt = -1;
  Inode inode;
  iread(&inode, INODE_SZ, superblock.inode+INODE_SZ*inode_id);
  deleteInode(inode_id);
  deleteBlock(inode);
  n--;
  return 0;
}

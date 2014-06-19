#include <cstring>
#include "disk.h"
// ��ʼ��mfd������admin���������
void MFD::init() {
  for (int i = 0; i < MAX_USER; ++i) users[i].pt = -1;
  users[0].init("admin", 0);
  n = 1;
}
// ����û���ͬʱΪ�û�����һ��ufd
int MFD::add(char *s) {
  if (n == MAX_USER) return e_max;
  if (find(s) >= 0) return e_dup;
  int new_uid = newId();
  users[new_uid].init((const char *)s, new_uid);
  UFD new_ufd; new_ufd.init();
  iwrite(&new_ufd, UFD_SZ, superblock.ufd+new_uid*UFD_SZ);
  n++;
  return 0;
}
// ����һ���յ��û�id
int MFD::newId() {
  for (int i = 0; i < MAX_FILE; ++i)
    if (users[i].pt == -1)
      return i;
  return -1;
}
// �����û�
int MFD::find(char *s) {
  for (int i = 0; i < MAX_USER; ++i)
    if (users[i].pt != -1 && strcmp(s, users[i].name) == 0)
      return i;
    return -1;
}
// ��½
int MFD::login(char *s) {
    return find(s);
}
// ɾ���û����û�Ŀ¼���ղ���ɾ��
int MFD::del(char *s) {
  int uid = find(s);
  if (uid == -1) return e_not;
  UFD ufd; // �ȶ�ȡ���û��ռ��Ŀ¼
  iread(&ufd, UFD_SZ, superblock.ufd+uid*UFD_SZ);
  if (ufd.n > 0) return e_invalid; // Ŀ¼��Ϊ��
  users[uid].init("", -1);
  n--;
  return 0;
}

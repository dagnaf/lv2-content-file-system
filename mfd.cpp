#include <cstring>
#include "disk.h"
// 初始化mfd，设置admin，其余清空
void MFD::init() {
  for (int i = 0; i < MAX_USER; ++i) users[i].pt = -1;
  users[0].init("admin", 0);
  n = 1;
}
// 添加用户，同时为用户创建一个ufd
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
// 申请一个空的用户id
int MFD::newId() {
  for (int i = 0; i < MAX_FILE; ++i)
    if (users[i].pt == -1)
      return i;
  return -1;
}
// 查找用户
int MFD::find(char *s) {
  for (int i = 0; i < MAX_USER; ++i)
    if (users[i].pt != -1 && strcmp(s, users[i].name) == 0)
      return i;
    return -1;
}
// 登陆
int MFD::login(char *s) {
    return find(s);
}
// 删除用户，用户目录不空不能删除
int MFD::del(char *s) {
  int uid = find(s);
  if (uid == -1) return e_not;
  UFD ufd; // 先读取该用户空间的目录
  iread(&ufd, UFD_SZ, superblock.ufd+uid*UFD_SZ);
  if (ufd.n > 0) return e_invalid; // 目录不为空
  users[uid].init("", -1);
  n--;
  return 0;
}

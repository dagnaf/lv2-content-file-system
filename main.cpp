// main.cpp
// disk.h����������ģ����̵ĺ���
#include "disk.h"
// �������
int main(int argc, char const *argv[]) {
  init(); // ��ʼ������
  while (1) {
    prompt(); // ��ʾ·��
    // ��ȡһ���������ɵ��ʣ��������Ϊ�������
    if (getCMD() == 1) continue;
    if (leave() == 1) break; // �������Ϊ�뿪��������
    else cmdHandler(); // �����������
  }
  return 0;
}

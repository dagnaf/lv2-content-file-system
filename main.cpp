// main.cpp
// disk.h包含了所有模拟磁盘的函数
#include "disk.h"
// 程序入口
int main(int argc, char const *argv[]) {
  init(); // 初始化磁盘
  while (1) {
    prompt(); // 提示路径
    // 获取一行命令，并拆成单词，如果命令为空则继续
    if (getCMD() == 1) continue;
    if (leave() == 1) break; // 如果命令为离开，则跳出
    else cmdHandler(); // 否则分析命令
  }
  return 0;
}

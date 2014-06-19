#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "disk.h"

FILE *fd;
SuperBlock superblock;
MFD mfd;
// 盘块位示图 和 i节点位示图
int bitmap_block[BLOCK_BITMAP_WORD];
int bitmap_inode[INODE_BITMAP_WORD];

char cmd[25]; // 命令头，例如create -u abc，命令头就是create
char line[100]; // 输入的命令整行
char *tokens[10]; // 分隔空格后的命令单词
int num_tokens; // 单词个数
char cwd[50]; // 当前路径
int uid; // 当前登陆的用户id，-1表示未登陆
int cuid; // 表示当前所在文件夹是属于哪个用户的
// 事例文字，用于写操作
char sample[BLOCK_SZ] = "sample text\n";
// 读操作的缓冲数组
char buf[BLOCK_SZ];

// 读磁盘
void iread(void *obj, int sz, long offset, int cl, const char *mode) {
  // 如果文件未打开，则打开
  if (fd == NULL) fd = fopen(VDISK, mode);
  // 设置读取文件时的偏移位置offset，seek_set为文件开始
  // 即将读指针 指向 文件开始后的offset个字节处
  fseek(fd, offset, SEEK_SET);
  // 从文件读指针开始读取sz个字节
  fread(obj, sz, 1, fd);
  // 如果要关闭文件，则关闭
  if (cl) iclose();
}
// 写磁盘
void iwrite(void *obj, int sz, long offset, int cl, const char *mode) {
  // 基本同读磁盘
  if (fd == NULL) fd = fopen(VDISK, mode);
  fseek(fd, offset, SEEK_SET);
  fwrite(obj, sz, 1, fd);
  if (cl) iclose();
}
// 关闭文件
void iclose() {
  fclose(fd); fd = NULL;
}
// 获取命令
int getCMD() {
  num_tokens = 1;
  fgets(line, 100, stdin); // 从标准输入获得一行
  // 如果为空串，返回1表示
  if ((tokens[0] = strtok(line, " \n\t")) == NULL) return 1;
  // 否则继续获取单词
  while ((tokens[num_tokens] = strtok(NULL, " \t\n")) != NULL) ++num_tokens;
  return 0;
}
// 错误处理
void errHandler(int e_type, const char *src, char *arg = "") {
  // 打印错误来源
  printf("%s: ", src);
  switch(e_type) {
    case e_max: printf("max sized\n"); break;
    case e_dup: printf("duplicated\n"); break;
    case e_not: printf("%s not found\n", arg); break;
    default: printf("error\n"); break;
  }
}
// 安装
void install() {
  printf("installing ...\n");
  fd = fopen(VDISK, "wb");
  if (fd == NULL) { // 文件不能创建，重新退出后再尝试
    printf("cannot install file system, restart program\n");
    exit(1);
  }
  // 初始化磁盘中的内容，例如超级块中磁盘划分的指针，根目录区的admin等
  superblock.init();
  iwrite(&superblock, BLOCK_SZ, superblock.sb, 0);
  mfd.init();
  iwrite(&mfd, MFD_BLOCKS * BLOCK_SZ, superblock.mfd, 0);
  UFD new_ufd; new_ufd.init();
  iwrite(&new_ufd, UFD_SZ, superblock.ufd, 0);
  memset(bitmap_inode, 0, sizeof(bitmap_inode));
  iwrite(bitmap_inode, INODE_BITMAP_BLOCKS * BLOCK_SZ, superblock.inode_bitmap, 0);
  memset(bitmap_block, 0, sizeof(bitmap_inode));
  iwrite(bitmap_block, BLOCK_BITMAP_BLOCKS * BLOCK_SZ, superblock.block_bitmap, 0);
  iclose();
}
void init() {
  printf("setting up ...\n");
  fd = fopen(VDISK, "rb+");
  if (fd == NULL) { // 无法打开文件， 提示安装
    printf("file system not installed, ready to install? <y/n> ");
    if (getCMD() == 0 && strcmp(tokens[0], "y") == 0) install();
    else {
      printf("file system must be installed first\n");
      exit(1);
    }
    // 安装完成，重新打开
    fd = fopen(VDISK, "rb");
    if (fd == NULL) {
      printf("fail to open disk, restart file system\n");
      exit(1);
    }
  }
  // 读取初始化过的磁盘数据，包括超级块等
  iread(&superblock, sizeof(superblock), superblock.sb, 0);
  iread(&mfd, MFD_SZ, superblock.mfd, 0);
  iread(bitmap_inode, INODE_BITMAP_SZ, superblock.inode_bitmap, 0);
  iread(bitmap_block, BLOCK_BITMAP_SZ, superblock.block_bitmap);
  strcpy(cwd, "/root"); // 设置当前路径
  printf("welcome\n");
  uid = -1; cuid = -1; // 设置未登陆，当前用户和当前目录所在用户为空
}
void infoSys() {
    int used;
    used = BLOCKS - superblock.block_cap;
    printf("system reserved blocks: %d\n", superblock.data/BLOCK_SZ);
    printf("blocks: %d/%d\n", used, BLOCKS - superblock.data/BLOCK_SZ);
    printf("block size: %d\n", BLOCK_SZ);
    used = MAX_USER*MAX_FILE - superblock.inode_cap;
    printf("inodes: %d/%d\n", used, MAX_USER*MAX_FILE);
    printf("users: %d\n", mfd.n);
}
void prompt() {
  if (uid != -1) printf("%s@local ", mfd.users[uid].name);
  printf("%s $ ", cwd);
}
// 显示用户空间所有文件
void showDir() {
  int flag = 0; // 标记是否为空
  if (strcmp(cwd, "/root") == 0) { // 在根目录区则显示所有用户名
    for (int i = 0; i < MAX_USER; ++i) {
      if (mfd.users[i].pt != -1) {
        flag = 1;
        printf("\t<dir>\t%s\n", mfd.users[i].name);
      }
    }
  } else { // 否则显示当前目录下的所有文件
    UFD ufd;
    iread(&ufd, UFD_SZ, superblock.ufd+UFD_SZ*cuid);
    for (int i = 0; i < MAX_FILE; ++i) {
      if (ufd.files[i].pt != -1) {
        flag = 1;
        printf("\t<file>\t%s - (inode %d)\n", ufd.files[i].name, ufd.files[i].pt);
      }
    }
  }
  // 如果目录为空，显示空
  if (!flag) printf("empty\n");
}
// 登陆，暂时不需要密码
void login() {
  if (tokens[1] != NULL) {
    int new_uid = mfd.login(tokens[1]);
    if (new_uid >= 0) {
      uid = new_uid; cuid = uid; // 登陆的同时进入相应登陆的目录
      strcpy(cwd, "/root/");
      strcat(cwd, tokens[1]);
    } else printf("login: username not found\n");
  } else printf("login: lonin username\n");
}
// 申请一个空闲的i节点
int newInode() {
  for (int i = 0; i < MAX_USER*MAX_FILE; ++i) {
    int r = i / WORD_LEN;
    int c = i % WORD_LEN;
    if (((bitmap_inode[r]) & (1 << c)) == 0) {
      bitmap_inode[r] |= (1 << c); // 申请成功，置1
      superblock.inode_cap--; // 同时系统i节点数减少1
      // 写回磁盘保存信息
      iwrite(&superblock, BLOCK_SZ, superblock.sb, 0);
      iwrite(&bitmap_inode[r], sizeof(int), superblock.inode_bitmap+r*sizeof(int));
      return i;
    }
  }
  return -1;
}
// 创建用户
void createUser() {
  int e = mfd.add(tokens[2]);
  if (e == 0) iwrite(&mfd, MFD_BLOCKS * BLOCK_SZ, superblock.mfd);
  else errHandler(e, "create", tokens[2]);
}
// 创建文件
void createFile() {
  UFD new_ufd; // 首先读取当前目录的文件目录
  iread(&new_ufd, UFD_SZ, superblock.ufd+UFD_SZ*cuid);
  int e = new_ufd.add(tokens[1]); // 在该目录下添加文件
  if (e == 0) iwrite(&new_ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ); // 否则写入
  else errHandler(e, "create", tokens[1]); // 如果有错，例如重名等，提示
}
// 创建命令
void create() {
  if (tokens[1] != NULL) {
    if (strcmp(tokens[1], "-u") == 0) { // 如果是create -u 的类型则表示创建用户。
      if (tokens[2] == NULL) {
        printf("create: create -u username | filename\n");
      } else { // 必须由admin来创建
        if (uid != 0) printf("no permission\n");
        else createUser();
      }
    } else {
      if (cuid == -1) printf("create: cannot create file at root\n");
      else createFile();
    }
  } else printf("create: create -u username | filename\n");
}
// 改变路径
void change() {
  if (uid != 0) { // 不是admin就不能改变路径，因为目录总共只有2级
    printf("cd: no such directory\n");
  } else if (tokens[1] == NULL) { // 参数为空返回跟目录
    strcpy(cwd, "/root"); cuid = -1; // 同时当前目录不属于任何用户
  } else {
    if (strcmp(tokens[1], "..") == 0) { // .. 返回上1级
      strcpy(cwd, "/root"); cuid = -1; // 同时当前目录不属于任何用户
    } else if (strcmp(tokens[1], ".") == 0) {
    } else {
      if (strcmp(cwd, "/root") == 0) { // 如果当前是根目录
        int new_uid = mfd.find(tokens[1]); // 参数如果是已经存在对应的用户文件夹
        if (new_uid >= 0) { // 那么可以进入
          cuid = new_uid;
          strcat(cwd, "/"); strcat(cwd, tokens[1]);
        } else printf("cd: no such directory\n");
      } else printf("cd: no such directory\n");
    }
  }
}
// 释放i节点
void deleteInode(int x) {
  int r = x / WORD_LEN;
  int c = x % WORD_LEN;
  bitmap_inode[r] &= ~(1 << c);
  fd = fopen(VDISK, "rb+");
  superblock.inode_cap++;
  iwrite(&superblock, superblock.sb, BLOCK_SZ, 0);
  iwrite(&bitmap_inode[r], sizeof(int), superblock.inode_bitmap+r*sizeof(int));
}
// 释放i节点，同时释放i节点addr所占有的盘块
void deleteBlock(Inode inode) {
  int r, c, flag = 0;
  for (int i = 0; i < inode.blocks; ++i) {
    int x = inode.addr[i];
    if (x != -1) {
      flag = 1;
      superblock.block_cap++;
      r = x / WORD_LEN; c = x % WORD_LEN;
      bitmap_block[r] &= ~(1 << c);
    } else break;
  }
  inode.update(0);
  if (!flag) return;
  iwrite(bitmap_block, BLOCK_BITMAP_BLOCKS * BLOCK_SZ, superblock.block_bitmap);
}
// 删除用户
void delUser() {
  int e = mfd.del(tokens[2]);
  if (e == 0) { // 当用户目录不空时不能删除
    iwrite(&mfd, MFD_BLOCKS * BLOCK_SZ, superblock.mfd);
    strcpy(cwd, "/root");
  } else errHandler(e, "delete", tokens[2]);
}
// 删除文件
void delFile() {
  UFD ufd;
  iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
  int e = ufd.del(tokens[1]);
  if (e == 0) iwrite(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
  else errHandler(e, "delete", tokens[1]);
}
// 删除命令
void del() {
  if (tokens[1] != NULL) {
    if (strcmp(tokens[1], "-u") == 0) {
      if (tokens[2] == NULL) {
        printf("delete: delete -u username | filename\n");
      } else {
        if (uid != 0)
          printf("no permission\n");
        else if (strcmp(tokens[2], "admin") == 0)
          printf("delete: admin cannot be deleted\n");
        else delUser();
      }
    } else {
        if (cuid == -1) printf("delete: cannot delete file at root\n");
        else delFile();
    }
  } else {
    printf("delete: delete -u username | filename\n");
  }
}
// 提示信息，如果无参数转至infoSys，否则在该函数中显示文件信息
void info() {
  if (tokens[1] == NULL) infoSys();
  else {
    UFD ufd; // 首先查找文件是否存在
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
    int fid = ufd.find(tokens[1]);
    if (fid >= 0) {
      int inode_id = ufd.files[fid].pt;
      Inode inode;
      iread(&inode, INODE_SZ, superblock.inode+inode_id*INODE_SZ);
      inode.print();
    } else errHandler(e_not, "info", tokens[1]);
  }
}
// 申请空的盘块
int newBlock(int id) {
  int st = superblock.data/BLOCK_SZ;
  for (int i = (id+1) % BLOCKS; i != id; i = (i+1) % BLOCKS) {
    if (i < st) i = st;
    int r = i / WORD_LEN;
    int c = i % WORD_LEN;
    if ((bitmap_block[r] & (1 << c)) == 0) {
      bitmap_block[r] |= (1 << c);
      superblock.block_cap--;
      return i;
    }
  }
  return -1;
}
// 复制命令
void cp() {
  if (cuid == -1) printf("copy: cannot copy file at root\n");
  else if (tokens[1] == NULL || tokens[2] == NULL) {
    printf("copy: copy copyfilename srcfilename eg copy 1.txt 2.txt\n");
  } else {
    UFD ufd; // 首先读取当前目录下的文件
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
    int fid_dest = ufd.find(tokens[1]); // 看是否复制的文件会重名
    if (fid_dest >= 0) {
      printf("copy: copy file:%s already exists\n", tokens[1]);
      return;
    } // 再看源文件是否存在
    int fid_src = ufd.find(tokens[2]);
    if (fid_src < 0) {
      printf("copy: source file:%s not found\n", tokens[2]);
      return;
    } // 看系统空间是否足够
    int inode_id_src = ufd.files[fid_src].pt;
    Inode inode_src;
    iread(&inode_src, INODE_SZ, superblock.inode+inode_id_src*INODE_SZ);
    if (inode_src.sz > superblock.block_cap) {
      printf("copy: blocks not enough\n");
      return;
    }
    // 一切ok，先创建一个空文件
    printf("creating copyfile ...\n");
    createFile();
    // 重新读取目录
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ, 0);
    fid_dest = ufd.find(tokens[1]);
    int inode_id_dest = ufd.files[fid_dest].pt;
    Inode inode_dest;
    iread(&inode_dest, INODE_SZ, superblock.inode+inode_id_dest*INODE_SZ, 0);
    // 开始复制
    int i; int block_id = 0;
    for (i = 0; i < inode_src.blocks ; ++i) {
      block_id = newBlock(block_id);
      inode_dest.addr[i] = block_id;
      // 读取源文件
      iread(buf, BLOCK_SZ, inode_src.addr[i]*BLOCK_SZ, 0);
      // 拷贝
      iwrite(buf, BLOCK_SZ, inode_dest.addr[i]*BLOCK_SZ, 0);
    }
    // 设置盘块末尾
    inode_dest.addr[i] = -1;
    inode_dest.update(inode_src.blocks);
    // 系统空间变化，位示图变化，i节点区变化，所有信息写回磁盘
    iwrite(bitmap_block, BLOCK_BITMAP_BLOCKS * BLOCK_SZ, superblock.block_bitmap, 0);
    iwrite(&inode_dest, INODE_SZ, superblock.inode+inode_id_dest*INODE_SZ, 0);
    iwrite(&superblock, superblock.sb, BLOCK_SZ);
  }
}
// 无用测试
void test(int block_id) {
    memset(buf, 0, sizeof(buf));
    iread(buf, BLOCK_SZ, block_id*BLOCK_SZ);
    puts(buf);
}
// 写命令，只能指定写文件的大小块数，没有具体内容。每一块都由测试字符串代替填充
void write() {
  if (tokens[1] == NULL || tokens[2] == NULL) {
    printf("write: write filename blocks eg write 1.txt 10\n");
  } else {
    int sz = -1; // 先读取要写的盘块大小
    sscanf(tokens[2], "%d", &sz);
    // 还未实现间i节点的接索引，所以一个文件最多只能写10块
    if (sz == -1 || (sz <= 10 && superblock.block_cap < sz) || sz > 10) {
      printf("write: blocks not enough or lv1+ allocation not implemented\n");
      return;
    }
    UFD ufd; // 看要写的文件是否存在
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
    int fid = ufd.find(tokens[1]);
    if (fid < 0) {
      printf("write: file not found\n");
      return;
    } // 读取i节点
    int inode_id = ufd.files[fid].pt;
    int block_id = 0;
    int i; Inode new_inode;
    iread(&new_inode, INODE_SZ, superblock.inode+inode_id*INODE_SZ, 0);
    // 文件本来就有内容，则清空
    if (sz < new_inode.sz) deleteBlock(new_inode);
    // 清空后重写
    for (i = 0; i < sz ; ++i) {
      block_id = newBlock(block_id);
      new_inode.addr[i] = block_id;
      iwrite(sample, BLOCK_SZ, block_id*BLOCK_SZ, 0);
    }
    // 设置盘块结束位置
    new_inode.addr[i] = -1;
    new_inode.update(sz);
    // 保存
    iwrite(bitmap_block, BLOCK_BITMAP_BLOCKS*BLOCK_SZ, superblock.block_bitmap, 0);
    iwrite(&new_inode, INODE_SZ, superblock.inode+inode_id*INODE_SZ, 0);
    iwrite(&superblock, BLOCK_SZ, superblock.sb);
  }
}
void read() {
  if (tokens[1] == NULL) {
      printf("read: read filename\n");
  } else {
    UFD ufd; // 看文件是否存在
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
    int fid = ufd.find(tokens[1]);
    if (fid < 0) {
      printf("read: file not found\n");
      return;
    } // 读取i节点
    int inode_id = ufd.files[fid].pt;
    int i; Inode inode;
    iread(&inode, INODE_SZ, superblock.inode+inode_id*INODE_SZ, 0);
    // 根据i节点中addr所指向的盘块号，读取盘块中的内容
    for (i = 0; i < inode.blocks ; ++i) {
      memset(buf, 0, sizeof(buf));
      iread(buf, BLOCK_SZ, inode.addr[i] * BLOCK_SZ, 0);
      printf("%s", buf);
    }
    iclose();
  }
}
// 处理命令
void cmdHandler() {
  if (strcmp(tokens[0], "login") == 0) {
    login();
  } else if (uid == -1) { // 未登陆时，只能看有哪些用户
    if (strcmp(tokens[0], "dir") == 0) showDir();
    else printf("no permission\n");
  } else if (strcmp(tokens[0], "dir") == 0) {
    showDir();
  } else if (strcmp(tokens[0], "create") == 0) {
    create();
  } else if (strcmp(tokens[0], "delete") == 0) {
    del();
  } else if (strcmp(tokens[0], "cd") == 0) {
    change();
  } else if (strcmp(tokens[0], "reinstall") == 0) {
    if (uid == 0) { install(); init(); }
    else printf("no permission\n");
  } else if (strcmp(tokens[0], "logout") == 0) {
    uid = -1; cuid = -1;
    strcpy(cwd, "/root");
  } else if (strcmp(tokens[0], "write") == 0) {
    write();
  } else if (strcmp(tokens[0], "read") == 0) {
    read();
  } else if (strcmp(tokens[0], "copy") == 0) {
    cp();
  } else if (strcmp(tokens[0], "info") == 0) {
    info();
  } else printf("cmd: no such command\n");
}
// 退出
int leave() {
    if (strcmp(tokens[0], "exit") == 0) {
        printf("goodbye!\n");
        return 1;
    } else return 0;
}

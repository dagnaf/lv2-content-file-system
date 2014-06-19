#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "disk.h"

FILE *fd;
SuperBlock superblock;
MFD mfd;
// �̿�λʾͼ �� i�ڵ�λʾͼ
int bitmap_block[BLOCK_BITMAP_WORD];
int bitmap_inode[INODE_BITMAP_WORD];

char cmd[25]; // ����ͷ������create -u abc������ͷ����create
char line[100]; // �������������
char *tokens[10]; // �ָ��ո��������
int num_tokens; // ���ʸ���
char cwd[50]; // ��ǰ·��
int uid; // ��ǰ��½���û�id��-1��ʾδ��½
int cuid; // ��ʾ��ǰ�����ļ����������ĸ��û���
// �������֣�����д����
char sample[BLOCK_SZ] = "sample text\n";
// �������Ļ�������
char buf[BLOCK_SZ];

// ������
void iread(void *obj, int sz, long offset, int cl, const char *mode) {
  // ����ļ�δ�򿪣����
  if (fd == NULL) fd = fopen(VDISK, mode);
  // ���ö�ȡ�ļ�ʱ��ƫ��λ��offset��seek_setΪ�ļ���ʼ
  // ������ָ�� ָ�� �ļ���ʼ���offset���ֽڴ�
  fseek(fd, offset, SEEK_SET);
  // ���ļ���ָ�뿪ʼ��ȡsz���ֽ�
  fread(obj, sz, 1, fd);
  // ���Ҫ�ر��ļ�����ر�
  if (cl) iclose();
}
// д����
void iwrite(void *obj, int sz, long offset, int cl, const char *mode) {
  // ����ͬ������
  if (fd == NULL) fd = fopen(VDISK, mode);
  fseek(fd, offset, SEEK_SET);
  fwrite(obj, sz, 1, fd);
  if (cl) iclose();
}
// �ر��ļ�
void iclose() {
  fclose(fd); fd = NULL;
}
// ��ȡ����
int getCMD() {
  num_tokens = 1;
  fgets(line, 100, stdin); // �ӱ�׼������һ��
  // ���Ϊ�մ�������1��ʾ
  if ((tokens[0] = strtok(line, " \n\t")) == NULL) return 1;
  // ���������ȡ����
  while ((tokens[num_tokens] = strtok(NULL, " \t\n")) != NULL) ++num_tokens;
  return 0;
}
// ������
void errHandler(int e_type, const char *src, char *arg = "") {
  // ��ӡ������Դ
  printf("%s: ", src);
  switch(e_type) {
    case e_max: printf("max sized\n"); break;
    case e_dup: printf("duplicated\n"); break;
    case e_not: printf("%s not found\n", arg); break;
    default: printf("error\n"); break;
  }
}
// ��װ
void install() {
  printf("installing ...\n");
  fd = fopen(VDISK, "wb");
  if (fd == NULL) { // �ļ����ܴ����������˳����ٳ���
    printf("cannot install file system, restart program\n");
    exit(1);
  }
  // ��ʼ�������е����ݣ����糬�����д��̻��ֵ�ָ�룬��Ŀ¼����admin��
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
  if (fd == NULL) { // �޷����ļ��� ��ʾ��װ
    printf("file system not installed, ready to install? <y/n> ");
    if (getCMD() == 0 && strcmp(tokens[0], "y") == 0) install();
    else {
      printf("file system must be installed first\n");
      exit(1);
    }
    // ��װ��ɣ����´�
    fd = fopen(VDISK, "rb");
    if (fd == NULL) {
      printf("fail to open disk, restart file system\n");
      exit(1);
    }
  }
  // ��ȡ��ʼ�����Ĵ������ݣ������������
  iread(&superblock, sizeof(superblock), superblock.sb, 0);
  iread(&mfd, MFD_SZ, superblock.mfd, 0);
  iread(bitmap_inode, INODE_BITMAP_SZ, superblock.inode_bitmap, 0);
  iread(bitmap_block, BLOCK_BITMAP_SZ, superblock.block_bitmap);
  strcpy(cwd, "/root"); // ���õ�ǰ·��
  printf("welcome\n");
  uid = -1; cuid = -1; // ����δ��½����ǰ�û��͵�ǰĿ¼�����û�Ϊ��
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
// ��ʾ�û��ռ������ļ�
void showDir() {
  int flag = 0; // ����Ƿ�Ϊ��
  if (strcmp(cwd, "/root") == 0) { // �ڸ�Ŀ¼������ʾ�����û���
    for (int i = 0; i < MAX_USER; ++i) {
      if (mfd.users[i].pt != -1) {
        flag = 1;
        printf("\t<dir>\t%s\n", mfd.users[i].name);
      }
    }
  } else { // ������ʾ��ǰĿ¼�µ������ļ�
    UFD ufd;
    iread(&ufd, UFD_SZ, superblock.ufd+UFD_SZ*cuid);
    for (int i = 0; i < MAX_FILE; ++i) {
      if (ufd.files[i].pt != -1) {
        flag = 1;
        printf("\t<file>\t%s - (inode %d)\n", ufd.files[i].name, ufd.files[i].pt);
      }
    }
  }
  // ���Ŀ¼Ϊ�գ���ʾ��
  if (!flag) printf("empty\n");
}
// ��½����ʱ����Ҫ����
void login() {
  if (tokens[1] != NULL) {
    int new_uid = mfd.login(tokens[1]);
    if (new_uid >= 0) {
      uid = new_uid; cuid = uid; // ��½��ͬʱ������Ӧ��½��Ŀ¼
      strcpy(cwd, "/root/");
      strcat(cwd, tokens[1]);
    } else printf("login: username not found\n");
  } else printf("login: lonin username\n");
}
// ����һ�����е�i�ڵ�
int newInode() {
  for (int i = 0; i < MAX_USER*MAX_FILE; ++i) {
    int r = i / WORD_LEN;
    int c = i % WORD_LEN;
    if (((bitmap_inode[r]) & (1 << c)) == 0) {
      bitmap_inode[r] |= (1 << c); // ����ɹ�����1
      superblock.inode_cap--; // ͬʱϵͳi�ڵ�������1
      // д�ش��̱�����Ϣ
      iwrite(&superblock, BLOCK_SZ, superblock.sb, 0);
      iwrite(&bitmap_inode[r], sizeof(int), superblock.inode_bitmap+r*sizeof(int));
      return i;
    }
  }
  return -1;
}
// �����û�
void createUser() {
  int e = mfd.add(tokens[2]);
  if (e == 0) iwrite(&mfd, MFD_BLOCKS * BLOCK_SZ, superblock.mfd);
  else errHandler(e, "create", tokens[2]);
}
// �����ļ�
void createFile() {
  UFD new_ufd; // ���ȶ�ȡ��ǰĿ¼���ļ�Ŀ¼
  iread(&new_ufd, UFD_SZ, superblock.ufd+UFD_SZ*cuid);
  int e = new_ufd.add(tokens[1]); // �ڸ�Ŀ¼������ļ�
  if (e == 0) iwrite(&new_ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ); // ����д��
  else errHandler(e, "create", tokens[1]); // ����д����������ȣ���ʾ
}
// ��������
void create() {
  if (tokens[1] != NULL) {
    if (strcmp(tokens[1], "-u") == 0) { // �����create -u ���������ʾ�����û���
      if (tokens[2] == NULL) {
        printf("create: create -u username | filename\n");
      } else { // ������admin������
        if (uid != 0) printf("no permission\n");
        else createUser();
      }
    } else {
      if (cuid == -1) printf("create: cannot create file at root\n");
      else createFile();
    }
  } else printf("create: create -u username | filename\n");
}
// �ı�·��
void change() {
  if (uid != 0) { // ����admin�Ͳ��ܸı�·������ΪĿ¼�ܹ�ֻ��2��
    printf("cd: no such directory\n");
  } else if (tokens[1] == NULL) { // ����Ϊ�շ��ظ�Ŀ¼
    strcpy(cwd, "/root"); cuid = -1; // ͬʱ��ǰĿ¼�������κ��û�
  } else {
    if (strcmp(tokens[1], "..") == 0) { // .. ������1��
      strcpy(cwd, "/root"); cuid = -1; // ͬʱ��ǰĿ¼�������κ��û�
    } else if (strcmp(tokens[1], ".") == 0) {
    } else {
      if (strcmp(cwd, "/root") == 0) { // �����ǰ�Ǹ�Ŀ¼
        int new_uid = mfd.find(tokens[1]); // ����������Ѿ����ڶ�Ӧ���û��ļ���
        if (new_uid >= 0) { // ��ô���Խ���
          cuid = new_uid;
          strcat(cwd, "/"); strcat(cwd, tokens[1]);
        } else printf("cd: no such directory\n");
      } else printf("cd: no such directory\n");
    }
  }
}
// �ͷ�i�ڵ�
void deleteInode(int x) {
  int r = x / WORD_LEN;
  int c = x % WORD_LEN;
  bitmap_inode[r] &= ~(1 << c);
  fd = fopen(VDISK, "rb+");
  superblock.inode_cap++;
  iwrite(&superblock, superblock.sb, BLOCK_SZ, 0);
  iwrite(&bitmap_inode[r], sizeof(int), superblock.inode_bitmap+r*sizeof(int));
}
// �ͷ�i�ڵ㣬ͬʱ�ͷ�i�ڵ�addr��ռ�е��̿�
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
// ɾ���û�
void delUser() {
  int e = mfd.del(tokens[2]);
  if (e == 0) { // ���û�Ŀ¼����ʱ����ɾ��
    iwrite(&mfd, MFD_BLOCKS * BLOCK_SZ, superblock.mfd);
    strcpy(cwd, "/root");
  } else errHandler(e, "delete", tokens[2]);
}
// ɾ���ļ�
void delFile() {
  UFD ufd;
  iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
  int e = ufd.del(tokens[1]);
  if (e == 0) iwrite(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
  else errHandler(e, "delete", tokens[1]);
}
// ɾ������
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
// ��ʾ��Ϣ������޲���ת��infoSys�������ڸú�������ʾ�ļ���Ϣ
void info() {
  if (tokens[1] == NULL) infoSys();
  else {
    UFD ufd; // ���Ȳ����ļ��Ƿ����
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
// ����յ��̿�
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
// ��������
void cp() {
  if (cuid == -1) printf("copy: cannot copy file at root\n");
  else if (tokens[1] == NULL || tokens[2] == NULL) {
    printf("copy: copy copyfilename srcfilename eg copy 1.txt 2.txt\n");
  } else {
    UFD ufd; // ���ȶ�ȡ��ǰĿ¼�µ��ļ�
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
    int fid_dest = ufd.find(tokens[1]); // ���Ƿ��Ƶ��ļ�������
    if (fid_dest >= 0) {
      printf("copy: copy file:%s already exists\n", tokens[1]);
      return;
    } // �ٿ�Դ�ļ��Ƿ����
    int fid_src = ufd.find(tokens[2]);
    if (fid_src < 0) {
      printf("copy: source file:%s not found\n", tokens[2]);
      return;
    } // ��ϵͳ�ռ��Ƿ��㹻
    int inode_id_src = ufd.files[fid_src].pt;
    Inode inode_src;
    iread(&inode_src, INODE_SZ, superblock.inode+inode_id_src*INODE_SZ);
    if (inode_src.sz > superblock.block_cap) {
      printf("copy: blocks not enough\n");
      return;
    }
    // һ��ok���ȴ���һ�����ļ�
    printf("creating copyfile ...\n");
    createFile();
    // ���¶�ȡĿ¼
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ, 0);
    fid_dest = ufd.find(tokens[1]);
    int inode_id_dest = ufd.files[fid_dest].pt;
    Inode inode_dest;
    iread(&inode_dest, INODE_SZ, superblock.inode+inode_id_dest*INODE_SZ, 0);
    // ��ʼ����
    int i; int block_id = 0;
    for (i = 0; i < inode_src.blocks ; ++i) {
      block_id = newBlock(block_id);
      inode_dest.addr[i] = block_id;
      // ��ȡԴ�ļ�
      iread(buf, BLOCK_SZ, inode_src.addr[i]*BLOCK_SZ, 0);
      // ����
      iwrite(buf, BLOCK_SZ, inode_dest.addr[i]*BLOCK_SZ, 0);
    }
    // �����̿�ĩβ
    inode_dest.addr[i] = -1;
    inode_dest.update(inode_src.blocks);
    // ϵͳ�ռ�仯��λʾͼ�仯��i�ڵ����仯��������Ϣд�ش���
    iwrite(bitmap_block, BLOCK_BITMAP_BLOCKS * BLOCK_SZ, superblock.block_bitmap, 0);
    iwrite(&inode_dest, INODE_SZ, superblock.inode+inode_id_dest*INODE_SZ, 0);
    iwrite(&superblock, superblock.sb, BLOCK_SZ);
  }
}
// ���ò���
void test(int block_id) {
    memset(buf, 0, sizeof(buf));
    iread(buf, BLOCK_SZ, block_id*BLOCK_SZ);
    puts(buf);
}
// д���ֻ��ָ��д�ļ��Ĵ�С������û�о������ݡ�ÿһ�鶼�ɲ����ַ����������
void write() {
  if (tokens[1] == NULL || tokens[2] == NULL) {
    printf("write: write filename blocks eg write 1.txt 10\n");
  } else {
    int sz = -1; // �ȶ�ȡҪд���̿��С
    sscanf(tokens[2], "%d", &sz);
    // ��δʵ�ּ�i�ڵ�Ľ�����������һ���ļ����ֻ��д10��
    if (sz == -1 || (sz <= 10 && superblock.block_cap < sz) || sz > 10) {
      printf("write: blocks not enough or lv1+ allocation not implemented\n");
      return;
    }
    UFD ufd; // ��Ҫд���ļ��Ƿ����
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
    int fid = ufd.find(tokens[1]);
    if (fid < 0) {
      printf("write: file not found\n");
      return;
    } // ��ȡi�ڵ�
    int inode_id = ufd.files[fid].pt;
    int block_id = 0;
    int i; Inode new_inode;
    iread(&new_inode, INODE_SZ, superblock.inode+inode_id*INODE_SZ, 0);
    // �ļ������������ݣ������
    if (sz < new_inode.sz) deleteBlock(new_inode);
    // ��պ���д
    for (i = 0; i < sz ; ++i) {
      block_id = newBlock(block_id);
      new_inode.addr[i] = block_id;
      iwrite(sample, BLOCK_SZ, block_id*BLOCK_SZ, 0);
    }
    // �����̿����λ��
    new_inode.addr[i] = -1;
    new_inode.update(sz);
    // ����
    iwrite(bitmap_block, BLOCK_BITMAP_BLOCKS*BLOCK_SZ, superblock.block_bitmap, 0);
    iwrite(&new_inode, INODE_SZ, superblock.inode+inode_id*INODE_SZ, 0);
    iwrite(&superblock, BLOCK_SZ, superblock.sb);
  }
}
void read() {
  if (tokens[1] == NULL) {
      printf("read: read filename\n");
  } else {
    UFD ufd; // ���ļ��Ƿ����
    iread(&ufd, UFD_SZ, superblock.ufd+cuid*UFD_SZ);
    int fid = ufd.find(tokens[1]);
    if (fid < 0) {
      printf("read: file not found\n");
      return;
    } // ��ȡi�ڵ�
    int inode_id = ufd.files[fid].pt;
    int i; Inode inode;
    iread(&inode, INODE_SZ, superblock.inode+inode_id*INODE_SZ, 0);
    // ����i�ڵ���addr��ָ����̿�ţ���ȡ�̿��е�����
    for (i = 0; i < inode.blocks ; ++i) {
      memset(buf, 0, sizeof(buf));
      iread(buf, BLOCK_SZ, inode.addr[i] * BLOCK_SZ, 0);
      printf("%s", buf);
    }
    iclose();
  }
}
// ��������
void cmdHandler() {
  if (strcmp(tokens[0], "login") == 0) {
    login();
  } else if (uid == -1) { // δ��½ʱ��ֻ�ܿ�����Щ�û�
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
// �˳�
int leave() {
    if (strcmp(tokens[0], "exit") == 0) {
        printf("goodbye!\n");
        return 1;
    } else return 0;
}

LV2(user & file) Content File System

disk size 100M
block size 1KB

Structure
- superblock
- MFD
- UFD
- inode
- inode bitmap
- block bitmap
- data

Command
- login username
  admin reseved
- logout
- dir
- read filename
- write filename filesize
  all contents will be "sample text\n"
  filesize in block
  max file size = 10
  eg. write a.txt 5
- create [-u] username|filename
  max user = 8
  max file per user = 256
- delete [-u] username|filename
- copy destfile srcfile
- info [filename]

mix-indexed inode not implemented
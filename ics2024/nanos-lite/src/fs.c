#include <fs.h>

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);


typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, NULL},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int w = cfg.width, h = cfg.height;
  file_table[FD_FB].size = w * h * sizeof(uint32_t);
}

#define NR_FILES 24
size_t ramdisk_write(const void * buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);


int fs_open(const char *pathname, int flags, int mode){
  // printf("Pathname is %s\n", pathname);
  for(int i = 0; i < (sizeof(file_table)/sizeof(file_table[0])); i++){
    // printf("File_table[%d].name is %s\n", i, file_table[i].name);
    if(strcmp(file_table[i].name, pathname) == 0){
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("No such file %s", pathname);
}

int fs_close(int fd){
  return 0;
}

size_t fs_read(int fd, void *buf, size_t len){
  ReadFn read = file_table[fd].read;
  if(read != NULL){
    return read(buf, 0, len);
  }

  if(fd <= 2){
    Log("Fd is %d now.\n", fd);
    return 0;
  }

  size_t open_offset = file_table[fd].open_offset; // open_offset is the offset of the file that can be read
  size_t read_len = len; // read_len is the length of the data that can be read
  size_t disk_offset = file_table[fd].disk_offset; 
  size_t size = file_table[fd].size;
  if(open_offset > size){
    return 0;
  }
  if(open_offset + read_len > size){
    read_len = size - open_offset;
  }
  ramdisk_read(buf, disk_offset + open_offset, read_len); // read data from disk
  file_table[fd].open_offset += read_len; // update the open_offset
  return read_len; // return the length of the data that has been read
}

size_t fs_write(int fd, const void *buf, size_t len){
  WriteFn write = file_table[fd].write;
  if(write != NULL){
    return write(buf, 0, len);
  }

  
  if(fd == 0){
    Log("Ignore this write %s", file_table[fd].name); // ignore the general sign
    return 0;
  }
  if(fd == 1 || fd == 2){ // if the fd is 1 or 2, write the data to the serial port
    for(size_t i = 0; i < len; i++){
      putch(((char *)buf)[i]);
    }
    return len;
  }

  // 忘记了加fd == 3的情况
  if(fd == 3){
    fb_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
    return len;
  }


  size_t open_offset = file_table[fd].open_offset; // open_offset is the offset of the file that can be written
  size_t write_len = len; // write_len is the length of the data that can be written
  size_t disk_offset = file_table[fd].disk_offset;
  size_t size = file_table[fd].size;
  if(open_offset > size){
    return 0;
  }
  if(open_offset + write_len > size){
    write_len = size - open_offset;
  }
  ramdisk_write(buf, disk_offset + open_offset, write_len); // write data to disk
  file_table[fd].open_offset += write_len; // update the open_offset
  return write_len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  if(fd <= 2){
    Log("Ignore this lseek %s please.\n", file_table[fd].name); // ignore the general sign
    return 0;
  }
  Finfo* file = &file_table[fd];
  size_t new_offset;
  switch(whence) {
    case SEEK_SET:
      new_offset = offset;
      break;
    case SEEK_CUR:
      new_offset = file->open_offset + offset;
      break;
    case SEEK_END:
      new_offset = file->size + offset;
      break;
    default:
      panic("Invalid whence %d", whence);
  }
  if(new_offset > file->size || new_offset < 0){
    Log("Overflow. This place is out of boundary.\n");
    return -1;
  } 

  file->open_offset = new_offset;
  return new_offset;

}
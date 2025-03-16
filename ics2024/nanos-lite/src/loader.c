#include <proc.h>
#include <elf.h>

size_t ramdisk_read(void *buf, size_t offset, size_t len);

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_open(const char *pathname, int flags, int mode);


static uintptr_t loader(PCB *pcb, const char *filename) {
  // TODO();
  int fd = fs_open(filename, 0, 0);
  if(fd < 0){
    panic("No such file %s", filename);
  }
  Elf_Ehdr ehdr;

  assert(fs_read(fd, &ehdr, sizeof(ehdr)) == sizeof(ehdr)); // read ELF header
  assert(*(uint32_t *)&ehdr.e_ident == 0x464c457f); // ELF header Check

  Elf_Phdr phdr;
  for(int i = 0; i < ehdr.e_phnum; i++) {
    uint32_t base = ehdr.e_phoff + i * ehdr.e_phentsize;
    fs_lseek(fd, base, 0);

    assert(fs_read(fd, &phdr, ehdr.e_phentsize) == ehdr.e_phentsize); // read program header
    
    if(phdr.p_type == PT_LOAD) {
      char *buf_malloc = (char *)malloc(phdr.p_memsz); // allocate memory to replace ramdisk_read  
      fs_lseek(fd, phdr.p_offset, 0);
      
      assert(fs_read(fd, buf_malloc, phdr.p_filesz) == phdr.p_filesz); // read data from disk
      memcpy((void*)phdr.p_vaddr, buf_malloc, phdr.p_filesz); // copy data to memory
      memset((void*)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz); // set the rest of the memory to 0
      free(buf_malloc); // free the memory
    }
  }
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define ALIGNMENT(x) (((x) + 0x0F) & ~0x0F)

union NODE{
  struct {
    size_t size_block;
    unsigned flags_free;
    union NODE *next;
  } METADATA;
  char aligment[16];
};

// comming soon untuk yang arena model 
union MEMORY_CHUNK {
  union NODE *start_arena;
  uint32_t block_count;
};

void *HEAD = NULL,*TAIL = NULL;

/*!
 * @brief s_break() ini fungsinya hanya untuk update memory dan reserve brk()
 * kenapa saya memakai current_block yang selalu di call brk(),karena disini hanya untuk
 * melihat dimana break terakhir di update
 */
void *s_break(size_t size){
  void *current_block = (void *)syscall(SYS_brk,NULL);
  void *modified_block = (char *)current_block + ALIGNMENT(size);
  if((void *)syscall(SYS_brk,modified_block) == (void *)-1){
    fprintf(stderr,"FAILED : error to call sys_brk()\n");
    return NULL;
  }
  if(!HEAD) HEAD = current_block;
  TAIL = modified_block;
  return current_block;
}

typedef union NODE NODE_T;

// idk apa butuh static bss gitu buat simpan metadata HEAD dan TAILnya?
NODE_T *HEAD_INTERFACE = NULL,*TAIL_INTERFACE = NULL;

NODE_T *search_block_free(size_t size){
  NODE_T *current_block = HEAD_INTERFACE;
  while(current_block){
    if(current_block->METADATA.flags_free && current_block->METADATA.size_block >= size){
      return current_block;
    }
    current_block = current_block->METADATA.next;
  }
  return NULL;
}
/// @brief Alokasikan memory sesuai dengan size N
void *smalloc(size_t n){
  NODE_T *block = search_block_free(n);
  if(block){
    block->METADATA.flags_free = 0;
    return (void *)((char *)block + sizeof(NODE_T));
  }

  void *new_mem = s_break(n + sizeof(NODE_T));
  if(!new_mem) return NULL;

  NODE_T *new_block = (NODE_T *)new_mem;
  new_block->METADATA.flags_free = 0;
  new_block->METADATA.size_block = ALIGNMENT(n);
  new_block->METADATA.next = NULL;

  if(!HEAD_INTERFACE){
    HEAD_INTERFACE = new_block;
    TAIL_INTERFACE = new_block;
  }
  else{
    TAIL_INTERFACE->METADATA.next = new_block;
    TAIL_INTERFACE = new_block;
  }
  return (void *)((char *)new_block + sizeof(NODE_T));
}

/*!
 * @brief sfree() fungsi untuk membebaskan atau menandai block memory kosong,disini saya tidak membuat yang
 * versi ketika kita membebaskan memory disaat semisal block nya available untuk di bebaskan
 * disini sfree() fungsinya terlalu overhead karena terus terusan memanggil s_break() untuk return ke kernel
 */

void sfree(void *ptr){
  if(!ptr) return;
  NODE_T *block = (NODE_T *)((char *)ptr - sizeof(NODE_T));
  if(block){
    block->METADATA.flags_free = 1;
  }
  if(block == TAIL_INTERFACE){
    NODE_T *prev = NULL;
    NODE_T *current_block = HEAD_INTERFACE;
    while(current_block && current_block->METADATA.next != block) current_block = current_block->METADATA.next;
    prev = current_block;

    if(prev){
      prev->METADATA.next = NULL;
      TAIL_INTERFACE = prev;
      if((void *)syscall(SYS_brk,(void *)block) == (void *)-1){
        return;
      }
    }
    else {
      HEAD_INTERFACE = TAIL_INTERFACE = NULL;
      HEAD = TAIL = NULL;
      if((void *)syscall(SYS_brk,(void *)block) == (void *)-1){
        return;
      }
    }
  }
}

/**
 * @brief sdump_size berfungsi untuk me "seeking" memory size dengan cast ke metadatanya
 */
size_t sdump_size(void **ptr){
  if(!*ptr) return 0;
  union NODE *block = (union NODE *)((char *)(*ptr) - sizeof(union NODE));
  return block->METADATA.size_block;
}

int main(){
  char *buffer = smalloc(32);
  const char *str = "Hello OwO liking femboys isnt gay\n";
  memcpy(buffer,str,strlen(str) + 1);
  printf("%s",buffer); // print hasil dulu sebelum di free disini tidak akan segfault OwO
  sfree(buffer);
  return 0;
}

/*
 * Karena memory allocator kali ini tidak ada set NULL buat free blocknya
 * jadi semisal kita mau mengakses seperti kode di bawah ini akan terjadi 
 * segfault.
*/

// int main(){ 
//   char *buffer = smalloc(32);
//   const char *str = "Hello OwO liking femboys isnt gay\n";
//   memcpy(buffer,str.strlen(str) + 1);
//   sfree(buffer);
//   printf("%s",buffer);
//   return 0;
// }

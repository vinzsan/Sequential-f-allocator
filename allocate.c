#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#define FLAGS 1L // idk wtf i define tshit
#define ALIGN(x) (((x) + 15) & ~0xF)

typedef char align_metadata[16];

union NODE {
  struct {
    size_t size;
    unsigned is_free;
    union NODE *next;
  } METADATA;
  align_metadata stub;
};

typedef union NODE NODE_t ;

__attribute__((hot,used))
void *s_brk(void *ptr){
  void *ret_brk = NULL;
  __asm__ __volatile__(
    "syscall\n\t"
    :"=a"(ret_brk)
    :"a"(12),"D"(ptr)
    :"rsi","rdx","rcx","r11","memory"
  );
  return ret_brk;
}

static void *h = NULL,*t = NULL;

void *test_expand(size_t n){
  if(h == NULL){
    h = s_brk(NULL);
    if(h == (void *)-1 || h == NULL){
      fprintf(stderr,"ERROR : failed to initialize head\n");
      _exit(1);
    }
    t = (char *)h + ALIGN(n);
    s_brk(t);
    return h;
  }
  void *old_brk = t;
  t = (char *)t + ALIGN(n);
  if(s_brk(t) == (void *)-1){
    fprintf(stderr,"ERROR : failed test_expand() %s\n",strerror(errno));
    return NULL;
  }
  //h = old_brk;
  return old_brk;
}

static NODE_t *head = NULL,*tail = NULL;

NODE_t *src_block_free(size_t n){
  NODE_t *current = head;
  while(current){
    if(current->METADATA.is_free && current->METADATA.size >= n){
      return current;
    }
    current = current->METADATA.next;
  }
  return NULL;
}

void *expand_with_metadata(size_t n){
  n = ALIGN(n);

  NODE_t *block = src_block_free(n);
  if(block){
    block->METADATA.is_free = 0;
    return (char *)block + sizeof(NODE_t);
  }
  void *mem = test_expand(n + sizeof(NODE_t));
  if(!mem) return NULL;

  NODE_t* new_block = (NODE_t *)mem;
  new_block->METADATA.size = n;
  new_block->METADATA.is_free = 0;
  new_block->METADATA.next = NULL;
  if(!head){
    head = new_block;
    tail = new_block;
  }
  else {
    tail->METADATA.next = new_block;
    tail = new_block;
  }
  return (char *)new_block + sizeof(NODE_t);
}

void free_with_metadata(void *ptr){
  if(!ptr){
    fprintf(stderr,"??? : metadata not found %p\n",ptr);
    return;
  }
  NODE_t *free_ptr = (NODE_t *)((char *)ptr - sizeof(NODE_t));
  free_ptr->METADATA.is_free = 1;
  if(free_ptr == tail){
    s_brk((void *)free_ptr);
    NODE_t *prev = head;
    while(prev && prev->METADATA.next != free_ptr) prev = prev->METADATA.next;
    if(prev) prev->METADATA.next = NULL;
    tail = prev;
  }
}
// void free_with_metadata(void *ptr){
//   NODE_t *free_brk = (NODE_t *)((char *)ptr - sizeof(NODE_t));
//   if(free_brk->METADATA.is_free == 0){
//     free_brk->METADATA.is_free = 1;
//   }
//   s_brk((void *)((char *)free_brk + sizeof(NODE_t)));
// }

int main(){

  char *ptr = expand_with_metadata(1024);
  printf("PTR : %p\n",ptr);
  char *ptr1 = expand_with_metadata(1024);
  printf("PTR : %p\n",ptr1);
  free_with_metadata(ptr);
  free_with_metadata(ptr1);
  printf("PTR : %p\n",expand_with_metadata(1024));
  printf("PTR : %p\n",expand_with_metadata(32));
  char *ptr3 = expand_with_metadata(10);
  NODE_t *dummy = ((NODE_t *)((char *)ptr3 - sizeof(NODE_t)));
  printf("DUMMY SIZE : %zu\nFLAGS : %d\n",dummy->METADATA.size,dummy->METADATA.is_free);
  free_with_metadata(ptr3);
  printf("FLAGS : %d\n",dummy->METADATA.is_free);
  printf("PTR : %p\n",ptr);
  return 0;
}

#include <stdio.h>
#include <string.h>

struct RumahKuning{
  char nama_siswa[32];
  size_t jumlah;
};

void read_semua(struct RumahKuning *rk){
  printf("Nama siswa : %s\n",rk->nama_siswa);
  printf("Jumlah siswa : %zu\n",rk->jumlah);
  rk->jumlah = 5;
}

void read_semua_copy(struct RumahKuning rk){
  printf("Jumlah : %zu\n",rk.jumlah);
  printf("Nama siswa : %s\n",rk.nama_siswa);
  rk.jumlah = 15;
}

int main(){
  struct RumahKuning rk;
  strcpy(rk.nama_siswa,"Wira");
  rk.jumlah = 10;
  read_semua(&rk);
  printf("JUMLAH MENJADI : %zu\n",rk.jumlah);
  read_semua_copy(rk);
  printf("JUMLAH MENJADI : %zu\n",rk.jumlah);
  return 0;
}

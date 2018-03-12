#include "stdio.h"
#include "stdlib.h"


#define u64 unsigned long long


int crypto_stream_speck128192ctr_ref(
  unsigned char *out, 
  unsigned long long outlen, 
  const unsigned char *n, 
  const unsigned char *k
)
{
  u64 i,nonce[2],K[3],key[33],x,y,t;
  unsigned char *block=malloc(16);   

  if (!outlen) {free(block); return 0;}

  nonce[0]=((u64*)n)[0];
  nonce[1]=((u64*)n)[1];

  for(i=0;i<3;i++) K[i]=((u64 *)k)[i];

  ExpandKey(K,key);

  t=0;
  while(outlen>=16){
    x=nonce[1]; y=nonce[0]; nonce[0]++;
    Encrypt(&x,&y,key);                          
    ((u64 *)out)[1+t]=x; 
    ((u64 *)out)[0+t]=y; 
    t+=2;                                      
    outlen-=16;                                  
  }

  if (outlen>0){
    x=nonce[1]; y=nonce[0];
    Encrypt(&x,&y,key);
    ((u64 *)block)[1]=x; ((u64 *)block)[0]=y;
    for(i=0;i<outlen;i++) out[i+8*t]=block[i];
  }

  free(block);

  return 0;
}



int crypto_stream_speck128192ctr_ref_xor(
  unsigned char *out, 
  const unsigned char *in, 
  unsigned long long inlen, 
  const unsigned char *n, 
  const unsigned char *k)
{
  u64 i,nonce[2],K[3],key[33],x,y,t;
  unsigned char *block=malloc(16);  
 
  if (!inlen) {free(block); return 0;}

  nonce[0]=((u64*)n)[0];
  nonce[1]=((u64*)n)[1];

  for(i=0;i<3;i++) K[i]=((u64 *)k)[i];

  ExpandKey(K,key);

  t=0;
  while(inlen>=16){
    x=nonce[1]; y=nonce[0]; nonce[0]++;
    Encrypt(&x,&y,key);                          
    ((u64 *)out)[1+t]=x^((u64 *)in)[1+t]; 
    ((u64 *)out)[0+t]=y^((u64 *)in)[0+t]; 
    t+=2;                                      
    inlen-=16;                                  
  }
  if (inlen>0){
    x=nonce[1]; y=nonce[0];
    Encrypt(&x,&y,key);
    ((u64 *)block)[1]=x; ((u64 *)block)[0]=y;
    for(i=0;i<inlen;i++) out[i+8*t]=block[i]^in[i+8*t];
  }

  free(block);

  return 0;
}


#define ROR64(x,r) (((x)>>(r))|((x)<<(64-(r))))
#define ROL64(x,r) (((x)<<(r))|((x)>>(64-(r))))
#define R(x,y,k) (x=ROR64(x,8), x+=y, x^=k, y=ROL64(y,3), y^=x)
#define RI(x,y,k) (y^=x, y=ROR64(y,3), x^=k, x-=y, x=ROL64(x,8))

int Encrypt(u64 *u,u64 *v,u64 key[])
{
  u64 i,x=*u,y=*v;

  for(i=0;i<33;i++) R(x,y,key[i]);

  *u=x; *v=y;

  return 0;
}



int Decrypt(u64 *u,u64 *v,u64 key[])
{
  int i;
  u64 x=*u,y=*v;

  for(i=32;i>=0;i--) RI(x,y,key[i]);

  *u=x; *v=y;

  return 0;
}



int ExpandKey(u64 K[],u64 key[])
{
  u64 i,C=K[2],B=K[1],A=K[0];

  for(i=0;i<32;i+=2){
    key[i]=A; R(B,A,i);
    key[i+1]=A; R(C,A,i+1);
  }
  key[32]=A;

  return 0;
}

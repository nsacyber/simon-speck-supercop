#include "stdio.h"
#include "stdlib.h"

#define u32 unsigned


int crypto_stream_speck6496ctr_ref(
  unsigned char *out, 
  unsigned long long outlen, 
  const unsigned char *n, 
  const unsigned char *k
)
{
  u32 i,nonce[2],K[3],key[26],x,y,t;
  unsigned char *block=malloc(8);   

  if (!outlen) {free(block); return 0;}

  nonce[0]=((u32*)n)[0];
  nonce[1]=((u32*)n)[1];

  for(i=0;i<3;i++) K[i]=((u32 *)k)[i];

  ExpandKey(K,key);

  t=0;
  while(outlen>=8){
    x=nonce[1]; y=nonce[0]; nonce[0]++;
    Encrypt(&x,&y,key);                          
    ((u32 *)out)[1+t]=x; 
    ((u32 *)out)[0+t]=y; 
    t+=2;                                      
    outlen-=8;                                  
  }

  if (outlen>0){
    x=nonce[1]; y=nonce[0];
    Encrypt(&x,&y,key);
    ((u32 *)block)[1]=x; ((u32 *)block)[0]=y;
    for(i=0;i<outlen;i++) out[i+4*t]=block[i];
  }

  free(block);

  return 0;
}



int crypto_stream_speck6496ctr_ref_xor(
  unsigned char *out, 
  const unsigned char *in, 
  unsigned long long inlen, 
  const unsigned char *n, 
  const unsigned char *k)
{
  u32 i,nonce[2],K[3],key[26],x,y,t;
  unsigned char *block=malloc(8);  
 
  if (!inlen) {free(block); return 0;}

  nonce[0]=((u32*)n)[0];
  nonce[1]=((u32*)n)[1];

  for(i=0;i<3;i++) K[i]=((u32 *)k)[i];

  ExpandKey(K,key);

  t=0;
  while(inlen>=8){
    x=nonce[1]; y=nonce[0]; nonce[0]++;
    Encrypt(&x,&y,key);                          
    ((u32 *)out)[1+t]=x^((u32 *)in)[1+t]; 
    ((u32 *)out)[0+t]=y^((u32 *)in)[0+t]; 
    t+=2;                                      
    inlen-=8;                                  
  }
  if (inlen>0){
    x=nonce[1]; y=nonce[0];
    Encrypt(&x,&y,key);
    ((u32 *)block)[1]=x; ((u32 *)block)[0]=y;
    for(i=0;i<inlen;i++) out[i+4*t]=block[i]^in[i+4*t];
  }

  free(block);

  return 0;
}


#define ROR32(x,r) (((x)>>(r))|((x)<<(32-(r))))
#define ROL32(x,r) (((x)<<(r))|((x)>>(32-(r))))
#define R(x,y,k) (x=ROR32(x,8), x+=y, x^=k, y=ROL32(y,3), y^=x)
#define RI(x,y,k) (y^=x, y=ROR32(y,3), x^=k, x-=y, x=ROL32(x,8))

int Encrypt(u32 *u,u32 *v,u32 key[])
{
  u32 i,x=*u,y=*v;

  for(i=0;i<26;i++) R(x,y,key[i]);

  *u=x; *v=y;

  return 0;
}



int Decrypt(u32 *u,u32 *v,u32 key[])
{
  int i;
  u32 x=*u,y=*v;

  for(i=25;i>=0;i--) RI(x,y,key[i]);

  *u=x; *v=y;
 
  return 0;
}



int ExpandKey(u32 K[],u32 key[])
{
  u32 i,C=K[2],B=K[1],A=K[0];

  for(i=0;i<26;i+=2){
    key[i]=A; R(B,A,i);
    key[i+1]=A; R(C,A,i+1);
  }

  return 0;
}

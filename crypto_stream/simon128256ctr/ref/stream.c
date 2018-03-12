#include "stdio.h"
#include "stdlib.h"


#define u64 unsigned long long


int crypto_stream_simon128256ctr_ref(
  unsigned char *out, 
  unsigned long long outlen, 
  const unsigned char *n, 
  const unsigned char *k
)
{
  u64 i,nonce[2],K[4],key[72],x,y,t;
  unsigned char *block=malloc(16);   

  if (!outlen) {free(block); return 0;}

  nonce[0]=((u64*)n)[0];
  nonce[1]=((u64*)n)[1];

  for(i=0;i<4;i++) K[i]=((u64 *)k)[i];

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



int crypto_stream_simon128256ctr_ref_xor(
  unsigned char *out, 
  const unsigned char *in, 
  unsigned long long inlen, 
  const unsigned char *n, 
  const unsigned char *k)
{
  u64 i,nonce[2],K[4],key[72],x,y,t;
  unsigned char *block=malloc(16);  
 
  if (!inlen) {free(block); return 0;}

  nonce[0]=((u64*)n)[0];
  nonce[1]=((u64*)n)[1];

  for(i=0;i<4;i++) K[i]=((u64 *)k)[i];

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
#define f(x) ((ROL64(x,1) & ROL64(x,8)) ^ ROL64(x,2))
#define R2(x,y,k,l) (y^=f(x), y^=k, x^=f(y), x^=l)

int Encrypt(u64 *u,u64 *v,u64 key[])
{
  u64 i,x=*u,y=*v;

  for(i=0;i<72;i+=2) R2(x,y,key[i],key[i+1]);

  *u=x; *v=y;

  return 0;
}



int Decrypt(u64 *u,u64 *v,u64 key[])
{
  int i;
  u64 x=*u,y=*v;

  for(i=71;i>=0;i-=2) R2(y,x,key[i],key[i-1]);

  *u=x; *v=y;

  return 0;
}



int ExpandKey(u64 K[],u64 key[])
{
  u64 i,c=0xfffffffffffffffcLL;
  u64 z=0xfdc94c3a046d678bLL;

  key[0]=K[0]; key[1]=K[1]; key[2]=K[2]; key[3]=K[3];

  for(i=4;i<68;i++){
    key[i]=c^(z&1)^key[i-4]^ROR64(key[i-1],3)^key[i-3]^ROR64(key[i-1],4)^ROR64(key[i-3],1);
    z>>=1;
  }
  key[68]=c^key[64]^ROR64(key[67],3)^key[65]^ROR64(key[67],4)^ROR64(key[65],1);
  key[69]=c^1^key[65]^ROR64(key[68],3)^key[66]^ROR64(key[68],4)^ROR64(key[66],1);
  key[70]=c^key[66]^ROR64(key[69],3)^key[67]^ROR64(key[69],4)^ROR64(key[67],1);
  key[71]=c^key[67]^ROR64(key[70],3)^key[68]^ROR64(key[70],4)^ROR64(key[68],1);

  return 0;
}

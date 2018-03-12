#include "stdio.h"
#include "stdlib.h"

#define u32 unsigned
#define u64 unsigned long long


int crypto_stream_simon64128ctr_ref(
  unsigned char *out, 
  unsigned long long outlen, 
  const unsigned char *n, 
  const unsigned char *k
)
{
  u32 i,nonce[2],K[4],key[44],x,y,t;
  unsigned char *block=malloc(8);   

  if (!outlen) {free(block); return 0;}

  nonce[0]=((u32*)n)[0];
  nonce[1]=((u32*)n)[1];

  for(i=0;i<4;i++) K[i]=((u32 *)k)[i];

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



int crypto_stream_simon64128ctr_ref_xor(
  unsigned char *out, 
  const unsigned char *in, 
  unsigned long long inlen, 
  const unsigned char *n, 
  const unsigned char *k)
{
  u32 i,nonce[2],K[4],key[44],x,y,t;
  unsigned char *block=malloc(8);  
 
  if (!inlen) {free(block); return 0;}

  nonce[0]=((u32*)n)[0];
  nonce[1]=((u32*)n)[1];

  for(i=0;i<4;i++) K[i]=((u32 *)k)[i];

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
#define f(x) ((ROL32(x,1) & ROL32(x,8)) ^ ROL32(x,2))
#define R2(x,y,k,l) (y^=f(x), y^=k, x^=f(y), x^=l)

int Encrypt(u32 *u,u32 *v,u32 key[])
{
  u32 i,x=*u,y=*v;

  for(i=0;i<44;i+=2) R2(x,y,key[i],key[i+1]);

  *u=x; *v=y;

  return 0;
}



int Decrypt(u32 *u,u32 *v,u32 key[])
{
  int i;
  u32 x=*u,y=*v;

  for(i=43;i>=0;i-=2) R2(y,x,key[i],key[i-1]);

  *u=x; *v=y;

  return 0;
}



int ExpandKey(u32 K[],u32 key[])
{
  u32 i,c=0xfffffffc;
  u64 z=0xfc2ce51207a635dbLL;

  key[0]=K[0]; key[1]=K[1]; key[2]=K[2]; key[3]=K[3];

  for(i=4;i<44;i++){
    key[i]=c^(z&1)^key[i-4]^ROR32(key[i-1],3)^key[i-3]^ROR32(key[i-1],4)^ROR32(key[i-3],1);
    z>>=1;
  }

  return 0;
}

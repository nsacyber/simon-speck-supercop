//AVX512 C code for SPECK 64/128
/***********************************************************************************
 ** DISCLAIMER. THIS SOFTWARE WAS WRITTEN BY EMPLOYEES OF THE U.S.
 ** GOVERNMENT AS A PART OF THEIR OFFICIAL DUTIES AND, THEREFORE, IS NOT
 ** PROTECTED BY COPYRIGHT. THE U.S. GOVERNMENT MAKES NO WARRANTY, EITHER
 ** EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY IMPLIED WARRANTIES
 ** OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, REGARDING THIS SOFTWARE.
 ** THE U.S. GOVERNMENT FURTHER MAKES NO WARRANTY THAT THIS SOFTWARE WILL NOT
 ** INFRINGE ANY OTHER UNITED STATES OR FOREIGN PATENT OR OTHER
 ** INTELLECTUAL PROPERTY RIGHT. IN NO EVENT SHALL THE U.S. GOVERNMENT BE
 ** LIABLE TO ANYONE FOR COMPENSATORY, PUNITIVE, EXEMPLARY, SPECIAL,
 ** COLLATERAL, INCIDENTAL, CONSEQUENTIAL, OR ANY OTHER TYPE OF DAMAGES IN
 ** CONNECTION WITH OR ARISING OUT OF COPY OR USE OF THIS SOFTWARE.
 ***********************************************************************************/


#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Speck64128AVX512.h"

int crypto_stream_speck64128ctr_avx512(unsigned char *out, unsigned long long outlen, const unsigned char *n, const unsigned char *k);
inline __attribute__((always_inline)) int Encrypt(unsigned char *out, u32 nonce[], u512 rk[], u32 key[], int numbytes);
int crypto_stream_speck64128ctr_avx512_xor(unsigned char *out, const unsigned char *in, unsigned long long inlen, const unsigned char *n, const unsigned char *k);
inline __attribute__((always_inline)) int Encrypt_Xor(unsigned char *out, const unsigned char *in, u32 nonce[], u512 rk[], u32 key[], int numbytes);
int ExpandKey(u32 K[], u512 rk[], u32 key[]);



int crypto_stream_speck64128ctr_avx512(
  unsigned char *out, 
  unsigned long long outlen, 
  const unsigned char *n, 
  const unsigned char *k
)
{
  int i;
  u32 nonce[2],K[4],key[27],A,B,C,D,x,y;
  unsigned char block[8];
  u32 *const block32=(u32 *)block;
  u512 rk[27];

  if (!outlen) return 0;

  nonce[0]=((u32*)n)[0];
  nonce[1]=((u32*)n)[1];

  for(i=0;i<numkeywords;i++) K[i]=((u32 *)k)[i];

  if (outlen<=8){
    D=K[3]; C=K[2]; B=K[1]; A=K[0];
    x=nonce[1]; y=nonce[0]; nonce[0]++;
    for(i=0;i<numrounds;i+=3){
      Rx1b(x,y,A); Rx1b(B,A,i); 
      Rx1b(x,y,A); Rx1b(C,A,i+1);
      Rx1b(x,y,A); Rx1b(D,A,i+2);
    }
    block32[1]=x; block32[0]=y;
    for(i=0;i<outlen;i++) out[i]=block[i];

    return 0;
  }

  ExpandKey(K,rk,key);

  while(outlen>=512){
    Encrypt(out,nonce,rk,key,512);
    out+=512; outlen-=512;
  }

  if (outlen>=256){
    Encrypt(out,nonce,rk,key,256);
    out+=256; outlen-=256;
  }

  if (outlen>=128){
    Encrypt(out,nonce,rk,key,128);
    out+=128; outlen-=128;
  }

  if (outlen>=64){
    Encrypt(out,nonce,rk,key,64);
    out+=64; outlen-=64;
  }
  if (outlen>=32){
    Encrypt(out,nonce,rk,key,32);
    out+=32; outlen-=32;
  }

  if (outlen>=16){
    Encrypt(out,nonce,rk,key,16);
    out+=16; outlen-=16;
  }

  if (outlen>=8){
    Encrypt(out,nonce,rk,key,8);
    out+=8; outlen-=8;
  }

  if (outlen>0){ 
    Encrypt(block,nonce,rk,key,8);
    for (i=0;i<outlen;i++) out[i] = block[i];
  }

  return 0;
}



inline __attribute__((always_inline)) int Encrypt(unsigned char *out, u32 nonce[], u512 rk[], u32 key[], int numbytes)
{
  u32  x[2],y[2];
  u512 X[4],Y[4];
  unsigned char block1024[128];

  if (numbytes==8){
    x[0]=nonce[1]; y[0]=nonce[0]; nonce[0]++;
    Enc(x,y,key,1);
    ((u32 *)out)[1]=x[0]; ((u32 *)out)[0]=y[0];

    return 0;
  }

  if (numbytes==16){
    x[0]=nonce[1]; y[0]=nonce[0]; nonce[0]++;
    x[1]=nonce[1]; y[1]=nonce[0]; nonce[0]++;
    Enc(x,y,key,2);
    ((u32 *)out)[1]=x[0]; ((u32 *)out)[0]=y[0];
    ((u32 *)out)[3]=x[1]; ((u32 *)out)[2]=y[1];

    return 0;
  }

  if (numbytes==32){
    SET1(X[0],nonce[1]); SET16(Y[0],nonce[0]);
    Enc(X,Y,rk,16);
    nonce[0]+=(numbytes>>3);
    STORE(block1024,X[0],Y[0]);
    memcpy(out,block1024,32);

    return 0;
  }
  
  if (numbytes==64){
    SET1(X[0],nonce[1]); SET16(Y[0],nonce[0]);
    Enc(X,Y,rk,16);
    nonce[0]+=(numbytes>>3);
    STORE(block1024,X[0],Y[0]);
    memcpy(out,block1024,64);

    return 0;
  }
 

  SET1(X[0],nonce[1]); SET16(Y[0],nonce[0]);

  if (numbytes==128) Enc(X,Y,rk,16); 
  else{
    X[1]=X[0]; Y[1]=ADD(Y[0],_sixteen);
    if (numbytes==256) Enc(X,Y,rk,32); 
    else{
      X[2]=X[0]; Y[2]=ADD(Y[1],_sixteen);
      if (numbytes==384) Enc(X,Y,rk,48); 
      else{
	X[3]=X[0]; Y[3]=ADD(Y[2],_sixteen);
	Enc(X,Y,rk,64); 
      }
    }
  }

  nonce[0]+=(numbytes>>3);

  STORE(out,X[0],Y[0]);
  if (numbytes>=256) STORE(out+128,X[1],Y[1]);
  if (numbytes>=384) STORE(out+256,X[2],Y[2]);
  if (numbytes>=512) STORE(out+384,X[3],Y[3]);

  return 0;
}



int crypto_stream_speck64128ctr_avx512_xor(
  unsigned char *out, 
  const unsigned char *in, 
  unsigned long long inlen, 
  const unsigned char *n, 
  const unsigned char *k)
{
  int i;
  u32 nonce[2],K[4],key[27],A,B,C,D,x,y;
  unsigned char block[8];
  u32 *const block32=(u32 *)block;
  u64 *const block64=(u64 *)block;
  u512 rk[27];


  if (!inlen) return 0;

  nonce[0]=((u32*)n)[0];
  nonce[1]=((u32*)n)[1];

  for(i=0;i<numkeywords;i++) K[i]=((u32 *)k)[i];

  if (inlen<=8){
    D=K[3]; C=K[2]; B=K[1]; A=K[0];
    x=nonce[1]; y=nonce[0]; nonce[0]++;
    for(i=0;i<numrounds;i+=3){
      Rx1b(x,y,A); Rx1b(B,A,i); 
      Rx1b(x,y,A); Rx1b(C,A,i+1);
      Rx1b(x,y,A); Rx1b(D,A,i+2);
    }
    block32[1]=x; block32[0]=y;
    for(i=0;i<inlen;i++) out[i]=block[i]^in[i];

    return 0;
  }

  ExpandKey(K,rk,key);

  while(inlen>=512){
    Encrypt_Xor(out,in,nonce,rk,key,512);
    in+=512; inlen-=512; out+=512;
  }

  if (inlen>=256){
    Encrypt_Xor(out,in,nonce,rk,key,256);
    in+=256; inlen-=256; out+=256;
  }

  if (inlen>=128){
    Encrypt_Xor(out,in,nonce,rk,key,128);
    in+=128; inlen-=128; out+=128;
  }

  if (inlen>=64){
    Encrypt_Xor(out,in,nonce,rk,key,64);
    in+=64; inlen-=64; out+=64;
  }

  if (inlen>=32){
    Encrypt_Xor(out,in,nonce,rk,key,32);
    in+=32; inlen-=32; out+=32;
  }

  if (inlen>=16){
    Encrypt_Xor(out,in,nonce,rk,key,16);
    in+=16; inlen-=16; out+=16;
  }

  if (inlen>=8){
    Encrypt_Xor(block,in,nonce,rk,key,8);
    ((u64 *)out)[0]=block64[0]^((u64 *)in)[0];
    in+=8; inlen-=8; out+=8;
  }

  if (inlen>0){ 
    Encrypt_Xor(block,in,nonce,rk,key,8);
    for (i=0;i<inlen;i++) out[i]=block[i]^in[i];
  }

  return 0;
}



inline __attribute__((always_inline)) int Encrypt_Xor(unsigned char *out, const unsigned char *in, u32 nonce[], u512 rk[], u32 key[], int numbytes)
{
  u32  x[2],y[2];
  u512 X[4],Y[4];
  unsigned char block1024[128];



  if (numbytes==8){ 
    x[0]=nonce[1]; y[0]=nonce[0]; nonce[0]++;
    Enc(x,y,key,1);
    ((u32 *)out)[1]=x[0]; ((u32 *)out)[0]=y[0];

    return 0;
  }

  if (numbytes==16){
    x[0]=nonce[1]; y[0]=nonce[0]; nonce[0]++;
    x[1]=nonce[1]; y[1]=nonce[0]; nonce[0]++;
    Enc(x,y,key,2);
    ((u32 *)out)[1]=x[0]^((u32 *)in)[1]; ((u32 *)out)[0]=y[0]^((u32 *)in)[0];
    ((u32 *)out)[3]=x[1]^((u32 *)in)[3]; ((u32 *)out)[2]=y[1]^((u32 *)in)[2];

    return 0;
  }

  if (numbytes==32){
    SET1(X[0],nonce[1]); SET16(Y[0],nonce[0]);
    Enc(X,Y,rk,16);
    nonce[0]+=(numbytes>>3);
    memcpy(block1024,in,32);
    XOR_STORE(block1024,block1024,X[0],Y[0]);
    memcpy(out,block1024,32);

    return 0;
  }
  
  if (numbytes==64){
    SET1(X[0],nonce[1]); SET16(Y[0],nonce[0]);
    Enc(X,Y,rk,16);
    nonce[0]+=(numbytes>>3);
    memcpy(block1024,in,64);
    XOR_STORE(block1024,block1024,X[0],Y[0]);
    memcpy(out,block1024,64);

    return 0;
  }
 
  
  SET1(X[0],nonce[1]); SET16(Y[0],nonce[0]);

  if (numbytes==128) Enc(X,Y,rk,16); 
  else{
    X[1]=X[0]; Y[1]=ADD(Y[0],_sixteen);
    if (numbytes==256) Enc(X,Y,rk,32); 
    else{
      X[2]=X[0]; Y[2]=ADD(Y[1],_sixteen);
      if (numbytes==384) Enc(X,Y,rk,48); 
      else{
	X[3]=X[0]; Y[3]=ADD(Y[2],_sixteen);
	Enc(X,Y,rk,64); 
      }
    }
  }

  nonce[0]+=(numbytes>>3);

  XOR_STORE(in,out,X[0],Y[0]);
  if (numbytes>=256) XOR_STORE(in+128,out+128,X[1],Y[1]);
  if (numbytes>=384) XOR_STORE(in+256,out+256,X[2],Y[2]);
  if (numbytes>=512) XOR_STORE(in+384,out+384,X[3],Y[3]);

  return 0;
}



int ExpandKey(u32 K[], u512 rk[], u32 key[])
{
  u32 A=K[0],B=K[1],C=K[2],D=K[3]; 

  EK(A,B,C,D,rk,key);

  return 0;
}

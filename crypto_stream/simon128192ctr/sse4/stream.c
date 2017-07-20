//SSE4 C code for a counter mode implementation of SIMON 128/192.

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


#include <stdio.h>
#include <stdlib.h>
#include "Simon128192SSE4.h"


int crypto_stream_simon128192ctr_sse4(unsigned char *out, unsigned long long outlen, const unsigned char *n, const unsigned char *k);
inline __attribute__((always_inline)) int Encrypt(unsigned char *out,u64 nonce[],u128 rk[][8],u64 key[],int numbytes);
int crypto_stream_simon128192ctr_sse4_xor(unsigned char *out, const unsigned char *in, unsigned long long inlen, const unsigned char *n, const unsigned char *k);
inline __attribute__((always_inline)) int Encrypt_Xor(unsigned char *out, const unsigned char *in, u64 nonce[], u128 rk[][8], u64 key[], int numbytes);
int ExpandKeyBS(u64 K[],u128 rk[][8]);
int ExpandKeyNBS(u64 K[], u128 rk[][8], u64 key[]);
inline __attribute__((always_inline)) int Transpose(u128 M[]);



int crypto_stream_simon128192ctr_sse4(
  unsigned char *out,
  unsigned long long outlen,
  const unsigned char *n,
  const unsigned char *k
)
{
  int i;
  u64 nonce[2],K[4],key[72];
  unsigned char block[16];
  u128 rk[72][8];

  if (!outlen) return 0;

  nonce[0]=((u64*)n)[0];
  nonce[1]=((u64*)n)[1];

  for(i=0;i<numkeywords;i++) K[i]=((u64 *)k)[i];

  if (outlen>=256){
    ExpandKeyBS(K,rk);

    while(outlen>=256){
      Encrypt(out,nonce,rk,key,256);
      out+=256; outlen-=256;
    }
  }

  if (!outlen) return 0;

  ExpandKeyNBS(K,rk,key);

  while(outlen>=128){
    Encrypt(out,nonce,rk,key,128);
    out+=128; outlen-=128;
  }

  if (outlen>=96){
    Encrypt(out,nonce,rk,key,96);
    out+=96; outlen-=96;
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

  if (outlen>0){
    Encrypt(block,nonce,rk,key,16);
    for (i=0;i<outlen;i++) out[i] = block[i];
  }

  return 0;
}



inline __attribute__((always_inline)) int Encrypt(unsigned char *out, u64 nonce[], u128 rk[][8], u64 key[], int numbytes)
{
  u64  i,j,x[4],y[4];
  u128 X[8],Y[8];

  if (numbytes==16){
    x[0]=nonce[1]; y[0]=nonce[0]; nonce[0]++;
    Enc(x,y,key,1);
    ((u64 *)out)[1]=x[0]; ((u64 *)out)[0]=y[0];

    return 0;
  }

  SET1(X[0],nonce[1]); SET2(Y[0],nonce[0]);

  if (numbytes==32) Enc(X,Y,rk,2);
  else{
    X[1]=X[0]; Y[1]=ADD(Y[0],_two);
    if (numbytes==64) Enc(X,Y,rk,4);
    else{
      X[2]=X[0]; Y[2]=ADD(Y[1],_two);
      if (numbytes==96) Enc(X,Y,rk,6);
      else{
        X[3]=X[0]; Y[3]=ADD(Y[2],_two);
        if (numbytes==128) Enc(X,Y,rk,8);
	else{
	  X[4]=X[0]; Y[4]=ADD(Y[3],_two);
	  X[5]=X[0]; Y[5]=ADD(Y[4],_two);
	  X[6]=X[0]; Y[6]=ADD(Y[5],_two);
	  X[7]=X[0]; Y[7]=ADD(Y[6],_two);
	  Transpose(X); Transpose(Y);
	  for(i=0;i<68;i+=2){
	    for(j=2;j<8;j++) Y[j]=XOR(XOR(rk[i][j],Y[j]),XOR(AND(X[j-1],ROL8(X[j])),X[j-2]));
	    Y[0]=XOR(XOR(rk[i][0],Y[0]),ROL8(XOR(AND(X[7],X[0]),X[6])));
	    Y[1]=XOR(XOR(rk[i][1],Y[1]),XOR(AND(X[0],ROL8(X[1])),ROL8(X[7])));
	    for(j=2;j<8;j++) X[j]=XOR(XOR(rk[i+1][j],X[j]),XOR(AND(Y[j-1],ROL8(Y[j])),Y[j-2]));
	    X[0]=XOR(XOR(rk[i+1][0],X[0]),ROL8(XOR(AND(Y[7],Y[0]),Y[6])));
	    X[1]=XOR(XOR(rk[i+1][1],X[1]),XOR(AND(Y[0],ROL8(Y[1])),ROL8(Y[7])));
	  }
	  for(j=2;j<8;j++) Y[j]=XOR(XOR(rk[68][j],Y[j]),XOR(AND(X[j-1],ROL8(X[j])),X[j-2]));
	  Y[0]=XOR(XOR(rk[68][0],Y[0]),ROL8(XOR(AND(X[7],X[0]),X[6])));
	  Y[1]=XOR(XOR(rk[68][1],Y[1]),XOR(AND(X[0],ROL8(X[1])),ROL8(X[7])));
	  Transpose(X); Transpose(Y);
	}
      }
    }
  }

  nonce[0]+=(numbytes>>4);

  STORE(out,X[0],Y[0]);
  if (numbytes>=64)  STORE(out+32,X[1],Y[1]);
  if (numbytes>=96)  STORE(out+64,X[2],Y[2]);
  if (numbytes>=128) STORE(out+96,X[3],Y[3]);
  if (numbytes>=256){
    STORE(out+128,X[4],Y[4]);
    STORE(out+160,X[5],Y[5]);
    STORE(out+192,X[6],Y[6]);
    STORE(out+224,X[7],Y[7]);
  }

  return 0;
}



int crypto_stream_simon128192ctr_sse4_xor(
  unsigned char *out,
  const unsigned char *in,
  unsigned long long inlen,
  const unsigned char *n,
  const unsigned char *k
)
{
  u64 i,nonce[2],K[4],key[72];
  unsigned char block[16];
  u64 * const block64 = (u64 *)block;
  u128 rk[72][8];

  if (!inlen) return 0;

  nonce[0]=((u64 *)n)[0];
  nonce[1]=((u64 *)n)[1];

  for(i=0;i<numkeywords;i++) K[i]=((u64 *)k)[i];

  if (inlen>=256){
    ExpandKeyBS(K,rk);

    while(inlen>=256){
      Encrypt_Xor(out,in,nonce,rk,key,256);
      in+=256; inlen-=256; out+=256;
    }
  }

  if (!inlen) return 0;

  ExpandKeyNBS(K,rk,key);

  while(inlen>=128){
    Encrypt_Xor(out,in,nonce,rk,key,128);
    in+=128; inlen-=128; out+=128;
  }

  if (inlen>=96){
    Encrypt_Xor(out,in,nonce,rk,key,96);
    in+=96; inlen-=96; out+=96;
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
    Encrypt_Xor(block,in,nonce,rk,key,16);
    ((u64 *)out)[0]=block64[0]^((u64 *)in)[0];
    ((u64 *)out)[1]=block64[1]^((u64 *)in)[1];
    in+=16; inlen-=16; out+=16;
  }

  if (inlen>0){
    Encrypt_Xor(block,in,nonce,rk,key,16);
    for (i=0;i<inlen;i++) out[i]=block[i]^in[i];
  }

  return 0;
}



inline __attribute__((always_inline)) int Encrypt_Xor(unsigned char *out, const unsigned char *in, u64 nonce[], u128 rk[][8], u64 key[], int numbytes)
{
  u64  i,j,x[4],y[4];
  u128 X[8],Y[8];

  if (numbytes==16){
    x[0]=nonce[1]; y[0]=nonce[0]; nonce[0]++;
    Enc(x,y,key,1);
    ((u64 *)out)[1]=x[0]; ((u64 *)out)[0]=y[0];

    return 0;
  }

  SET1(X[0],nonce[1]); SET2(Y[0],nonce[0]);

  if (numbytes==32) Enc(X,Y,rk,2);
  else{
    X[1]=X[0]; Y[1]=ADD(Y[0],_two);
    if (numbytes==64) Enc(X,Y,rk,4);
    else{
      X[2]=X[0]; Y[2]=ADD(Y[1],_two);
      if (numbytes==96) Enc(X,Y,rk,6);
      else{
        X[3]=X[0]; Y[3]=ADD(Y[2],_two);
        if (numbytes==128) Enc(X,Y,rk,8);
	else{
	  X[4]=X[0]; Y[4]=ADD(Y[3],_two);
	  X[5]=X[0]; Y[5]=ADD(Y[4],_two);
	  X[6]=X[0]; Y[6]=ADD(Y[5],_two);
	  X[7]=X[0]; Y[7]=ADD(Y[6],_two);
	  Transpose(X); Transpose(Y);
	  for(i=0;i<68;i+=2){
	    for(j=2;j<8;j++) Y[j]=XOR(XOR(rk[i][j],Y[j]),XOR(AND(X[j-1],ROL8(X[j])),X[j-2]));
	    Y[0]=XOR(XOR(rk[i][0],Y[0]),ROL8(XOR(AND(X[7],X[0]),X[6])));
	    Y[1]=XOR(XOR(rk[i][1],Y[1]),XOR(AND(X[0],ROL8(X[1])),ROL8(X[7])));
	    for(j=2;j<8;j++) X[j]=XOR(XOR(rk[i+1][j],X[j]),XOR(AND(Y[j-1],ROL8(Y[j])),Y[j-2]));
	    X[0]=XOR(XOR(rk[i+1][0],X[0]),ROL8(XOR(AND(Y[7],Y[0]),Y[6])));
	    X[1]=XOR(XOR(rk[i+1][1],X[1]),XOR(AND(Y[0],ROL8(Y[1])),ROL8(Y[7])));
	  }
	  for(j=2;j<8;j++) Y[j]=XOR(XOR(rk[68][j],Y[j]),XOR(AND(X[j-1],ROL8(X[j])),X[j-2]));
	  Y[0]=XOR(XOR(rk[68][0],Y[0]),ROL8(XOR(AND(X[7],X[0]),X[6])));
	  Y[1]=XOR(XOR(rk[68][1],Y[1]),XOR(AND(X[0],ROL8(X[1])),ROL8(X[7])));
	  Transpose(X); Transpose(Y);
	}
      }
    }
  }

  nonce[0]+=(numbytes>>4);

  XOR_STORE(in,out,X[0],Y[0]);
  if (numbytes>=64)  XOR_STORE(in+32,out+32,X[1],Y[1]);
  if (numbytes>=96)  XOR_STORE(in+64,out+64,X[2],Y[2]);
  if (numbytes>=128) XOR_STORE(in+96,out+96,X[3],Y[3]);
  if (numbytes>=256){
    XOR_STORE(in+128,out+128,X[4],Y[4]);
    XOR_STORE(in+160,out+160,X[5],Y[5]);
    XOR_STORE(in+192,out+192,X[6],Y[6]);
    XOR_STORE(in+224,out+224,X[7],Y[7]);
  }

  return 0;
}


int ExpandKeyBS(u64 K[],u128 rk[][8])
{
  int i,j;

  for(i=0;i<3;i++){
    rk[i][0]=SET(K[i],K[i]);
    for(j=1;j<8;j++){
      rk[i][j]=rk[i][0];
    }
    Transpose(rk[i]);
  }

  EKBS(rk);

  return 0;
}



int ExpandKeyNBS(u64 K[], u128 rk[][8], u64 key[])
{
  u64 A=K[0], B=K[1], C=K[2];

  EKNBS(A,B,C,rk,key);

  return 0;
}



inline __attribute__((always_inline)) int Transpose(u128 T[8])
{
  u128 W[4];
  const u128 mask4 = SET(0x0f0f0f0f0f0f0f0fLL, 0x0f0f0f0f0f0f0f0fLL);
  const u128 mask2 = SET(0x3333333333333333LL, 0x3333333333333333LL);
  const u128 mask1 = SET(0x5555555555555555LL, 0x5555555555555555LL);


  W[0] = AND(XOR(SR(T[0],4),T[4]), mask4);
  T[4] = XOR(T[4], W[0]);
  W[0] = SL(W[0],4);
  T[0] = XOR(T[0], W[0]);

  W[1] = AND(XOR(SR(T[1],4),T[5]), mask4);
  T[5] = XOR(T[5], W[1]);
  W[1] = SL(W[1],4);
  T[1] = XOR(T[1], W[1]);

  W[2] = AND(XOR(SR(T[2],4),T[6]), mask4);
  T[6] = XOR(T[6], W[2]);
  W[2] = SL(W[2],4);
  T[2] = XOR(T[2], W[2]);

  W[3] = AND(XOR(SR(T[3],4),T[7]), mask4);
  T[7] = XOR(T[7], W[3]);
  W[3] = SL(W[3],4);
  T[3] = XOR(T[3], W[3]);

  W[0] = AND( XOR(SR(T[0],2),T[2]), mask2);
  T[2] = XOR(T[2], W[0]);
  T[0] = XOR(T[0], SL(W[0],2));

  W[1] = AND( XOR(SR(T[1],2),T[3]), mask2);
  T[3] = XOR(T[3], W[1]);
  T[1] = XOR(T[1], SL(W[1],2));

  W[2] = AND( XOR(SR(T[4],2),T[6]), mask2);
  T[6] = XOR(T[6], W[2]);
  T[4] = XOR(T[4], SL(W[2],2));

  W[3] = AND( XOR(SR(T[5],2),T[7]), mask2);
  T[7] = XOR(T[7], W[3]);
  T[5] = XOR(T[5], SL(W[3],2));

 
  W[0] = AND( XOR(SR(T[0],1),T[1]), mask1);
  T[1] = XOR(T[1], W[0]);
  T[0] = XOR(T[0], SL(W[0],1));

  W[1] = AND( XOR(SR(T[2],1),T[3]), mask1);
  T[3] = XOR(T[3], W[1]);
  T[2] = XOR(T[2], SL(W[1],1));

  W[2] = AND( XOR(SR(T[4],1),T[5]), mask1);
  T[5] = XOR(T[5], W[2]);
  T[4] = XOR(T[4], SL(W[2],1));
  
  W[3] = AND( XOR(SR(T[6],1),T[7]), mask1);
  T[7] = XOR(T[7], W[3]);
  T[6] = XOR(T[6], SL(W[3],1));
  
  return 0;
}


//AVX2 C code for SIMON 64/96

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
#include "Simon6496AVX2.h"


int crypto_stream_simon6496ctr_avx2(unsigned char *out, unsigned long long outlen, const unsigned char *n, const unsigned char *k);
int Encrypt(unsigned char *out, u32 nonce[], u256 rk[][8], u32 key[], int numbytes);
int crypto_stream_simon6496ctr_avx2_xor(unsigned char *out, const unsigned char *in, unsigned long long inlen, const unsigned char *n, const unsigned char *k);
int Encrypt_Xor(unsigned char *out, const unsigned char *in, u32 nonce[], u256 rk[][8], u32 key[], int numbytes);
int ExpandKeyBS(u32 K[],u256 rk[][8]);
int ExpandKeyNBS(u32 K[], u256 rk[][8], u32 key[]);
inline int Transpose(u256 M[]);



int crypto_stream_simon6496ctr_avx2(
  unsigned char *out,
  unsigned long long outlen,
  const unsigned char *n,
  const unsigned char *k
)
{
  int i;
  u32 nonce[2],K[4],key[44];
  unsigned char block[8];
  u256 rk[44][8];

  if (!outlen) return 0;

  nonce[0]=((u32*)n)[0];
  nonce[1]=((u32*)n)[1];

  for(i=0;i<numkeywords;i++) K[i]=((u32 *)k)[i];

  if (outlen>=512){
    ExpandKeyBS(K,rk);

    while(outlen>=512){
      Encrypt(out,nonce,rk,key,512);
      out+=512; outlen-=512;
    }
  }

  if (!outlen) return 0;

  ExpandKeyNBS(K,rk,key);

  while(outlen>=256){
    Encrypt(out,nonce,rk,key,256);
    out+=256; outlen-=256;
  }

  if (outlen>=192){
    Encrypt(out,nonce,rk,key,192);
    out+=192; outlen-=192;
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



int Encrypt(unsigned char *out, u32 nonce[], u256 rk[][8], u32 key[], int numbytes)
{
  u32  i,j,x[4],y[4];
  u256 X[8],Y[8];


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
    x[0]=nonce[1]; y[0]=nonce[0]; nonce[0]++;
    x[1]=nonce[1]; y[1]=nonce[0]; nonce[0]++;
    x[2]=nonce[1]; y[2]=nonce[0]; nonce[0]++;
    x[3]=nonce[1]; y[3]=nonce[0]; nonce[0]++;
    Enc(x,y,key,4);
    ((u32 *)out)[1]=x[0]; ((u32 *)out)[0]=y[0];
    ((u32 *)out)[3]=x[1]; ((u32 *)out)[2]=y[1];
    ((u32 *)out)[5]=x[2]; ((u32 *)out)[4]=y[2];
    ((u32 *)out)[7]=x[3]; ((u32 *)out)[6]=y[3];

    return 0;
  }

  SET1(X[0],nonce[1]); SET8(Y[0],nonce[0]);

  if (numbytes==64) Enc(X,Y,rk,8);
  else{
    X[1]=X[0]; Y[1]=ADD(Y[0],_eight);
    if (numbytes==128) Enc(X,Y,rk,16);
    else{
      X[2]=X[0]; Y[2]=ADD(Y[1],_eight);
      if (numbytes==192) Enc(X,Y,rk,24);
      else{
	X[3]=X[0]; Y[3]=ADD(Y[2],_eight);
	if (numbytes==256) Enc(X,Y,rk,32);
	else{
	  X[4]=X[0]; Y[4]=ADD(Y[3],_eight);
	  X[5]=X[0]; Y[5]=ADD(Y[4],_eight);
	  X[6]=X[0]; Y[6]=ADD(Y[5],_eight);
	  X[7]=X[0]; Y[7]=ADD(Y[6],_eight);
	  Transpose(X); Transpose(Y);
	  for(i=0;i<42;i+=2){
	    Y[0]=XOR(XOR(rk[i][0],Y[0]),ROL8(XOR(AND(X[7],X[0]),X[6])));
	    Y[1]=XOR(XOR(rk[i][1],Y[1]),XOR(AND(X[0],ROL8(X[1])),ROL8(X[7])));
	    for(j=2;j<8;j++) Y[j]=XOR(XOR(rk[i][j],Y[j]),XOR(AND(X[j-1],ROL8(X[j])),X[j-2]));
	    for(j=2;j<8;j++) X[j]=XOR(XOR(rk[i+1][j],X[j]),XOR(AND(Y[j-1],ROL8(Y[j])),Y[j-2]));
	    X[0]=XOR(XOR(rk[i+1][0],X[0]),ROL8(XOR(AND(Y[7],Y[0]),Y[6])));
	    X[1]=XOR(XOR(rk[i+1][1],X[1]),XOR(AND(Y[0],ROL8(Y[1])),ROL8(Y[7])));
	  }
	  Transpose(X); Transpose(Y);
	}
      }
    }
  }

  nonce[0]+=(numbytes>>3);

  STORE(out,X[0],Y[0]);
  if (numbytes>=128) STORE(out+64,X[1],Y[1]);
  if (numbytes>=192) STORE(out+128,X[2],Y[2]);
  if (numbytes>=256) STORE(out+192,X[3],Y[3]);
  if (numbytes>=512){
    STORE(out+256,X[4],Y[4]);
    STORE(out+320,X[5],Y[5]);
    STORE(out+384,X[6],Y[6]);
    STORE(out+448,X[7],Y[7]);
  }

  return 0;
}



int crypto_stream_simon6496ctr_avx2_xor(
  unsigned char *out,
  const unsigned char *in,
  unsigned long long inlen,
  const unsigned char *n,
  const unsigned char *k
)
{
  int i;
  u32 nonce[2],K[4],key[44];
  unsigned char block[8];
  u64 * const block64 = (u64 *)block;
  u256 rk[44][8];

  if (!inlen) return 0;

  nonce[0]=((u32*)n)[0];
  nonce[1]=((u32*)n)[1];

  for(i=0;i<numkeywords;i++) K[i]=((u32 *)k)[i];

  if (inlen>=512){
    ExpandKeyBS(K,rk);

    while(inlen>=512){
      Encrypt_Xor(out,in,nonce,rk,key,512);
      in+=512; inlen-=512; out+=512;
    }
  }

  if (!inlen) return 0;

  ExpandKeyNBS(K,rk,key);

  while(inlen>=256){
    Encrypt_Xor(out,in,nonce,rk,key,256);
    in+=256; inlen-=256; out+=256;
  }

  if (inlen>=192){
    Encrypt_Xor(out,in,nonce,rk,key,192);
    in+=192; inlen-=192; out+=192;
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



int Encrypt_Xor(unsigned char *out, const unsigned char *in, u32 nonce[], u256 rk[][8], u32 key[], int numbytes)
{
  u32  i,j,x[4],y[4];
  u256 X[8],Y[8];


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
    x[0]=nonce[1]; y[0]=nonce[0]; nonce[0]++;
    x[1]=nonce[1]; y[1]=nonce[0]; nonce[0]++;
    x[2]=nonce[1]; y[2]=nonce[0]; nonce[0]++;
    x[3]=nonce[1]; y[3]=nonce[0]; nonce[0]++;
    Enc(x,y,key,4);
    ((u32*)out)[1]=x[0]^((u32 *)in)[1]; ((u32*)out)[0]=y[0]^((u32 *)in)[0];
    ((u32*)out)[3]=x[1]^((u32 *)in)[3]; ((u32*)out)[2]=y[1]^((u32 *)in)[2];
    ((u32*)out)[5]=x[2]^((u32 *)in)[5]; ((u32*)out)[4]=y[2]^((u32 *)in)[4];
    ((u32*)out)[7]=x[3]^((u32 *)in)[7]; ((u32*)out)[6]=y[3]^((u32 *)in)[6];

    return 0;
  }

  SET1(X[0],nonce[1]); SET8(Y[0],nonce[0]);

  if (numbytes==64) Enc(X,Y,rk,8);
  else{
    X[1]=X[0]; Y[1]=ADD(Y[0],_eight);
    if (numbytes==128) Enc(X,Y,rk,16);
    else{
      X[2]=X[0]; Y[2]=ADD(Y[1],_eight);
      if (numbytes==192) Enc(X,Y,rk,24);
      else{
	X[3]=X[0]; Y[3]=ADD(Y[2],_eight);
	if (numbytes==256) Enc(X,Y,rk,32);
	else{
          X[4]=X[0]; Y[4]=ADD(Y[3],_eight);
	  X[5]=X[0]; Y[5]=ADD(Y[4],_eight);
	  X[6]=X[0]; Y[6]=ADD(Y[5],_eight);
	  X[7]=X[0]; Y[7]=ADD(Y[6],_eight);
	  Transpose(X); Transpose(Y);
	  for(i=0;i<42;i+=2){
	    Y[0]=XOR(XOR(rk[i][0],Y[0]),ROL8(XOR(AND(X[7],X[0]),X[6])));
	    Y[1]=XOR(XOR(rk[i][1],Y[1]),XOR(AND(X[0],ROL8(X[1])),ROL8(X[7])));
	    for(j=2;j<8;j++) Y[j]=XOR(XOR(rk[i][j],Y[j]),XOR(AND(X[j-1],ROL8(X[j])),X[j-2]));
	    for(j=2;j<8;j++) X[j]=XOR(XOR(rk[i+1][j],X[j]),XOR(AND(Y[j-1],ROL8(Y[j])),Y[j-2]));
	    X[0]=XOR(XOR(rk[i+1][0],X[0]),ROL8(XOR(AND(Y[7],Y[0]),Y[6])));
	    X[1]=XOR(XOR(rk[i+1][1],X[1]),XOR(AND(Y[0],ROL8(Y[1])),ROL8(Y[7])));
	  }
	  Transpose(X); Transpose(Y);
	}
      }
    }
  }

  nonce[0]+=(numbytes>>3);

  XOR_STORE(in,out,X[0],Y[0]);
  if (numbytes>=128) XOR_STORE(in+64,out+64,X[1],Y[1]);
  if (numbytes>=192) XOR_STORE(in+128,out+128,X[2],Y[2]);
  if (numbytes>=256) XOR_STORE(in+192,out+192,X[3],Y[3]);
  if (numbytes>=512){
    XOR_STORE(in+256,out+256,X[4],Y[4]);
    XOR_STORE(in+320,out+320,X[5],Y[5]);
    XOR_STORE(in+384,out+384,X[6],Y[6]);
    XOR_STORE(in+448,out+448,X[7],Y[7]);
  }

  return 0;
}



int ExpandKeyBS(u32 K[],u256 rk[][8])
{
  int i,j;

  for(i=0;i<3;i++){
    rk[i][0]=SET(K[i],K[i],K[i],K[i],K[i],K[i],K[i],K[i]);
    for(j=1;j<8;j++){
      rk[i][j]=rk[i][0];
    }
    Transpose(rk[i]);
  }

  EKBS(rk);

  return 0;
}



int ExpandKeyNBS(u32 K[], u256 rk[][8], u32 key[])
{
  u32 A=K[0], B=K[1], C=K[2];

  EKNBS(A,B,C,rk,key);

  return 0;
}



inline int Transpose(u256 M[])
{
  u256 W[4];
  const  u256 mask4 = SET(0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f);
  const  u256 mask2 = SET(0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333);
  const  u256 mask1 = SET(0x55555555,0x55555555,0x55555555,0x55555555,0x55555555,0x55555555,0x55555555,0x55555555);


  W[0] = AND(XOR(SR(M[0],4),M[4]), mask4);
  M[4] = XOR(M[4], W[0]);
  W[0] = SL(W[0],4);
  M[0] = XOR(M[0], W[0]);

  W[1] = AND(XOR(SR(M[1],4),M[5]), mask4);
  M[5] = XOR(M[5], W[1]);
  W[1] = SL(W[1],4);
  M[1] = XOR(M[1], W[1]);

  W[2] = AND(XOR(SR(M[2],4),M[6]), mask4);
  M[6] = XOR(M[6], W[2]);
  W[2] = SL(W[2],4);
  M[2] = XOR(M[2], W[2]);

  W[3] = AND(XOR(SR(M[3],4),M[7]), mask4);
  M[7] = XOR(M[7], W[3]);
  W[3] = SL(W[3],4);
  M[3] = XOR(M[3], W[3]);

  W[0] = AND(XOR(SR(M[0],2),M[2]), mask2);
  M[2] = XOR(M[2], W[0]);
  W[0] = SL(W[0],2);
  M[0] = XOR(M[0], W[0]);

  W[1] = AND( XOR(SR(M[1],2),M[3]), mask2);
  M[3] = XOR(M[3], W[1]);
  W[1] = SL(W[1],2);
  M[1] = XOR(M[1], W[1]);

  W[2] = AND( XOR(SR(M[4],2),M[6]), mask2);
  M[6] = XOR(M[6], W[2]);
  W[2] = SL(W[2],2);
  M[4] = XOR(M[4], W[2]);

  W[3] = AND( XOR(SR(M[5],2),M[7]), mask2);
  M[7] = XOR(M[7], W[3]);
  W[3] = SL(W[3],2);
  M[5] = XOR(M[5], W[3]);

  W[0] = AND(XOR(SR(M[0],1),M[1]), mask1);
  M[1] = XOR(M[1], W[0]);
  W[0] = SL(W[0],1);
  M[0] = XOR(M[0], W[0]);

  W[1] = AND( XOR(SR(M[2],1),M[3]), mask1);
  M[3] = XOR(M[3], W[1]);
  W[1] = SL(W[1],1);
  M[2] = XOR(M[2], W[1]);

  W[2] = AND( XOR(SR(M[4],1),M[5]), mask1);
  M[5] = XOR(M[5], W[2]);
  W[2] = SL(W[2],1);
  M[4] = XOR(M[4], W[2]);

  W[3] = AND( XOR(SR(M[6],1),M[7]), mask1);
  M[7] = XOR(M[7], W[3]);
  W[3] = SL(W[3],1);
  M[6] = XOR(M[6], W[3]);

  return 0;
}

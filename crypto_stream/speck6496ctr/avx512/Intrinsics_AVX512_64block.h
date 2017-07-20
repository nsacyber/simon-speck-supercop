//Some definitions using AVX2 intriniscs.  This file
//is used for all of our AVX2 implementations of Simon 
//and Speck with 64-bit block sizes.

#include <immintrin.h>

#define u32 unsigned
#define u64 unsigned long long
#define u512 __m512i

#define LCS(x,r) (((x)<<r)|((x)>>(32-r)))
#define RCS(x,r) (((x)>>r)|((x)<<(32-r)))

#define SET _mm512_set_epi32
#define XOR _mm512_xor_si512  
#define AND _mm512_and_si512
#define ADD _mm512_add_epi32
#define ROL(X,r) (_mm512_rol_epi32(X,r))
#define ROR(X,r) (_mm512_ror_epi32(X,r))   


#define _q16 SET(0xf,0xe,0x7,0x6,0xd,0xc,0x5,0x4,0xb,0xa,0x3,0x2,0x9,0x8,0x1,0x0)
#define _sixteen SET(0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10)

#define SET1(X,c) (X=SET(c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c))
#define SET16(X,c)(X=SET(c,c,c,c,c,c,c,c,c,c,c,c,c,c,c,c), X=ADD(X,_q16))

#define LOW _mm512_unpacklo_epi32
#define HIGH _mm512_unpackhi_epi32
#define LD(ip) _mm512_loadu_si512((__m512i *)(ip))
#define ST(ip,X) _mm512_storeu_si512((__m512i *)(ip),X)
#define STORE(out,X,Y) (ST(out,LOW(Y,X)),ST(out+64,HIGH(Y,X)))
#define XOR_STORE(in,out,X,Y) (ST(out,XOR(LD(in),LOW(Y,X))), ST(out+64,XOR(LD(in+64),HIGH(Y,X))))

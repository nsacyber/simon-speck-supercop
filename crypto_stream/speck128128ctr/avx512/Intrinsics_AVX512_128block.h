//Some definitions using AVX512 intriniscs.  This file
//is used for all of our AVX512 implementations of Simon 
//and Speck with 128-bit block sizes.

#include <immintrin.h>


#define u32 unsigned
#define u64 unsigned long long
#define u512 __m512i


#define LCS(x,r) (((x)<<r)|((x)>>(64-r)))
#define RCS(x,r) (((x)>>r)|((x)<<(64-r)))

#define SET _mm512_set_epi64
#define XOR _mm512_xor_si512    
#define ADD _mm512_add_epi64
#define AND _mm512_and_si512
#define ROL(X,r) (_mm512_rol_epi64(X,r))
#define ROR(X,r) (_mm512_ror_epi64(X,r))   
#define _q8 SET(0x7LL,0x3LL,0x6LL,0x2LL,0x5LL,0x1LL,0x4LL,0x0LL)

#define _eight SET(0x8LL,0x8LL,0x8LL,0x8LL,0x8LL,0x8LL,0x8LL,0x8LL)
#define SET1(X,c) (X=SET(c,c,c,c,c,c,c,c))
#define SET8(X,c) (X=SET(c,c,c,c,c,c,c,c), X=ADD(X,_q8))


#define LOW  _mm512_unpacklo_epi64
#define HIGH _mm512_unpackhi_epi64
#define LD(ip) (_mm512_load_epi64(((void *)(ip))))
#define ST(ip,X) _mm512_storeu_si512((void *)(ip),X)
#define STORE(out,X,Y) (ST(out,LOW(Y,X)), ST(out+64,HIGH(Y,X)))
#define XOR_STORE(in,out,X,Y) (ST(out,XOR(LD(in),LOW(Y,X))), ST(out+64,XOR(LD(in+64),HIGH(Y,X))))

#include <stddef.h> /* size_t */
#include <string.h> /* memcpy, memset */
#include <mmintrin.h>
#include "crypto_uint8.h"
typedef crypto_uint8 uint8;
#include "crypto_uint32.h"
typedef crypto_uint32 uint32;
#include "crypto_uint64.h"
typedef crypto_uint64 uint64;
#include "crypto_hash.h"

#define MK_64(hi32,lo32)  ((uint64) (lo32) + (((uint64) (hi32)) << 32))

#define SKEIN_VERSION           (1)
#define SKEIN_ID_STRING_LE      (0x33414853)
#define SKEIN_SCHEMA_VER        MK_64(SKEIN_VERSION,SKEIN_ID_STRING_LE)
#define SKEIN_KS_PARITY         _mm_set_pi32(0x1BD11BDA,0xA9FC1A22)

static const uint64 IV[] = {
    MK_64(0x4903ADFF,0x749C51CE),
    MK_64(0x0D95DE39,0x9746DF03),
    MK_64(0x8FD19341,0x27C79BCE),
    MK_64(0x9A255629,0xFF352CB1),
    MK_64(0x5DB62599,0xDF6CA7B0),
    MK_64(0xEABE394C,0xA9D5C3F4),
    MK_64(0x991112C7,0x1A75B523),
    MK_64(0xAE18A40B,0x660FCC33)
};

enum {
    R_512_0_0=46, R_512_0_1=36, R_512_0_2=19, R_512_0_3=37,
    R_512_1_0=33, R_512_1_1=27, R_512_1_2=14, R_512_1_3=42,
    R_512_2_0=17, R_512_2_1=49, R_512_2_2=36, R_512_2_3=39,
    R_512_3_0=44, R_512_3_1= 9, R_512_3_2=54, R_512_3_3=56,
    R_512_4_0=39, R_512_4_1=30, R_512_4_2=34, R_512_4_3=24,
    R_512_5_0=13, R_512_5_1=50, R_512_5_2=10, R_512_5_3=17,
    R_512_6_0=25, R_512_6_1=29, R_512_6_2=39, R_512_6_3=43,
    R_512_7_0= 8, R_512_7_1=35, R_512_7_2=56, R_512_7_3=22,
};

typedef struct {
    __m64  T[2];         /* tweak words: T[0]=byte cnt, T[1]=flags */
    __m64  X[8];         /* chaining variables */
    uint8  b[64];        /* partial block buffer (8-byte aligned) */
} Skein_512_Ctxt_t;

#define KW_TWK_BASE     (0)
#define KW_KEY_BASE     (3)
#define ks              (kw + KW_KEY_BASE)                
#define ts              (kw + KW_TWK_BASE)

static void Skein_512_Process_Block(Skein_512_Ctxt_t *ctx,const uint8 *blkPtr,size_t blkCnt,size_t byteCntAdd)
{
    __m64  kw[12];                         /* key schedule words : chaining vars + tweak */
    __m64  X0,X1,X2,X3,X4,X5,X6,X7;        /* local copy of vars, for speed */
    __m64  w[8];                          /* local copy of input block */
    __m64  z1;
    __m64  z3;
    __m64  z5;
    __m64  z7;

    ts[0] = ctx->T[0];
    ts[1] = ctx->T[1];
    do {
        ts[0] = _mm_add_si64(ts[0],_mm_set_pi32(0,byteCntAdd));

	z1 = SKEIN_KS_PARITY;
        ks[0] = ctx->X[0];
	z1 = _mm_xor_si64(z1,ks[0]);
        ks[1] = ctx->X[1];
	z1 = _mm_xor_si64(z1,ks[1]);
        ks[2] = ctx->X[2];
	z1 = _mm_xor_si64(z1,ks[2]);
        ks[3] = ctx->X[3];
	z1 = _mm_xor_si64(z1,ks[3]);
        ks[4] = ctx->X[4];
	z1 = _mm_xor_si64(z1,ks[4]);
        ks[5] = ctx->X[5];
	z1 = _mm_xor_si64(z1,ks[5]);
        ks[6] = ctx->X[6];
	z1 = _mm_xor_si64(z1,ks[6]);
        ks[7] = ctx->X[7];
	z1 = _mm_xor_si64(z1,ks[7]);
	ks[8] = z1;

        ts[2] = _mm_xor_si64(ts[0],ts[1]);

        X0 = 0[(__m64 *) blkPtr];
        X1 = 1[(__m64 *) blkPtr];
        X2 = 2[(__m64 *) blkPtr];
        X3 = 3[(__m64 *) blkPtr];
        X4 = 4[(__m64 *) blkPtr];
        X5 = 5[(__m64 *) blkPtr];
        X6 = 6[(__m64 *) blkPtr];
        X7 = 7[(__m64 *) blkPtr];

        w[0] = X0;
        w[1] = X1;
        w[2] = X2;
        w[3] = X3;
        w[4] = X4;
        w[5] = X5;
        w[6] = X6;
        w[7] = X7;

        X0 = _mm_add_si64(X0,ks[0]);
        X1 = _mm_add_si64(X1,ks[1]);
        X2 = _mm_add_si64(X2,ks[2]);
        X3 = _mm_add_si64(X3,ks[3]);
        X4 = _mm_add_si64(X4,ks[4]);
        X5 = _mm_add_si64(X5,_mm_add_si64(ks[5],ts[0]));
        X6 = _mm_add_si64(X6,_mm_add_si64(ks[6],ts[1]));
        X7 = _mm_add_si64(X7,ks[7]);

        blkPtr += 64;

#define R512(p0,p1,p2,p3,p4,p5,p6,p7,ROT,rNum)                      \
    X##p0 = _mm_add_si64(X##p0,X##p1); \
      X##p2 = _mm_add_si64(X##p2,X##p3); \
        X##p4 = _mm_add_si64(X##p4,X##p5); \
          X##p6 = _mm_add_si64(X##p6,X##p7); \
    z1 = X##p1; \
    X##p1 = _m_psrlqi(X##p1,64-ROT##_0); \
    z1 = _m_psllqi(z1,ROT##_0); \
    X##p1 = _mm_or_si64(X##p1,z1); \
      z3 = X##p3; \
      X##p3 = _m_psrlqi(X##p3,64-ROT##_1); \
      z3 = _m_psllqi(z3,ROT##_1); \
      X##p3 = _mm_or_si64(X##p3,z3); \
        z5 = X##p5; \
        z5 = _m_psllqi(z5,ROT##_2); \
        X##p5 = _m_psrlqi(X##p5,64-ROT##_2); \
        X##p5 = _mm_or_si64(X##p5,z5); \
          z7 = X##p7; \
          X##p7 = _m_psrlqi(X##p7,64-ROT##_3); \
          z7 = _m_psllqi(z7,ROT##_3); \
          X##p7 = _mm_or_si64(X##p7,z7); \
    X##p1 = _mm_xor_si64(X##p1,X##p0); \
      X##p3 = _mm_xor_si64(X##p3,X##p2); \
        X##p5 = _mm_xor_si64(X##p5,X##p4); \
          X##p7 = _mm_xor_si64(X##p7,X##p6); \

#define I512(R)                                                     \
    X0 = _mm_add_si64(X0,ks[((R)+1) % 9]);   /* inject the key schedule value */  \
    X1 = _mm_add_si64(X1,ks[((R)+2) % 9]);                                        \
    X2 = _mm_add_si64(X2,ks[((R)+3) % 9]);                                        \
    X3 = _mm_add_si64(X3,ks[((R)+4) % 9]);                                        \
    X4 = _mm_add_si64(X4,ks[((R)+5) % 9]);                                        \
    X5 = _mm_add_si64(X5,_mm_add_si64(ks[((R)+6) % 9],ts[((R)+1) % 3]));          \
    X6 = _mm_add_si64(X6,_mm_add_si64(ks[((R)+7) % 9],ts[((R)+2) % 3]));          \
    X7 = _mm_add_si64(X7,_mm_add_si64(ks[((R)+8) % 9],_mm_set_pi32(0,(R)+1)));     \

#define R512_8_rounds(R) \
        R512(0,1,2,3,4,5,6,7,R_512_0,8*(R)+ 1);   \
        R512(2,1,4,7,6,5,0,3,R_512_1,8*(R)+ 2);   \
        R512(4,1,6,3,0,5,2,7,R_512_2,8*(R)+ 3);   \
        R512(6,1,0,7,2,5,4,3,R_512_3,8*(R)+ 4);   \
        I512(2*(R));                              \
        R512(0,1,2,3,4,5,6,7,R_512_4,8*(R)+ 5);   \
        R512(2,1,4,7,6,5,0,3,R_512_5,8*(R)+ 6);   \
        R512(4,1,6,3,0,5,2,7,R_512_6,8*(R)+ 7);   \
        R512(6,1,0,7,2,5,4,3,R_512_7,8*(R)+ 8);   \
        I512(2*(R)+1);

        R512_8_rounds( 0);
        R512_8_rounds( 1);
        R512_8_rounds( 2);
        R512_8_rounds( 3);
        R512_8_rounds( 4);
        R512_8_rounds( 5);
        R512_8_rounds( 6);
        R512_8_rounds( 7);
        R512_8_rounds( 8);

        ctx->X[0] = _mm_xor_si64(X0,w[0]);
        ctx->X[1] = _mm_xor_si64(X1,w[1]);
        ctx->X[2] = _mm_xor_si64(X2,w[2]);
        ctx->X[3] = _mm_xor_si64(X3,w[3]);
        ctx->X[4] = _mm_xor_si64(X4,w[4]);
        ctx->X[5] = _mm_xor_si64(X5,w[5]);
        ctx->X[6] = _mm_xor_si64(X6,w[6]);
        ctx->X[7] = _mm_xor_si64(X7,w[7]);

        ts[1] = _mm_and_si64(ts[1],_mm_set_pi32(~(((uint32)  64 ) << 24),~0));
    } while (--blkCnt);
    ctx->T[0] = ts[0];
    ctx->T[1] = ts[1];
}


int crypto_hash
    (
    unsigned char *out,
    const unsigned char *in,
    unsigned long long inlen
    )

{
    Skein_512_Ctxt_t ctx;

    memcpy(ctx.X,IV,sizeof(ctx.X));
    ctx.T[0] = _mm_set_pi32(0,0);
    ctx.T[1] = _mm_set_pi32(((uint32) 112) << 24,0);

    if (inlen > 64) {
        size_t n = (inlen-1) / 64;
        Skein_512_Process_Block(&ctx,in,n,64);
        inlen -= n * 64;
        in    += n * 64;
    }

    memset(ctx.b,0,sizeof(ctx.b));
    if (inlen) memcpy(ctx.b,in,inlen);
    ctx.T[1] = _mm_or_si64(ctx.T[1],_mm_set_pi32(((uint32) 128) << 24,0));
    Skein_512_Process_Block(&ctx,ctx.b,1,inlen);
    
    memset(ctx.b,0,sizeof(ctx.b));
    ctx.T[0] = _mm_set_pi32(0,0);
    ctx.T[1] = _mm_set_pi32(((uint32) 255) << 24,0);
    Skein_512_Process_Block(&ctx,ctx.b,1,sizeof(uint64));

    0[(__m64 *) out] = ctx.X[0];
    1[(__m64 *) out] = ctx.X[1];
    2[(__m64 *) out] = ctx.X[2];
    3[(__m64 *) out] = ctx.X[3];
    4[(__m64 *) out] = ctx.X[4];
    5[(__m64 *) out] = ctx.X[5];
    6[(__m64 *) out] = ctx.X[6];
    7[(__m64 *) out] = ctx.X[7];

    return 0;
}

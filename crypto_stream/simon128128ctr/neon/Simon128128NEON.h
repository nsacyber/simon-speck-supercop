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



#include "Intrinsics_NEON_128block.h"


#define numrounds   68
#define numkeywords 2

#define RD(U,V,k) (V=XOR(V,ROL(U,2)), V=XOR(V,AND(ROL(U,1),ROL8(U))), V=XOR(V,k))

#define R1x2(U,V,rk,s) (RD(U[0],V[0],rk[s][0]))
#define R1x4(U,V,rk,s) (RD(U[0],V[0],rk[s][0]), RD(U[1],V[1],rk[s][0]))
#define R1x6(U,V,rk,s) (RD(U[0],V[0],rk[s][0]), RD(U[1],V[1],rk[s][0]), RD(U[2],V[2],rk[s][0]))
#define R1x8(U,V,rk,s) (RD(U[0],V[0],rk[s][0]), RD(U[1],V[1],rk[s][0]), RD(U[2],V[2],rk[s][0]), RD(U[3],V[3],rk[s][0]))

#define R2x2(X,Y,rk,r,s)  (R1x2(X,Y,rk,r), R1x2(Y,X,rk,s))
#define R2x4(X,Y,rk,r,s)  (R1x4(X,Y,rk,r), R1x4(Y,X,rk,s))
#define R2x6(X,Y,rk,r,s)  (R1x6(X,Y,rk,r), R1x6(Y,X,rk,s))
#define R2x8(X,Y,rk,r,s)  (R1x8(X,Y,rk,r), R1x8(Y,X,rk,s))

#define R2x16(X,Y,rk,r,s) (Y[2]=XOR(XOR(rk[r][2],Y[2]),XOR(AND(X[1],ROL8(X[2])),X[0])),	\
                           Y[3]=XOR(XOR(rk[r][3],Y[3]),XOR(AND(X[2],ROL8(X[3])),X[1])),	\
                           Y[4]=XOR(XOR(rk[r][4],Y[4]),XOR(AND(X[3],ROL8(X[4])),X[2])),	\
                           Y[5]=XOR(XOR(rk[r][5],Y[5]),XOR(AND(X[4],ROL8(X[5])),X[3])),	\
                           Y[6]=XOR(XOR(rk[r][6],Y[6]),XOR(AND(X[5],ROL8(X[6])),X[4])),	\
                           Y[7]=XOR(XOR(rk[r][7],Y[7]),XOR(AND(X[6],ROL8(X[7])),X[5])),	\
                           Y[0]=XOR(XOR(rk[r][0],Y[0]),ROL8(XOR(AND(X[7],X[0]),X[6]))),	\
                           Y[1]=XOR(XOR(rk[r][1],Y[1]),XOR(AND(X[0],ROL8(X[1])),ROL8(X[7]))), \
                           X[2]=XOR(XOR(rk[s][2],X[2]),XOR(AND(Y[1],ROL8(Y[2])),Y[0])),	\
                           X[3]=XOR(XOR(rk[s][3],X[3]),XOR(AND(Y[2],ROL8(Y[3])),Y[1])),\
                           X[4]=XOR(XOR(rk[s][4],X[4]),XOR(AND(Y[3],ROL8(Y[4])),Y[2])),	\
                           X[5]=XOR(XOR(rk[s][5],X[5]),XOR(AND(Y[4],ROL8(Y[5])),Y[3])),	\
                           X[6]=XOR(XOR(rk[s][6],X[6]),XOR(AND(Y[5],ROL8(Y[6])),Y[4])),	\
                           X[7]=XOR(XOR(rk[s][7],X[7]),XOR(AND(Y[6],ROL8(Y[7])),Y[5])),	\
                           X[0]=XOR(XOR(rk[s][0],X[0]),ROL8(XOR(AND(Y[7],Y[0]),Y[6]))),	\
                           X[1]=XOR(XOR(rk[s][1],X[1]),XOR(AND(Y[0],ROL8(Y[1])),ROL8(Y[7]))))


#define f(x) ((LCS(x,1) & LCS(x,8)) ^ LCS(x,2))
#define R2(x,y,k1,k2) (y^=f(x), y^=k1, x^=f(y), x^=k2)

#define R2x1(x,y,k,r,s) (R2(x[0],y[0],k[r],k[s]))

#define Enc(X,Y,rk,n) (R2x##n(X,Y,rk,0,1),   R2x##n(X,Y,rk,2,3),   R2x##n(X,Y,rk,4,5),   R2x##n(X,Y,rk,6,7),   R2x##n(X,Y,rk,8,9), \
		       R2x##n(X,Y,rk,10,11), R2x##n(X,Y,rk,12,13), R2x##n(X,Y,rk,14,15), R2x##n(X,Y,rk,16,17), R2x##n(X,Y,rk,18,19), \
		       R2x##n(X,Y,rk,20,21), R2x##n(X,Y,rk,22,23), R2x##n(X,Y,rk,24,25), R2x##n(X,Y,rk,26,27), R2x##n(X,Y,rk,28,29), \
		       R2x##n(X,Y,rk,30,31), R2x##n(X,Y,rk,32,33), R2x##n(X,Y,rk,34,35), R2x##n(X,Y,rk,36,37), R2x##n(X,Y,rk,38,39), \
		       R2x##n(X,Y,rk,40,41), R2x##n(X,Y,rk,42,43), R2x##n(X,Y,rk,44,45), R2x##n(X,Y,rk,46,47), R2x##n(X,Y,rk,48,49), \
                       R2x##n(X,Y,rk,50,51), R2x##n(X,Y,rk,52,53), R2x##n(X,Y,rk,54,55), R2x##n(X,Y,rk,56,57), R2x##n(X,Y,rk,58,59), \
                       R2x##n(X,Y,rk,60,61), R2x##n(X,Y,rk,62,63), R2x##n(X,Y,rk,64,65), R2x##n(X,Y,rk,66,67))



#define _D SET(0xffffffffffffffffLL,0xffffffffffffffffLL)
#define _C SET(0xffffffffffffff00LL,0xffffffffffffff00LL)

#define RKBS(rk,r,_V) (rk[r][7]= _D ^ rk[r-2][7] ^ ROR8(rk[r-1][2] ^ rk[r-1][3]), \
                       rk[r][6]= _D ^ rk[r-2][6] ^ ROR8(rk[r-1][1] ^ rk[r-1][2]), \
                       rk[r][5]= _D ^ rk[r-2][5] ^ ROR8(rk[r-1][0] ^ rk[r-1][1]), \
                       rk[r][4]= _D ^ rk[r-2][4] ^ rk[r-1][7] ^ ROR8(rk[r-1][0]), \
                       rk[r][3]= _D ^ rk[r-2][3] ^ rk[r-1][6] ^ rk[r-1][7], \
                       rk[r][2]= _D ^ rk[r-2][2] ^ rk[r-1][5] ^ rk[r-1][6], \
                       rk[r][1]= _C ^ rk[r-2][1] ^ rk[r-1][4] ^ rk[r-1][5], \
                       rk[r][0]= _V ^ rk[r-2][0] ^ rk[r-1][3] ^ rk[r-1][4])

#define EKBS(rk) (RKBS(rk,2,_D),  RKBS(rk,3,_C),  RKBS(rk,4,_D),  RKBS(rk,5,_C),  RKBS(rk,6,_D),  RKBS(rk,7,_D),  RKBS(rk,8,_D), \
                  RKBS(rk,9,_D),  RKBS(rk,10,_C), RKBS(rk,11,_D), RKBS(rk,12,_D), RKBS(rk,13,_D), RKBS(rk,14,_C), RKBS(rk,15,_C), \
                  RKBS(rk,16,_C), RKBS(rk,17,_C), RKBS(rk,18,_C), RKBS(rk,19,_C), RKBS(rk,20,_D), RKBS(rk,21,_D), RKBS(rk,22,_C), \
                  RKBS(rk,23,_D), RKBS(rk,24,_C), RKBS(rk,25,_C), RKBS(rk,26,_D), RKBS(rk,27,_C), RKBS(rk,28,_C), RKBS(rk,29,_D), \
                  RKBS(rk,30,_D), RKBS(rk,31,_C), RKBS(rk,32,_C), RKBS(rk,33,_C), RKBS(rk,34,_D), RKBS(rk,35,_C), RKBS(rk,36,_D), \
                  RKBS(rk,37,_C), RKBS(rk,38,_C), RKBS(rk,39,_C), RKBS(rk,40,_C), RKBS(rk,41,_D), RKBS(rk,42,_C), RKBS(rk,43,_C), \
                  RKBS(rk,44,_C), RKBS(rk,45,_D), RKBS(rk,46,_D), RKBS(rk,47,_D), RKBS(rk,48,_D), RKBS(rk,49,_D), RKBS(rk,50,_D), \
                  RKBS(rk,51,_C), RKBS(rk,52,_C), RKBS(rk,53,_D), RKBS(rk,54,_C), RKBS(rk,55,_D), RKBS(rk,56,_D), RKBS(rk,57,_C), \
                  RKBS(rk,58,_D), RKBS(rk,59,_D), RKBS(rk,60,_C), RKBS(rk,61,_C), RKBS(rk,62,_D), RKBS(rk,63,_D), RKBS(rk,64,_D), \
                  RKBS(rk,65,_C), RKBS(rk,66,_D), RKBS(rk,67,_C))



#define _c 0xfffffffffffffffcLL
#define _d 0xfffffffffffffffdLL


#define RKNBS(c0,c1,A,B,rk,key,i) (A^=c0^(RCS(B,3)^RCS(B,4)), key[i]=A, SET1(rk[i][0],A),\
				   B^=c1^(RCS(A,3)^RCS(A,4)), key[i+1]=B, SET1(rk[i+1][0],B))

#define EKNBS(A,B,rk,key) (SET1(rk[0][0],A), key[0]=A, SET1(rk[1][0],B), key[1]=B,\
			   RKNBS(_d,_c,A,B,rk,key,2),  RKNBS(_d,_c,A,B,rk,key,4),  RKNBS(_d,_d,A,B,rk,key,6),  RKNBS(_d,_d,A,B,rk,key,8), \
			   RKNBS(_c,_d,A,B,rk,key,10), RKNBS(_d,_d,A,B,rk,key,12), RKNBS(_c,_c,A,B,rk,key,14), RKNBS(_c,_c,A,B,rk,key,16), \
			   RKNBS(_c,_c,A,B,rk,key,18), RKNBS(_d,_d,A,B,rk,key,20), RKNBS(_c,_d,A,B,rk,key,22), RKNBS(_c,_c,A,B,rk,key,24), \
			   RKNBS(_d,_c,A,B,rk,key,26), RKNBS(_c,_d,A,B,rk,key,28), RKNBS(_d,_c,A,B,rk,key,30), RKNBS(_c,_c,A,B,rk,key,32), \
			   RKNBS(_d,_c,A,B,rk,key,34), RKNBS(_d,_c,A,B,rk,key,36), RKNBS(_c,_c,A,B,rk,key,38), RKNBS(_c,_d,A,B,rk,key,40), \
			   RKNBS(_c,_c,A,B,rk,key,42), RKNBS(_c,_d,A,B,rk,key,44), RKNBS(_d,_d,A,B,rk,key,46), RKNBS(_d,_d,A,B,rk,key,48), \
			   RKNBS(_d,_c,A,B,rk,key,50), RKNBS(_c,_d,A,B,rk,key,52), RKNBS(_c,_d,A,B,rk,key,54), RKNBS(_d,_c,A,B,rk,key,56), \
			   RKNBS(_d,_d,A,B,rk,key,58), RKNBS(_c,_c,A,B,rk,key,60), RKNBS(_d,_d,A,B,rk,key,62), RKNBS(_d,_c,A,B,rk,key,64), \
			   RKNBS(_d,_c,A,B,rk,key,66))

#define mask4 SET(0x0f0f0f0f0f0f0f0fLL,0x0f0f0f0f0f0f0f0fLL)
#define mask2 SET(0x3333333333333333LL,0x3333333333333333LL)
#define mask1 SET(0x5555555555555555LL,0x5555555555555555LL)

#define Transpose(M) (W[0]=AND(XOR(SR(M[0],4),M[4]),mask4), M[4]=XOR(M[4],W[0]), W[0]=SL(W[0],4), M[0]=XOR(M[0],W[0]), \
                       W[1]=AND(XOR(SR(M[1],4),M[5]),mask4), M[5]=XOR(M[5],W[1]), W[1]=SL(W[1],4), M[1]=XOR(M[1],W[1]), \
                       W[2]=AND(XOR(SR(M[2],4),M[6]),mask4), M[6]=XOR(M[6],W[2]), W[2]=SL(W[2],4), M[2]=XOR(M[2],W[2]), \
                       W[3]=AND(XOR(SR(M[3],4),M[7]),mask4), M[7]=XOR(M[7],W[3]), W[3]=SL(W[3],4), M[3]=XOR(M[3],W[3]), \
                       W[0]=AND(XOR(SR(M[0],2),M[2]),mask2), M[2]=XOR(M[2],W[0]), M[0]=XOR(M[0],SL(W[0],2)), \
                       W[1]=AND(XOR(SR(M[1],2),M[3]),mask2), M[3]=XOR(M[3],W[1]), M[1]=XOR(M[1],SL(W[1],2)), \
                       W[2]=AND(XOR(SR(M[4],2),M[6]),mask2), M[6]=XOR(M[6],W[2]), M[4]=XOR(M[4],SL(W[2],2)), \
                       W[3]=AND(XOR(SR(M[5],2),M[7]),mask2), M[7]=XOR(M[7],W[3]), M[5]=XOR(M[5],SL(W[3],2)), \
                       W[0]=AND(XOR(SR(M[0],1),M[1]),mask1), M[1]=XOR(M[1],W[0]), M[0]=XOR(M[0],SL(W[0],1)), \
                       W[1]=AND(XOR(SR(M[2],1),M[3]),mask1), M[3]=XOR(M[3],W[1]), M[2]=XOR(M[2],SL(W[1],1)), \
                       W[2]=AND(XOR(SR(M[4],1),M[5]),mask1), M[5]=XOR(M[5],W[2]), M[4]=XOR(M[4],SL(W[2],1)), \
                       W[3]=AND(XOR(SR(M[6],1),M[7]),mask1), M[7]=XOR(M[7],W[3]), M[6]=XOR(M[6],SL(W[3],1)))

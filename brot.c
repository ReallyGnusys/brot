#include <stdio.h>
#include <malloc.h>
#include <immintrin.h>



#define TOPTH 100.0
#define BOTTH 0.005

const double toptharr[4]  __attribute__ ((aligned (32))) = { TOPTH,TOPTH,TOPTH,TOPTH};
const double bottharr[4]  __attribute__ ((aligned (32))) = { BOTTH,BOTTH,BOTTH,BOTTH};


#define LOADXMM0(pointer)  __asm__ __volatile__("movaps\t( %0 ) , %%xmm0" :: "r"(pointer))
#define LOADXMM(reg,pointer)  __asm__ __volatile__("movaps\t( %0 ) , %%xmm"#reg :: "r"(pointer))
#define STOREXMM(reg,pointer)  __asm__ __volatile__("movaps\t%%xmm"#reg" , ( %0 )" :: "r"(pointer))

#define MOVMSKPS(xmmreg,gpreg)  __asm__ __volatile__("movmskps\t%%xmm"#xmmreg" ,  %0 " : "=r"(gpreg))

#define LOADYMM(reg,pointer)  __asm__ __volatile__("vmovapd\t( %0 ) , %%ymm"#reg :: "r"(pointer))
//#define MOVYMM(src,dest)  __asm__ __volatile__("vmovapd\t%%ymm"#src ", %ymm"#dest)
//#define MOVYMM(reg,pointer)  __asm__ __volatile__("vmovapd\t%%ymm"#reg" , ( %0 )" :: "r"(pointer))
//#define MOVYMM(reg,reg2)  __asm__ __volatile__("vmovapd\t%%ymm"#reg",%%ymm"#reg2)

#define VMOVMSKPD(ymmreg,gpreg)  __asm__ __volatile__("vmovmskpd\t%%ymm"#ymmreg" ,  %0 " : "=r"(gpreg))
#define STOREYMM(reg,pointer)  __asm__ __volatile__("vmovapd\t%%ymm"#reg" , ( %0 )" :: "r"(pointer))

#define MLOOPS 1024


#define POSSHADE  0x00010100
#define MAXALPHA  0xFF000000
#define NEGSHADE  0x00000101

#if 0
unsigned int *rgbout; // [MAXDIM*MAXDIM]  __attribute__ ((aligned (16)));
unsigned int *brot_init(int dim){
  rgbout = (unsigned int *)memalign(16,dim*dim*4);
  return rgbout;
}

#endif

void printbuff(const char *msg,double buff[4]){
  int i;
  printf("%s",msg);
  for ( i = 0 ; i < 4 ; i++)
    printf(" %e",buff[i]);
  printf("\n");
}


void brot_debug(){
  double tmpbuff[4]  __attribute__ ((aligned (32)));

  __m256d tmpbuff2;
  //  double tmpbuff2[4]  __attribute__ ((aligned (32)));
  _mm256_stream_pd (tmpbuff, tmpbuff2);
}



 unsigned int  *brot_draw(int dim, double x1 , double y1, double inc ){
   __m256d ymm0,ymm1,ymm2,ymm3,ymm4,ymm5,ymm6,ymm7,ymm8,ymm9,ymm10,ymm11,ymm12;


  printf("\nhello\n");
#if 1
  unsigned int *rgbout;

  printf("\nsizeof(unsigned int) %ld",sizeof(unsigned int));

  // allocate buffer
  rgbout =(unsigned int *)memalign(32,dim*dim*4);
  double xincarr[4] __attribute__ ((aligned (32))) = {4*inc,4*inc,4*inc,4*inc} ;
  double yincarr[4] __attribute__ ((aligned (32))) = {inc,inc,inc,inc} ;

  int xc = 0;
  int yc = 0;


  int bufferindex = 0;


  double xstart[4]  __attribute__ ((aligned (32)));

  double tmpbuff[4]  __attribute__ ((aligned (32)));


  int loop;
  unsigned long int  mask = 0;

  unsigned long int  pmask = 0;
  unsigned long int  nmask = 0;

  unsigned int inloops  = 0;
  int bit;



  if (rgbout == NULL){
    return NULL;
  }

  // init oi vec

  xstart[0] = x1;
  xstart[1] = xstart[0] + inc;
  xstart[2] = xstart[1] + inc;
  xstart[3] = xstart[2] + inc;

  printf("\nle func entry dim %d  x1 %.6lf  y1 %.6lf   inc %e.\n",dim,x1,y1,inc);
  int tmp;
  for (tmp = 0 ; tmp < 4 ; tmp++){
    printf(" %8.2f ",xstart[tmp]);
  }
  printf("\n");

  tmpbuff[0] = y1;
  tmpbuff[1] = y1;
  tmpbuff[2] = y1;
  tmpbuff[3] = y1;

  printf("about to load tmpbuff to ymm1\n");
  ymm1 = _mm256_load_pd(tmpbuff);
  _mm256_store_pd (tmpbuff,ymm1);

  // LOADYMM(2,tmpbuff);
  // STOREYMM(2,tmpbuff);
  printbuff("ystart",tmpbuff);

  printf("loaded tmpbuff to ymm1\n");
  //STOREXMM(1,rgbout+bufferindex);
  for ( yc = 0 ; yc < dim ; yc += 1){

    ymm0 = _mm256_load_pd(xstart);
    _mm256_store_pd (tmpbuff,ymm0);
    //LOADYMM(0,xstart);

    //STOREYMM(0,tmpbuff);
    //  printbuff("xstart",tmpbuff);
    //STOREYMM(0,tmpbuff);
    _mm256_store_pd (tmpbuff,ymm0);

    //    printbuff("xstart",tmpbuff);

    for ( xc = 0 ; xc < dim ; xc+= 4)  {

      mask = 0;
      pmask = 0;
      nmask = 0;
      for (bit = 0 ; bit < 4 ; bit++){
        rgbout[bufferindex + bit] = 0;
      }

      //  STOREYMM(0,tmpbuff);
      //printbuff("x(b4 vmov)",tmpbuff);
      ymm2 = ymm0;
      // _mm256_store_pd (tmpbuff,ymm2);
      // printbuff("x ",tmpbuff);
   /*
      __asm__ __volatile__("\
vmovapd %ymm0 , %ymm4");
      STOREYMM(0,tmpbuff);
      */

      //     STOREYMM(4,tmpbuff);
      // _mm256_store_pd (tmpbuff,ymm4);

      //   printbuff("x(2) ",tmpbuff);


      ymm3 = ymm1;
      ymm5 = ymm0;
      ymm6 = ymm1;

      //_mm256_store_pd (tmpbuff,ymm5);
      //  printbuff("x ",tmpbuff);
/*
        __asm__ __volatile__("\
vmovapd %ymm0 , %ymm4\n\
vmovapd %ymm2 , %ymm6\n\
vmovapd %ymm0 , %ymm10\n\
vmovapd %ymm2 , %ymm12");
      */
      /*
      MOVYMM(0,4);
      MOVYMM(2,6);
      */

ymm5 =	_mm256_mul_pd (	ymm5,ymm5);
ymm6 =	_mm256_mul_pd (	ymm6,ymm6);

/*
      __asm__ __volatile__("\
vmulpd %ymm10 , %ymm10 , %ymm10\n\
vmulpd %ymm12 , %ymm12 , %ymm12");
*/

// _mm256_store_pd (tmpbuff,ymm5);

      //      STOREYMM(5,tmpbuff);
      //printbuff("x sq",tmpbuff);
// _mm256_store_pd (tmpbuff,ymm6);

      //      STOREYMM(6,tmpbuff);
      //printbuff("y sq",tmpbuff);
      //getchar();

      for ( loop = 0 ; (loop < MLOOPS) && (mask != 0x0f) ; loop++) {
        inloops ++;



ymm3 =	_mm256_mul_pd (	ymm2,ymm3);

ymm3 =	_mm256_add_pd (	ymm3,ymm3);

 ymm3 =	_mm256_add_pd (	ymm1,ymm3);

 ymm2 = ymm5;

 // ymm2=	_mm256_sub_pd (	ymm6,ymm2);
 ymm2=	_mm256_sub_pd (	ymm2,ymm6);

 ymm2 =	_mm256_add_pd (	ymm0,ymm2);
 ymm5 = ymm2;

 ymm5 =	_mm256_mul_pd (	ymm5,ymm5);
 ymm6 = ymm3;
 ymm6 =	_mm256_mul_pd (	ymm6,ymm6);


 /*
     __asm__ __volatile__("\
vmulpd %ymm2 , %ymm3, %ymm3 \n\
vaddpd %ymm3 , %ymm3, %ymm3 \n\
vaddpd %ymm1 , %ymm3, %ymm3 \n\
vmovapd %ymm5 , %ymm2\n\
vsubpd %ymm6 , %ymm2, %ymm2\n\
vaddpd %ymm0 , %ymm2, %ymm2\n\
vmovapd %ymm2 , %ymm5\n\
vmulpd %ymm5 , %ymm5, %ymm5\n\
vmovapd %ymm3 , %ymm6\n\
vmulpd %ymm6 , %ymm6, %ymm6" );
 */

      // check upper threshhold

      // build modulus squared in xmm4

 ymm4 = ymm5;
 ymm4 = _mm256_add_pd (	ymm4,ymm6);
 ymm4 = _mm256_sqrt_pd (ymm4);

 /*
      __asm__ __volatile__("\
vmovapd %ymm5 , %ymm4\n\
vaddpd %ymm6 , %ymm4, %ymm4\n\
vsqrtpd %ymm4, %ymm4");

 */

 ymm7 = _mm256_load_pd(toptharr);
 // _mm256_store_pd (tmpbuff,ymm0);

 //   LOADYMM(7,toptharr);

ymm7 = _mm256_cmp_pd(ymm7,ymm4, _CMP_LE_OS);
/*
      __asm__ __volatile__("\
vcmplepd %ymm4 , %ymm7 , %ymm7 \n");
*/
 pmask = _mm256_movemask_pd(ymm7);

 //      VMOVMSKPD(7,pmask);

      // check low threshhold

      // build modulus squared in xmm4
      /*
      __asm__ __volatile__("\
movaps %xmm5 , %xmm4\n\
addps %xmm6 , %xmm4\n\
sqrtps %xmm4, %xmm4");

      */

#if CHKBOTTH
 ymm7 = _mm256_load_pd(bottharr);
 ymm4 = _mm256_cmp_pd(ymm4,ymm7, _CMP_LT_OS);
 nmask = _mm256_movemask_pd(ymm4);
#endif
 // printf("loop %d nmask %X pmask %x \n",loop,nmask,pmask);
 // getchar();
  /*
    LOADYMM(7,bottharr);
      __asm__ __volatile__("\
vcmpltpd %ymm7 , %ymm4, %ymm4 \n");
     VMOVMSKPD(4,nmask);
  */




      for (bit = 0 ; bit < 4 ; bit++){
        if (!(mask & (1 << bit))){
          //printf("\nchecking bit %d ",bit);
          if (pmask & (1 << bit))
            rgbout[bufferindex + bit] = (loop+1)*1*POSSHADE  | MAXALPHA;
#if CHKBOTTH

          else if (nmask & (1 << bit)){

            //  printf("\ntrying to set buff neg bit %d  bind %d  loop %d",bit,bufferindex,loop);
            rgbout[bufferindex + bit] = (loop+1)*1*NEGSHADE;

          }
#endif

          }

      }
      //printf("\n loop %d  buffind %d  mask %x nmask %x  pmask %x",loop,bufferindex,mask,nmask,pmask);

      mask |= pmask;
      mask |= nmask;




      }// end for loop
      /*
      __asm__ __volatile__("\
movapd %xmm5 , %xmm4\n\
addpd %xmm6 , %xmm4\n");
      STOREXMM(4,rgbout+bufferindex);
      */

    // inc oi
 ymm7 = _mm256_load_pd(xincarr);
 ymm0 = _mm256_add_pd (	ymm7,ymm0);
 /*
 LOADYMM(7,xincarr);

      __asm__ __volatile__("vaddpd %ymm7 , %ymm0, %ymm0");
 */

      bufferindex += 4;



      //printf("\nXC = %d",xc);
    }

    // inc oj
    ymm7 = _mm256_load_pd(yincarr);
    ymm1 = _mm256_add_pd (	ymm7,ymm1);
    /*
    LOADYMM(7,yincarr);

    __asm__ __volatile__("vaddpd %ymm7 , %ymm1, %ymm1");
    */

  }

  printf("\nlefunc says adios : inloops was %u\n",inloops);


  //STOREXMM(1,tmpbuff);
  return rgbout; //yc * xc;

#endif
  // return (unsigned int *)0;

  //  __asm__ __volatile__("movapd  ( %0 ) , %%xmm0 ");

}

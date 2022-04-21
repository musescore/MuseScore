/* Copyright (c) 2008-2011 Xiph.Org Foundation
   Written by Jean-Marc Valin */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "mdct.h"
#include "stack_alloc.h"
#include "kiss_fft.h"
#include "mdct.h"
#include "modes.h"

#ifndef M_PI
#define M_PI 3.141592653
#endif

int ret = 0;
void check(kiss_fft_scalar  * in,kiss_fft_scalar  * out,int nfft,int isinverse)
{
    int bin,k;
    double errpow=0,sigpow=0;
    double snr;
    for (bin=0;bin<nfft/2;++bin) {
        double ansr = 0;
        double difr;

        for (k=0;k<nfft;++k) {
           double phase = 2*M_PI*(k+.5+.25*nfft)*(bin+.5)/nfft;
           double re = cos(phase);

           re /= nfft/4;

           ansr += in[k] * re;
        }
        /*printf ("%f %f\n", ansr, out[bin]);*/
        difr = ansr - out[bin];
        errpow += difr*difr;
        sigpow += ansr*ansr;
    }
    snr = 10*log10(sigpow/errpow);
    printf("nfft=%d inverse=%d,snr = %f\n",nfft,isinverse,snr );
    if (snr<60) {
       printf( "** poor snr: %f **\n", snr);
       ret = 1;
    }
}

void check_inv(kiss_fft_scalar  * in,kiss_fft_scalar  * out,int nfft,int isinverse)
{
   int bin,k;
   double errpow=0,sigpow=0;
   double snr;
   for (bin=0;bin<nfft;++bin) {
      double ansr = 0;
      double difr;

      for (k=0;k<nfft/2;++k) {
         double phase = 2*M_PI*(bin+.5+.25*nfft)*(k+.5)/nfft;
         double re = cos(phase);

         /*re *= 2;*/

         ansr += in[k] * re;
      }
      /*printf ("%f %f\n", ansr, out[bin]);*/
      difr = ansr - out[bin];
      errpow += difr*difr;
      sigpow += ansr*ansr;
   }
   snr = 10*log10(sigpow/errpow);
   printf("nfft=%d inverse=%d,snr = %f\n",nfft,isinverse,snr );
   if (snr<60) {
      printf( "** poor snr: %f **\n", snr);
      ret = 1;
   }
}


void test1d(int nfft,int isinverse,int arch)
{
    size_t buflen = sizeof(kiss_fft_scalar)*nfft;
    kiss_fft_scalar *in;
    kiss_fft_scalar *in_copy;
    kiss_fft_scalar *out;
    opus_val16 *window;
    int k;

#ifdef CUSTOM_MODES
    int shift = 0;
    const mdct_lookup *cfg;
    mdct_lookup _cfg;
    clt_mdct_init(&_cfg, nfft, 0, arch);
    cfg = &_cfg;
#else
    int shift;
    const mdct_lookup *cfg;
    CELTMode *mode = opus_custom_mode_create(48000, 960, NULL);
    if (nfft == 1920) shift = 0;
    else if (nfft == 960) shift = 1;
    else if (nfft == 480) shift = 2;
    else if (nfft == 240) shift = 3;
    else return;
    cfg = &mode->mdct;
#endif

    in = (kiss_fft_scalar*)malloc(buflen);
    in_copy = (kiss_fft_scalar*)malloc(buflen);
    out = (kiss_fft_scalar*)malloc(buflen);
    window = (opus_val16*)malloc(sizeof(opus_val16)*nfft/2);

    for (k=0;k<nfft;++k) {
        in[k] = (rand() % 32768) - 16384;
    }

    for (k=0;k<nfft/2;++k) {
       window[k] = Q15ONE;
    }
    for (k=0;k<nfft;++k) {
       in[k] *= 32768;
    }

    if (isinverse)
    {
       for (k=0;k<nfft;++k) {
          in[k] /= nfft;
       }
    }

    for (k=0;k<nfft;++k)
       in_copy[k] = in[k];
    /*for (k=0;k<nfft;++k) printf("%d %d ", in[k].r, in[k].i);printf("\n");*/

    if (isinverse)
    {
       for (k=0;k<nfft;++k)
          out[k] = 0;
       clt_mdct_backward(cfg,in,out, window, nfft/2, shift, 1, arch);
       /* apply TDAC because clt_mdct_backward() no longer does that */
       for (k=0;k<nfft/4;++k)
          out[nfft-k-1] = out[nfft/2+k];
       check_inv(in,out,nfft,isinverse);
    } else {
       clt_mdct_forward(cfg,in,out,window, nfft/2, shift, 1, arch);
       check(in_copy,out,nfft,isinverse);
    }
    /*for (k=0;k<nfft;++k) printf("%d %d ", out[k].r, out[k].i);printf("\n");*/


    free(in);
    free(in_copy);
    free(out);
    free(window);
#ifdef CUSTOM_MODES
    clt_mdct_clear(&_cfg, arch);
#endif
}

int main(int argc,char ** argv)
{
    ALLOC_STACK;
    int arch = opus_select_arch();

    if (argc>1) {
        int k;
        for (k=1;k<argc;++k) {
            test1d(atoi(argv[k]),0,arch);
            test1d(atoi(argv[k]),1,arch);
        }
    }else{
        test1d(32,0,arch);
        test1d(32,1,arch);
        test1d(256,0,arch);
        test1d(256,1,arch);
        test1d(512,0,arch);
        test1d(512,1,arch);
        test1d(1024,0,arch);
        test1d(1024,1,arch);
        test1d(2048,0,arch);
        test1d(2048,1,arch);
#ifndef RADIX_TWO_ONLY
        test1d(36,0,arch);
        test1d(36,1,arch);
        test1d(40,0,arch);
        test1d(40,1,arch);
        test1d(60,0,arch);
        test1d(60,1,arch);
        test1d(120,0,arch);
        test1d(120,1,arch);
        test1d(240,0,arch);
        test1d(240,1,arch);
        test1d(480,0,arch);
        test1d(480,1,arch);
        test1d(960,0,arch);
        test1d(960,1,arch);
        test1d(1920,0,arch);
        test1d(1920,1,arch);
#endif
    }
    return ret;
}

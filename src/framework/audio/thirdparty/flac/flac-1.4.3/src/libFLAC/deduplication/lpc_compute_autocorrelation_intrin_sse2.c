/* This code is imported several times in lpc_intrin_sse2.c with different
 * values for MAX_LAG. Comments are for MAX_LAG == 14 */
	int i;
	__m128d sum0, sum1, sum2, sum3;
	__m128d d0, d1, d2, d3;
#if MAX_LAG > 8
	__m128d d4;
	__m128d sum4;
#endif
#if MAX_LAG > 10
	__m128d d5, d6;
	__m128d sum5, sum6;
#endif

	(void) lag;
	FLAC__ASSERT(lag <= MAX_LAG);

	/* Initialize all sum vectors with zero */
	sum0 = _mm_setzero_pd();
	sum1 = _mm_setzero_pd();
	sum2 = _mm_setzero_pd();
	sum3 = _mm_setzero_pd();
	d0 = _mm_setzero_pd();
	d1 = _mm_setzero_pd();
	d2 = _mm_setzero_pd();
	d3 = _mm_setzero_pd();
#if MAX_LAG > 8
	sum4 = _mm_setzero_pd();
	d4 = _mm_setzero_pd();
#endif
#if MAX_LAG > 10
	sum5 = _mm_setzero_pd();
	sum6 = _mm_setzero_pd();
	d5 = _mm_setzero_pd();
	d6 = _mm_setzero_pd();
#endif

	/* Loop backwards through samples from data_len to limit */
	for(i = data_len-1; i >= 0; i--) {
		__m128d d = _mm_set1_pd(data[i]);

		/* The next lines of code work like a queue. For more
		 * information see the lag8 version of this function */
#if MAX_LAG > 10
		d6 = _mm_shuffle_pd(d5, d6, _MM_SHUFFLE(0,0,0,1));
		d5 = _mm_shuffle_pd(d4, d5, _MM_SHUFFLE(0,0,0,1));
#endif
#if MAX_LAG > 8
		d4 = _mm_shuffle_pd(d3, d4, _MM_SHUFFLE(0,0,0,1));
#endif
		d3 = _mm_shuffle_pd(d2, d3, _MM_SHUFFLE(0,0,0,1));
		d2 = _mm_shuffle_pd(d1, d2, _MM_SHUFFLE(0,0,0,1));
		d1 = _mm_shuffle_pd(d0, d1, _MM_SHUFFLE(0,0,0,1));
		d0 = _mm_shuffle_pd(d,  d0, _MM_SHUFFLE(0,0,0,1));

		/* sumn += d*dn */
		sum0 = _mm_add_pd(sum0, _mm_mul_pd(d, d0));
		sum1 = _mm_add_pd(sum1, _mm_mul_pd(d, d1));
		sum2 = _mm_add_pd(sum2, _mm_mul_pd(d, d2));
		sum3 = _mm_add_pd(sum3, _mm_mul_pd(d, d3));
#if MAX_LAG > 8
		sum4 = _mm_add_pd(sum4, _mm_mul_pd(d, d4));
#endif
#if MAX_LAG > 10
		sum5 = _mm_add_pd(sum5, _mm_mul_pd(d, d5));
		sum6 = _mm_add_pd(sum6, _mm_mul_pd(d, d6));
#endif
	}

	/* Store sum0..sum6 in autoc[0..14] */
	_mm_storeu_pd(autoc,   sum0);
	_mm_storeu_pd(autoc+2, sum1);
	_mm_storeu_pd(autoc+4, sum2);
	_mm_storeu_pd(autoc+6 ,sum3);
#if MAX_LAG > 8
	_mm_storeu_pd(autoc+8, sum4);
#endif
#if MAX_LAG > 10
	_mm_storeu_pd(autoc+10,sum5);
	_mm_storeu_pd(autoc+12,sum6);
#endif

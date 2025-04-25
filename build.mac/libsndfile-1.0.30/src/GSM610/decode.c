/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

#include <stdio.h>

#include	"gsm610_priv.h"

/*
 *  4.3 FIXED POINT IMPLEMENTATION OF THE RPE-LTP DECODER
 */

static void Postprocessing (
	struct gsm_state	* S,
	register int16_t		* s)
{
	register int		k ;
	register int16_t		msr = S->msr ;
	register int16_t		tmp ;

	for (k = 160 ; k-- ; s++)
	{	tmp = GSM_MULT_R (msr, 28180) ;
		msr = GSM_ADD (*s, tmp) ;			/* Deemphasis 	     */
		*s = GSM_ADD (msr, msr) & 0xFFF8 ;	/* Truncation & Upscaling */
		}
	S->msr = msr ;
}

void Gsm_Decoder (
	struct gsm_state	* S,

	int16_t		* LARcr,	/* [0..7]		IN	*/

	int16_t		* Ncr,		/* [0..3] 		IN 	*/
	int16_t		* bcr,		/* [0..3]		IN	*/
	int16_t		* Mcr,		/* [0..3] 		IN 	*/
	int16_t		* xmaxcr,	/* [0..3]		IN 	*/
	int16_t		* xMcr,		/* [0..13*4]		IN	*/

	int16_t		* s)		/* [0..159]		OUT 	*/
{
	int		j, k ;
	int16_t		erp [40], wt [160] ;
	int16_t		*drp = S->dp0 + 120 ;

	for (j = 0 ; j <= 3 ; j++, xmaxcr++, bcr++, Ncr++, Mcr++, xMcr += 13)
	{	Gsm_RPE_Decoding (/*-S,-*/ *xmaxcr, *Mcr, xMcr, erp) ;
		Gsm_Long_Term_Synthesis_Filtering (S, *Ncr, *bcr, erp, drp) ;

		for (k = 0 ; k <= 39 ; k++) wt [j * 40 + k] = drp [k] ;
		}

	Gsm_Short_Term_Synthesis_Filter (S, LARcr, wt, s) ;
	Postprocessing (S, s) ;
}


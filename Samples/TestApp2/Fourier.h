// Fourier.h: interface for the Fourier class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOURIER_H__6939C9A4_8D93_41A3_9521_3B863957EB8E__INCLUDED_)
#define AFX_FOURIER_H__6939C9A4_8D93_41A3_9521_3B863957EB8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
 * fft.h
 *
 * loic fonteneau 15-feb-2001
 * Perform discrete FFT
 *
 * Original code : Don Cross <dcross@intersrv.com>
 * http://www.intersrv.com/~dcross/fft.html
 *
 */


///////////////////////////
//  function prototypes  //
///////////////////////////

void fft_double(unsigned int p_nSamples, bool p_bInverseTransform, double *p_lpRealIn, double *p_lpImagIn, double *p_lpRealOut, double *p_lpImagOut);

bool IsPowerOfTwo(unsigned int p_nX);

unsigned int NumberOfBitsNeeded(unsigned int p_nSamples);

unsigned int ReverseBits(unsigned int p_nIndex, unsigned int p_nBits);

double Index_to_frequency(unsigned int p_nBaseFreq, unsigned int p_nSamples, unsigned int p_nIndex);

#define FrequencyIntensity(re, im)		(sqrt((re*re)+(im*im)))
#define mag_sqrd(re,im)					(re*re+im*im)
#define Decibels(re,im)					((re == 0 && im == 0) ? (0) : 10.0 * log10(double(mag_sqrd(re,im))))
//#define Amplitude(re,im,len) (GetFrequencyIntensity(re,im)/(len))
#define Amplitude(re,im,len)			(FrequencyIntensity(re,im)/(len))
#define AmplitudeScaled(re,im,len,scale)	((int)Amplitude(re,im,len)%scale)


/////////////////////////////
//  constantes-definition  //
/////////////////////////////

#define  PI  (3.14159265358979323846)


#endif // !defined(AFX_FOURIER_H__6939C9A4_8D93_41A3_9521_3B863957EB8E__INCLUDED_)

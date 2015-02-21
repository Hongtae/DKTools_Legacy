#include "StdAfx.h"
#include "TestAudioFilter.h"
#include "Fourier.h"
#include <math.h>

TestAudioFilter::TestAudioFilter(void)
	: maxLevelValue(0)
{
	for (int i = 0; i < 10; i++)
	{
		leftLevels[i] = 0;
		rightLevels[i] = 0;
	}
}

TestAudioFilter::~TestAudioFilter(void)
{
}

bool IsPowerOfTwo( unsigned int p_nX );

void TestAudioFilter::OnProcessStream(void* pData, size_t size, double time)
{
	if (this->audioSource->Bits() == 16)
	{
		if (!::IsPowerOfTwo(size))		// size 가 2의 승수가 아니기 때문에 잘라버린다!
		{
			int n;
			for (n = 1; n <= size; n *= 2);
			n /= 2;
			size = n;
			if (size < 2)
				return;
		}

		short *pPCM = (short*)pData;
		size_t nLen = size / sizeof(short);

		int nChannel = this->audioSource->Channels();
		if (nChannel == 1)	// 모노
		{
			double *pDataFFT = new double[nLen];

			for (int i = 0; i < nLen; i++)
				pDataFFT[i] = (double)(pPCM[i]);

			float levels[10];
			this->fft(pDataFFT, nLen, levels, 10, 16, this->audioSource->Frequency());

			for (int i = 0; i < 10; i++)
			{
				this->leftLevels[i] = levels[i];
			}

			delete[] pDataFFT;
		}
		else
		{
			double *pDataL = new double[nLen/nChannel];
			double *pDataR = new double[nLen/nChannel];

			for (int i = 0; i < nLen/nChannel; i++)
			{
				pDataL[i] = (double)(pPCM[i * nChannel]);		// 홀수 channel (left)
				pDataR[i] = (double)(pPCM[i * nChannel + 1]);	// 짝수 channel (right)
			}

			float fFFTL[10];
			float fFFTR[10];

			this->fft(pDataL, nLen/nChannel, fFFTL, 10, 16, this->audioSource->Frequency());	// 16hz 부터 10개 구한다. (16~31),(32~63),(64~127)...
			this->fft(pDataR, nLen/nChannel, fFFTR, 10, 16, this->audioSource->Frequency());

			for (int i = 0; i < 10; i++)
			{
				this->leftLevels[i] = fFFTL[i];
				this->rightLevels[i] = fFFTR[i];
			}

			delete[] pDataL;
			delete[] pDataR;
		}
			
	}
}

void TestAudioFilter::fft(double* pcm, size_t len, float* level, int nLevelSize, int nStartHz, long frequency)
{
	double *pDataReal = new double[len];
	double *pDataImg = new double[len];

	fft_double(len, true, pcm, 0, pDataReal, pDataImg);

	for (int i = 0; i < nLevelSize; i++)
		level[i] = 0;

	double dStep = ((double)frequency / (double)len);
	long Max = nStartHz * 2;
	int index = 0;
	for (long i = (nStartHz * len) / frequency; i < len; i++)
	{
		double dCurrent = dStep * i;
		if (dCurrent > Max)
		{
			Max *= 2;
			index++;

			if (index >= nLevelSize)
				break;
		}

		double dVal = Decibels(pDataReal[i], pDataImg[i]);
	//	double dVal = AmplitudeScaled(pDataReal[i], pDataImg[i], len, 256);
	//	double dVal = ((int)mag_sqrd(pDataReal[i],  pDataImg[i])) % 256;
	//	double dVal = Amplitude(pDataReal[i], pDataImg[i], 256);
	//	double dVal = FrequencyIntensity(pDataReal[i], pDataImg[i]) / len;
		
		if (level[ index ] < dVal)
			level[ index ] = dVal;
	}

	maxLevelValue = 110;
	
	delete[] pDataReal;
	delete[] pDataImg;
}

#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class TestAudioFilter
{
public:
	TestAudioFilter(void);
	~TestAudioFilter(void);

	void OnProcessStream(void* pData, size_t size, double time);

	float leftLevels[10];		// 16 부터 *2 하여 16384 까지 10단계
	float rightLevels[10];
	void fft(double* pcm, size_t len, float* level, int nLevelSize, int nStartHz, long frequency);
	int maxLevelValue;

	DKObject<DKAudioPlayer>	audioSource;
};

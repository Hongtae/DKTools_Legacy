#pragma once
#include <DK.h>

////////////////////////////////////////////////////////////////////////////////
//
// AudioPlayer
//
// mp3 플레이어, iOS 에서는 한개만 재생하면 하드웨어 가속을 받음.
// 그 외 플랫폼에서는 DKAVMediaPlayer 와 차이가 없음.
// iOS 에서는 iPod 에서 재생할 수 있는 포맷만 지원함.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKUtils
{
	class AudioPlayer
	{
	public:
		AudioPlayer(void);
		~AudioPlayer(void);

		bool Load(const DKFoundation::DKString& file);	
		bool IsValid(void) const;

		bool Play(void);
		void Pause(void);
		void Stop(void);

		double Duration(void) const;
		double Progress(void) const;
		void SetProgress(double t);

		void SetVolume(float f);
		float Volume(void) const;
		
		DKFramework::DKAudioPlayer::AudioState State(void) const;

	private:
		void* context;
	};
}

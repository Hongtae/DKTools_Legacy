#include "AudioPlayer.h"
#include "../DKAVPlayer/DKAVPlayer.h"

#ifdef _WIN32
using namespace DKFoundation;
using namespace DKFramework;

namespace DKUtils
{
	struct AudioPlayerImpl
	{
		DKObject<DKAVMediaPlayer> source;
		bool paused;
	};
}

using namespace DKUtils;

AudioPlayer::AudioPlayer(void)
	: context(NULL)
{
}

AudioPlayer::~AudioPlayer(void)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		player->source->Stop();
		delete player;
	}	
}

bool AudioPlayer::Load(const DKString& file)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		player->source->Stop();
		delete player;
		context = NULL;
	}

	DKObject<DKAVMediaPlayer> source = DKAVMediaPlayer::Create(file, true, false);
	if (source)
	{
		source->SetPosition(0.0);
		AudioPlayerImpl* player = new AudioPlayerImpl;
		player->source = source;
		player->paused = false;
		context = reinterpret_cast<void*>(player);
		return true;
	}

	return false;
}

bool AudioPlayer::IsValid() const
{
	return context != NULL;
}

bool AudioPlayer::Play(void)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);

		DKAVMediaPlayer::MediaState state = player->source->State();
		if (state.controlState == DKAVMediaPlayer::MediaState::ControlStateStopped)
		{
			player->source->Play(1);
		}
		else
		{
			player->source->SetSpeed(1);
		}
		player->paused = false;
		return true;
	}
	return false;
}

void AudioPlayer::Pause(void)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		player->source->SetSpeed(0);
		player->paused = true;
	}
}

void AudioPlayer::Stop(void)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		player->source->Stop();
		player->source->SetPosition(0);
		player->paused = false;
	}
}

double AudioPlayer::Duration(void) const
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		return player->source->Duration();
	}
	return 0.0;
}

double AudioPlayer::Progress(void) const
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		return player->source->State().progress;
	}
	return 0.0;
}

void AudioPlayer::SetProgress(double t)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		player->source->SetPosition(t);
	}	
}

void AudioPlayer::SetVolume(float f)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		player->source->SetVolume(f);
	}
}

float AudioPlayer::Volume(void) const
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		return player->source->Volume();
	}
	return 0.0;
}

DKAudioPlayer::AudioState AudioPlayer::State(void) const
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		DKAVMediaPlayer::MediaState state = player->source->State();
		if (state.controlState == DKAVMediaPlayer::MediaState::ControlStateStopped)
			return DKAudioPlayer::AudioState::StateStopped;
		else if (player->paused)
			return DKAudioPlayer::AudioState::StatePaused;
		return DKAudioPlayer::AudioState::StatePlaying;
	}
	return DKAudioPlayer::AudioState::StateUnknown;
}

#endif

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#import <AVFoundation/AVFoundation.h>

@interface AudioPlayerImpl : AVAudioPlayer<AVAudioPlayerDelegate>
{
	BOOL			finished;
	BOOL			paused;
}

@property (readonly, nonatomic, assign) BOOL finished;
@property (readonly, nonatomic, assign) BOOL paused;
@end

@implementation AudioPlayerImpl
@synthesize finished;
@synthesize paused;

- (id)initWithContentsOfURL:(NSURL *)url error:(NSError **)outError
{
	self = [super initWithContentsOfURL:url error:outError];
	if (self)
	{
		finished = YES;
		paused = NO;
		self.currentTime = 0.0;
		self.delegate = self;
		[self prepareToPlay];
	}
	return self;
}

- (BOOL)play
{
	if ([super play])
	{
		finished = NO;
		paused = NO;
		return YES;
	}
	return NO;
}

- (BOOL)playAtTime:(NSTimeInterval)time
{
	if ([super playAtTime:time])
	{
		finished = NO;
		paused = NO;
		return YES;
	}
	return NO;
}

- (void)pause
{
	//[super pause];	// pause 를 호출하면 iOS4 에서 백그라운드 갔다왔을때 계속 재생되는 버그가 있음.
	[super stop];
	paused = YES;
}

- (void)stop
{
	[super stop];
	finished = YES;
	paused = NO;
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
	finished = YES;
	paused = NO;
}

@end

#include "AudioPlayer.h"

using namespace DKFoundation;
using namespace DKFramework;
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
		[player stop];
		[player autorelease];
	}
}

bool AudioPlayer::Load(const DKString& file)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		[player stop];
		[player autorelease];
		context = NULL;
	}
	if (file.Length() == 0)
		return false;
	
	NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:(const char*)DKStringU8(file)]];

	AudioPlayerImpl* player = [[AudioPlayerImpl alloc] initWithContentsOfURL:url error:nil];
	if (player)
	{
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
		return [player play] == YES;
	}
	return false;
}

void AudioPlayer::Pause(void)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		[player pause];
	}	
}

void AudioPlayer::Stop(void)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		[player stop];
	}	
}

double AudioPlayer::Duration(void) const
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		return player.duration;
	}
	return 0.0;
}

double AudioPlayer::Progress(void) const
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		return player.currentTime;
	}
	return 0.0;
}

void AudioPlayer::SetProgress(double t)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		player.currentTime = t;
	}	
}

void AudioPlayer::SetVolume(float f)
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		player.volume = f;
	}
}

float AudioPlayer::Volume(void) const
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		return player.volume;
	}
	return 0.0;
}

DKAudioPlayer::AudioState AudioPlayer::State(void) const
{
	if (context)
	{
		AudioPlayerImpl* player = reinterpret_cast<AudioPlayerImpl*>(context);
		if (player.finished)
			return DKAudioPlayer::AudioState::StateStopped;
		if (player.paused)
			return DKAudioPlayer::AudioState::StatePaused;
		
		return DKAudioPlayer::AudioState::StatePlaying;
	}
	return DKAudioPlayer::AudioState::StateUnknown;
}

#endif

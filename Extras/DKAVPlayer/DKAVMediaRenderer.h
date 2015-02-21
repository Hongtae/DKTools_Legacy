#pragma once

#include <DK.h>

struct AVCodecContext;
struct AVCodec;
struct AVPacket;

namespace DKFramework
{
	class DKAVMediaPlayer;
	class DKAVMediaStream;
	namespace Private {struct MediaStreamInfo;}

	class DKAVMediaRenderer : public DKFoundation::DKSharedInstance<DKAVMediaRenderer>
	{
	public:
		DKAVMediaRenderer(void);
		virtual ~DKAVMediaRenderer(void);

	public:
		struct StreamInfo
		{
			AVCodecContext*				context;
			AVCodec*					codec;
			double						timeBase;
			double						startTime;
			double						duration;		// 전체 시간
		};
		struct RenderCommand
		{
			enum Command
			{
				CommandRegister,		// 등록 및 상태 갱신 (등록 및 갱신후 MediaStreamInfo::StateStopped 상태가 된다)
				CommandUnregister,		// 목록에서 제거
				CommandRescale,			// 속도와 버퍼 크기 변경
				CommandNoop,
				CommandQuit,
			};
			Command						command;
			DKAVMediaPlayer*			context;		// 이걸로 구별함.
			double						minLength;		// 재생가능한 최소 버퍼링 시간
			double						speed;			// 렌더링 속도
			double						start;			// 시작 시간
			StreamInfo					audioStream;
			StreamInfo					videoStream;
			DKFramework::DKSize			videoSize;
			DKFoundation::DKObject<DKAVMediaStream>				stream;
			DKFoundation::DKObject<DKFramework::DKAudioSource>	audioOutput;			
		};

		typedef DKFoundation::DKMessageQueue<int, RenderCommand>	MessageQueue;
		MessageQueue mediaCommandQueue;	// 쓰레드와 메시지를 주고받기 위한 큐

	private:
		void RenderMediaStream(Private::MediaStreamInfo& media, double timeDelta);
		DKFoundation::DKObject<DKFoundation::DKThread> playbackThread;
		void PlaybackThreadProc(void);		// playback/decode 쓰레드
	};
}

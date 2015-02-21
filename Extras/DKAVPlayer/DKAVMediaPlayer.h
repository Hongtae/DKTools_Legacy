#pragma once
#include <DK.h>

////////////////////////////////////////////////////////////////////////////////
//
// DKAVMediaPlayer
//
// 미디어를 재생하는 클래스
// DKAVMediaPlayer::Create 함수를 호출하여 미디어 객체를 생성한다.
// buffer 파라메터에 미리 생성된 DKAVMediaBuffer 객체를 넣으면, 버퍼링 쓰레드를 공유하게 된다. (성능향상)
//
// Note:
// 메모리 객체나 작은 파일들은 버퍼링 쓰레드(DKAVMediaBuffer)를 공유하는것이 좋다.
// 인터넷으로 스트리밍 되는 객체들은 버퍼링 쓰레드를 공유하지 않는것이 좋다. (NULL 넣으면 된다)
// 버퍼링 쓰레드를 공유하게 되는 객체들중 하나가 딜레이가 발생하면 다른 모든 객체들도 딜레이가 생기게 된다.
//
// 재생속도를 빠르게 했을때 시스템이 재생속도를 못 따라가면 제한속도(speedLowLimit) 까지 느려진다.
// 재생 속도는 재생 도중엔 SetSpeed 로 설정하며, 재생할땐 Play 로 설정한다.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKAVMediaBuffer;
	class DKAVMediaRenderer;
	namespace Private {class IOStreamContext;}

	class DKAVMediaPlayer
	{
		friend class DKAVMediaBuffer;
		friend class DKAVMediaRenderer;

	public:
		typedef DKFoundation::DKMap<DKFoundation::DKString, DKFoundation::DKString> MetadataMap;
		struct MediaInfo
		{
			DKFoundation::DKString formatNameLong;
			DKFoundation::DKString formatNameShort;
			MetadataMap metadata;
		};
		struct MediaState
		{
			enum ControlState
			{
				ControlStateError = 0,
				ControlStatePlaying,
				ControlStateStopped,
			};
			enum RenderState
			{
				RenderStateStopped = 0,
				RenderStatePlaying,
				RenderStateWaiting,
			};
			enum BufferState
			{
				BufferStateStopped = 0,
				BufferStateFeeding,
			};
			ControlState	controlState;				// 미디어 상태
			RenderState		renderState;				// 재생 상태
			BufferState		bufferState;				// 버퍼링 상태
			double			speed;						// 재생 속도
			double			progress;					// 전체 진행 시간
			double			audio;						// 오디오 진행 시간
			double			video;						// 비디오 진행 시간
			double			bufferedAudio;				// 버퍼된 오디오 길이 (버퍼링)
			double			bufferedVideo;				// 버퍼된 비디오 길이 (버퍼링)
			double			audioDuration;				// 오디오 길이
			double			videoDuration;				// 비디오 길이
			double			mediaDuration;				// 전체 길이
			int				playCount;					// 재생 횟수
			double			minBufferLength;			// 재생 가능한 최소 버퍼 길이 (시간)
			double			maxBufferLength;			// 재생 가능한 최대 버퍼 길이 (시간)
		};

		DKAVMediaPlayer(DKAVMediaBuffer* buffer = NULL);
		virtual ~DKAVMediaPlayer(void);

		static DKFoundation::DKObject<DKAVMediaPlayer> Create(DKFoundation::DKStream* stream, bool enableAudio, bool enableVideo, DKAVMediaBuffer* buffer = NULL, double minBufferLength = 2, double maxBufferLength = 10);
		static DKFoundation::DKObject<DKAVMediaPlayer> Create(const DKFoundation::DKString& url, bool enableAudio, bool enableVideo, DKAVMediaBuffer* buffer = NULL, double minBufferLength = 2, double maxBufferLength = 10);

		bool SelectStream(int audio, int video);	// 스트림 선택
		bool Play(int count = 1);					// count 횟수만큼 재생함
		void Stop(void);							// 재생 중지함
		void Sync(void);							// 서브시스템과 싱크함 (싱크후 리턴함)
		MediaState State(void) const;				// 미디어 상태
		void State(MediaState& st) const;
		double Duration(void) const;				// 미디어 길이

		DKFramework::DKAudioSource* AudioSource(void);
		const DKFramework::DKAudioSource* AudioSource(void) const;
		DKFramework::DKTexture2D* VideoTexture(void);

		// VideoTexture: 현재 쓰레드에서 텍스쳐에 렌더링 하여 리턴한다. (티어링을 막기 위해 OpenGL 바인딩 된 쓰레드에서 렌더링 함)
		const DKFramework::DKTexture2D* VideoTexture(void) const;

		bool SetPosition(double pos);	// pos (시간단위)로 미디어 탐색 (0 < pos < duration)
		bool SetSpeed(double s);		// s 가 0 이면 pause 와 같음)

		void ResizeBuffer(double minSize, double maxSize);	// 버퍼 크기 재설정 (재생중 버퍼링 크기)

		void SetVolume(float f);
		float Volume(void) const;
	private:
		void Register(MediaState st);	// 렌더링 쓰레드와 버퍼에 등록하고 재생 및 버퍼링을 시작한다. (설정이 바뀌었어도 재등록함)
		void Unregister(void);			// 렌더링 쓰레드와 버퍼에 등록 해제. 재생 멈춤
		double StreamDuration(int index) const;

		void UpdateVideoData(DKFoundation::DKData* data);	// DKAVMediaRenderer 에서 디코딩 된 픽셀 데이터를 받음.
		void RenderVideoInternal(void);		// videoOutput 텍스쳐에 렌더링을 한다. VideoTexture 에서 호출함.

		void OnBufferStateChanged(MediaState::BufferState state);
		void OnRenderSatateChanged(MediaState::RenderState state);
		void OnRender(double time, double audio, double video, double speed);
		void OnRenderTimeRescaled(double speed, double minTime);
		void OnBufferLengthResized(double maxTime);
		void OnAudioFilter(double time, void* data, size_t size, int channels, int frequency, int bits);
		void OnBufferAudio(double time);
		void OnBufferVideo(double time);

		static DKFoundation::DKObject<DKAVMediaPlayer> CreateFromContext(void* ctxt, bool enableAudio, bool enableVideo, DKAVMediaBuffer* buffer, double minBufferLength, double maxBufferLength);
		bool IsSeekable(void) const;

		struct StreamInfo
		{
			enum Type
			{
				TypeAudio = 0,
				TypeVideo,
			};
			Type type;
			int index;
			void* codec;
			union CodecInfo
			{
				struct AudioInfo
				{
					int sampleRate;
					int channels;
					int bitRate;
				} audio;
				struct VideoInfo
				{
					int width;
					int height;
				} video;
			} codecInfo;			
			MetadataMap metadata;
			DKFoundation::DKString codecNameLong;
			DKFoundation::DKString codecNameShort;
		};
		StreamInfo* currentAudioStream;
		StreamInfo* currentVideoStream;

		void SyncRenderer(void);
		void SyncBufferController(void);

		float audioVolume;
		void* avContext;
		DKFoundation::DKArray<StreamInfo>					videoStreams;
		DKFoundation::DKArray<StreamInfo>					audioStreams;
		DKFoundation::DKObject<DKAVMediaRenderer>			renderer;			// media renderer
		DKFoundation::DKObject<DKAVMediaBuffer>				bufferController;

		DKFoundation::DKObject<DKFramework::DKAudioSource>	audioOutput;		// openal audio source
		DKFoundation::DKObject<DKFramework::DKTexture2D>	videoOutput;		// opengl texture2d

		MediaState	mediaState;
		MediaInfo	mediaInfo;

		// 텍스쳐는 티어링 현상을 막기 위해 실제 화면에 그려지는 쓰레드에서 그린다. (textureData 에 임시 보관함)
		DKFoundation::DKObject<DKFoundation::DKData>		textureData;
		DKFoundation::DKObject<Private::IOStreamContext>	ioStream;

		typedef DKFoundation::DKSpinLock MediaStateLock;
		typedef DKFoundation::DKCriticalSection<MediaStateLock> CriticalSection;
		MediaStateLock					lock;
	};
}

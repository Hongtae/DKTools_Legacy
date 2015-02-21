#include "../../DK/lib/OpenGL.h"
#include "../../DK/lib/OpenAL.h"
#include "FFmpeg/FFmpeg.h"

#include "DKAVMediaPlayer.h"
#include "DKAVMediaBuffer.h"
#include "DKAVMediaRenderer.h"
#include "DKAVMediaStream.h"

using namespace DKFoundation;
using namespace DKFramework;

// DKLIB_AV_LOG: 1 로 설정하면 디버그 로그를 출력함
#ifdef DKLIB_DEBUG_ENABLED
#define DKLIB_AV_LOG			1
#else
#define DKLIB_AV_LOG			0
#endif

// DKLIB_AV_LOCK: 1 로 설정하면 락 객체를 제공해준다
#define DKLIB_AV_LOCK			1

namespace DKFramework
{
	namespace Private
	{
		void AVLogCallback(void* p, int level, const char* fmt, va_list ap)
		{
#if DKLIB_AV_LOG
			DKLog("FFmpeg: %ls", (const wchar_t*)DKString::FormatV(fmt, ap));
#endif
		}

#if DKLIB_AV_LOCK
		typedef DKSpinLock AVLock;
		int AVLockCallback(void** mutex, enum AVLockOp op)
		{
			AVLock*& lock = reinterpret_cast<AVLock*&>(*mutex);
			switch (op)
			{
			case AV_LOCK_CREATE:  ///< Create a mutex
				lock = new AVLock;
				break;
			case AV_LOCK_OBTAIN:  ///< Lock the mutex
				lock->Lock();
				break;
			case AV_LOCK_RELEASE: ///< Unlock the mutex
				lock->Unlock();
				break;
			case AV_LOCK_DESTROY: ///< Free mutex resources
				delete lock;
				lock = NULL;
				break;
			}
			return 0;
		}
#endif

#define DKLIB_FFMPEG_VERSION_CHECK(fn, cn)	if (fn() != cn) {DKLog("WARNING: %s:%u %s:%u\n", #fn, fn(), #cn, cn);}
		struct InitAV
		{
			InitAV(void)
			{
				DKLIB_FFMPEG_VERSION_CHECK(avcodec_version, LIBAVCODEC_VERSION_INT);
				DKLIB_FFMPEG_VERSION_CHECK(avdevice_version, LIBAVDEVICE_VERSION_INT);
				DKLIB_FFMPEG_VERSION_CHECK(avfilter_version, LIBAVFILTER_VERSION_INT);
				DKLIB_FFMPEG_VERSION_CHECK(avformat_version, LIBAVFORMAT_VERSION_INT);
				DKLIB_FFMPEG_VERSION_CHECK(avutil_version, LIBAVUTIL_VERSION_INT);
				DKLIB_FFMPEG_VERSION_CHECK(swresample_version, LIBSWRESAMPLE_VERSION_INT);
				DKLIB_FFMPEG_VERSION_CHECK(swscale_version, LIBSWSCALE_VERSION_INT);

				av_register_all();
				av_log_set_callback(AVLogCallback);
#if DKLIB_AV_LOCK
				av_lockmgr_register(AVLockCallback);
#endif
				//DKLog("FFmpeg initialized.\n");
			}
		} initAV;

		class IOStreamContext
		{
		public:
			IOStreamContext(DKStream* s, size_t bufferSize)
				: stream(s)
				, avIOContext(NULL)
			{
				if (stream)
				{
					unsigned char* buffer = (unsigned char*)av_malloc(bufferSize);
					this->avIOContext = avio_alloc_context(buffer,
						bufferSize,
						0,
						this,
						&IOStreamContext::Read,
						NULL,
						&IOStreamContext::Seek);

					if (this->avIOContext == NULL && buffer)
						av_free(buffer);
				}
			}
			~IOStreamContext(void)
			{
				if (avIOContext)
				{
					av_free(avIOContext->buffer);
					av_free(avIOContext);
				}
			}
			static int Read(void* ctxt, uint8_t* buf, int size)
			{
				IOStreamContext* c = (IOStreamContext*)ctxt;
				if (c->stream->IsReadable())
					return c->stream->Read(buf, size);
				return -1;
			}
			static int64_t Seek(void* ctxt, int64_t offset, int whence)
			{
				IOStreamContext* c = (IOStreamContext*)ctxt;
				switch (whence)
				{
				case SEEK_SET:
					return c->stream->SetPos(offset);
				case SEEK_CUR:
					return c->stream->SetPos(c->stream->GetPos() + offset);
				case SEEK_END:
					return c->stream->SetPos(c->stream->TotalLength() + offset);
				case AVSEEK_SIZE:
					return c->stream->TotalLength();
				}
				return -1;
			}
			DKObject<DKStream>		stream;
			AVIOContext*			avIOContext;
		};
	}
}


using namespace DKFramework;
using namespace DKFramework::Private;

DKAVMediaPlayer::DKAVMediaPlayer(DKAVMediaBuffer* buffer)
	: avContext(NULL)
	, renderer(DKAVMediaRenderer::SharedInstance())
	, bufferController(buffer)
	, currentAudioStream(NULL)
	, currentVideoStream(NULL)
	, audioOutput(NULL)
	, videoOutput(NULL)
	, audioVolume(1.0)
{
	mediaState.controlState = MediaState::ControlStateStopped;
	mediaState.renderState = MediaState::RenderStateStopped;
	mediaState.bufferState = MediaState::BufferStateStopped;
	mediaState.speed = 1;
	mediaState.progress = 0;
	mediaState.audio = 0;
	mediaState.video = 0;
	mediaState.bufferedAudio = 0;
	mediaState.bufferedVideo = 0;
	mediaState.audioDuration = 0;
	mediaState.videoDuration = 0;
	mediaState.mediaDuration = 0;
	mediaState.playCount = 0;
	mediaState.minBufferLength = 1;
	mediaState.maxBufferLength = 10;

	if (bufferController == NULL)
		bufferController = DKObject<DKAVMediaBuffer>::New();
	
	audioOutput = DKObject<DKAudioSource>::New();
}

DKAVMediaPlayer::~DKAVMediaPlayer(void)
{
	mediaState.playCount = 0;	// 미리 0으로 설정 안하면 종료되면서 다시 재생 시작한다.

	DKAVMediaBuffer::BufferCommand bc = {DKAVMediaBuffer::BufferCommand::CommandUnregister, this};
	bufferController->bufferCommandQueue.ProcessMessage(bc);
	DKAVMediaRenderer::RenderCommand rc = {DKAVMediaRenderer::RenderCommand::CommandUnregister, this};
	renderer->mediaCommandQueue.ProcessMessage(rc);

	if (avContext)
	{
		AVFormatContext* formatCtxt = reinterpret_cast<AVFormatContext*>(avContext);
		avformat_close_input(&formatCtxt);
	}
	avContext = NULL;
	ioStream = NULL;

	audioOutput = NULL;
	videoOutput = NULL;

	renderer = NULL;
	textureData = NULL;

	DKLog("DKAVMediaPlayer:%x deleted. BufferController:%x, Renderer:%x\n", this, (DKAVMediaBuffer*)bufferController, (DKAVMediaRenderer*)renderer);
}

DKObject<DKAVMediaPlayer> DKAVMediaPlayer::CreateFromContext(void* ctxt, bool enableAudio, bool enableVideo, DKAVMediaBuffer* buffer, double minBufferLength, double maxBufferLength)
{
	if (ctxt == NULL)
		return NULL;

	AVFormatContext* formatContext = (AVFormatContext*)ctxt;

	formatContext->flags |= AVFMT_FLAG_GENPTS;

	if (avformat_find_stream_info(formatContext, NULL) >= 0)
	{
		//av_dump_format(formatContext, 0, (const char*)"DKAVMediaPlayer", 0);
		MediaInfo info;
		AVDictionaryEntry *metadataTag = NULL;
		while ((metadataTag = av_dict_get(formatContext->metadata, "", metadataTag, AV_DICT_IGNORE_SUFFIX)))
		{
			DKString key(metadataTag->key);
			DKString value(metadataTag->value);
			info.metadata.Update(key, value);
			DKLog("Metadata key:%ls value:%ls\n", (const wchar_t*)key, (const wchar_t*)value);
		}
		
		info.formatNameLong = DKString(formatContext->iformat->long_name);
		info.formatNameShort = DKString(formatContext->iformat->name);
		double duration = formatContext->duration != AV_NOPTS_VALUE ? formatContext->duration / AV_TIME_BASE : 0;

		DKLog("[DKAVMediaPlayer] Format: %ls(%ls) Duration: %f Seekable: %d\n",
			(const wchar_t*)info.formatNameShort, (const wchar_t*)info.formatNameLong, duration, formatContext->pb->seekable);

		// 스트림 찾기
		DKArray<StreamInfo> videoStreams;
		DKArray<StreamInfo> audioStreams;
		for (unsigned int i = 0; i < formatContext->nb_streams; i++)
		{
			AVStream* stream = formatContext->streams[i];
			AVCodecContext* codecContext = stream->codec;
			if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO && enableAudio)
			{
				AVCodec* codec = avcodec_find_decoder(codecContext->codec_id);
				if (codec && avcodec_open2(codecContext, codec, 0) >= 0)
				{
					StreamInfo si;
					si.type = StreamInfo::TypeAudio;
					si.index = i;
					si.codec = codec;
					si.codecNameLong = DKString(codec->long_name);
					si.codecNameShort = DKString(codec->name);
					si.codecInfo.audio.sampleRate = codecContext->sample_rate;
					si.codecInfo.audio.channels = codecContext->channels;
					si.codecInfo.audio.bitRate = codecContext->bit_rate;

					AVDictionaryEntry* metadataTag = NULL;
					while ((metadataTag = av_dict_get(stream->metadata, "", metadataTag, AV_DICT_IGNORE_SUFFIX)))
					{
						DKString key(metadataTag->key);
						DKString value(metadataTag->value);
						si.metadata.Update(key, value);
						DKLog("Audio-Stream at index:%d Metadata key:%ls value:%ls\n", i, (const wchar_t*)key, (const wchar_t*)value);
					}

					avcodec_close(codecContext);

					audioStreams.Add(si);

					DKLog("[DKAVMediaPlayer] Stream:%d Type:Audio Codec:%ls(%ls) Sample-Rate:%d Channels:%d Bit-Rate:%d\n",
						i, (const wchar_t*)si.codecNameShort, (const wchar_t*)si.codecNameLong,
						si.codecInfo.audio.sampleRate, si.codecInfo.audio.channels, si.codecInfo.audio.bitRate);
				}
			}
			if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO && enableVideo)
			{
				AVCodec* codec = avcodec_find_decoder(codecContext->codec_id);
				if (codec && avcodec_open2(codecContext, codec, 0) >= 0)
				{
					StreamInfo si;
					si.type = StreamInfo::TypeVideo;
					si.index = i;
					si.codec = codec;
					si.codecNameLong = DKString(codec->long_name);
					si.codecNameShort = DKString(codec->name);
					si.codecInfo.video.width = codecContext->width;
					si.codecInfo.video.height = codecContext->height;

					AVDictionaryEntry* metadataTag = NULL;
					while ((metadataTag = av_dict_get(stream->metadata, "", metadataTag, AV_DICT_IGNORE_SUFFIX)))
					{
						DKString key(metadataTag->key);
						DKString value(metadataTag->value);
						si.metadata.Update(key, value);
						DKLog("Video-Stream at index:%d Metadata key:%ls value:%ls\n", i, (const wchar_t*)key, (const wchar_t*)value);
					}

					avcodec_close(codecContext);

					videoStreams.Add(si);

					DKLog("[DKAVMediaPlayer] Stream:%d Type:Video Codec:%ls(%ls) Resolution:%d x %d\n",
						i, (const wchar_t*)si.codecNameShort, (const wchar_t*)si.codecNameLong,
						si.codecInfo.video.width, si.codecInfo.video.height);
				}
			}
		}

		if (videoStreams.Count() > 0 || audioStreams.Count() > 0)
		{
			maxBufferLength = Max<double>(minBufferLength, maxBufferLength);

			int bestAudio = -1;
			int bestVideo = -1;
			if (enableAudio && audioStreams.Count() > 0)
				bestAudio = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
			if (enableVideo && videoStreams.Count() > 0)
				bestVideo = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

			DKObject<DKAVMediaPlayer> obj = DKOBJECT_NEW DKAVMediaPlayer(buffer);
			obj->avContext = formatContext;
			obj->mediaInfo = info;
			obj->audioStreams = audioStreams;
			obj->videoStreams = videoStreams;
			obj->mediaState.minBufferLength = Max<double>(minBufferLength, 0);
			obj->mediaState.maxBufferLength = Max<double>(maxBufferLength, 1);

			struct
			{
				int operator () (int streamIndex, DKArray<StreamInfo>& array) const
				{
					for (size_t i = 0; i < array.Count(); ++i)
						if (array.Value(i).index == streamIndex)
							return i;
					return -1;
				}
			}getIndexFromArray;

			obj->SelectStream(getIndexFromArray(bestAudio, audioStreams), getIndexFromArray(bestVideo, videoStreams));
			return obj;
		}
	}
	return NULL;
}

DKObject<DKAVMediaPlayer> DKAVMediaPlayer::Create(DKFoundation::DKStream* stream, bool enableAudio, bool enableVideo, DKAVMediaBuffer* buffer,  double minBufferLength, double maxBufferLength)
{
	if (stream == NULL)
		return NULL;

	size_t bufferSize = 0x8000;		// 32768 bytes

	DKObject<IOStreamContext> ioContext = DKOBJECT_NEW IOStreamContext(stream, bufferSize);

	const char* filename = "DKAVMediaPlayer";

	AVFormatContext* ctxt = avformat_alloc_context();
	ctxt->pb = ioContext->avIOContext;
	if (avformat_open_input(&ctxt, filename, NULL, NULL) != 0)
	{
		DKLog("Failed to open file.\n");
		return NULL;
	}
	DKObject<DKAVMediaPlayer> source = CreateFromContext(ctxt, enableAudio, enableVideo, buffer, minBufferLength, maxBufferLength);
	if (source)
	{
		source->ioStream = ioContext;
		return source;
	}
	DKLog("[%s] Failed to create source.\n", DKLIB_FUNCTION_NAME);
	avformat_close_input(&ctxt);
	return NULL;
}

DKObject<DKAVMediaPlayer> DKAVMediaPlayer::Create(const DKFoundation::DKString& url, bool enableAudio, bool enableVideo, DKAVMediaBuffer* buffer,  double minBufferLength, double maxBufferLength)
{
	if (url.Length() == 0)
		return NULL;

	AVFormatContext* ctxt = NULL;

	DKStringU8 filePath(url);
	if (avformat_open_input(&ctxt, (const char*)filePath, NULL, NULL) != 0)
	{
		// 파일 스트림으로 다시 시도.
		return Create(DKFile::Create(url, DKFile::ModeOpenReadOnly, DKFile::ModeShareAll).SafeCast<DKStream>(), enableAudio, enableVideo, buffer, minBufferLength, maxBufferLength);
	}
	DKObject<DKAVMediaPlayer> source = CreateFromContext(ctxt, enableAudio, enableVideo, buffer, minBufferLength, maxBufferLength);
	if (source)
	{
		return source;
	}
	DKLog("[%s] Failed to create source.\n", DKLIB_FUNCTION_NAME);
	avformat_close_input(&ctxt);
	return NULL;
}

bool DKAVMediaPlayer::SelectStream(int audio, int video)
{
	CriticalSection guard(this->lock);

	if (mediaState.controlState != MediaState::ControlStateStopped)
		return false;
	if (mediaState.renderState != MediaState::RenderStateStopped)
		return false;
	if (mediaState.bufferState != MediaState::BufferStateStopped)
		return false;

	if (audio >= 0 && audio < audioStreams.Count())
	{
		StreamInfo* stream = &audioStreams.Value(audio);
		if (currentAudioStream != stream)
		{
			currentAudioStream = stream;
		}
	}
	else
	{
		currentAudioStream = NULL;
	}
	
	if (video >= 0 && video < videoStreams.Count())
	{
		StreamInfo* stream = &videoStreams.Value(video);
		if (currentVideoStream != stream)
		{
			currentVideoStream = stream;

			// OpenGL 텍스쳐 생성!
			AVCodecContext* codecContext = reinterpret_cast<AVFormatContext*>(avContext)->streams[stream->index]->codec;

			int maxTextureSize = 0;
			glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextureSize);

			DKASSERT_DEBUG(maxTextureSize > 0);

			int texWidth = Min<int>(codecContext->width, maxTextureSize);
			int texHeight = Min<int>(codecContext->height, maxTextureSize);

			void* pixels = DKMemoryHeapAlloc(texWidth * texHeight * 3);
			memset(pixels, 0, texWidth * texHeight * 3);
			videoOutput = DKTexture2D::Create(texWidth, texHeight , DKTexture2D::FormatRGB, DKTexture2D::TypeUnsignedByte, pixels);
			DKMemoryHeapFree(pixels);

			DKASSERT_DEBUG(videoOutput != NULL);

			DKLog("[%s] Texture2D (%d x %d) created.\n", DKLIB_FUNCTION_NAME, texWidth, texHeight);
		}
	}
	else
	{
		currentVideoStream = NULL;
		videoOutput = NULL;
	}
	textureData = NULL;

	mediaState.audioDuration = 0;
	mediaState.videoDuration = 0;
	mediaState.mediaDuration = (double)(reinterpret_cast<AVFormatContext*>(avContext)->duration) / AV_TIME_BASE;

	if (currentAudioStream)
	{
		if (reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->duration != AV_NOPTS_VALUE)
		{
			mediaState.audioDuration = (double)reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->duration * 
				av_q2d(reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->time_base);
		}
		else
		{
			mediaState.audioDuration = mediaState.mediaDuration;
		}
	}
	if (currentVideoStream)
	{
		if (reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->duration != AV_NOPTS_VALUE)
		{
			mediaState.videoDuration = (double)reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->duration * 
				av_q2d(reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->time_base);
		}
		else
		{
			mediaState.videoDuration = mediaState.mediaDuration;
		}
	}

	return true;
}

void DKAVMediaPlayer::Sync(void)
{
	SyncRenderer();
	SyncBufferController();
}

void DKAVMediaPlayer::SyncRenderer(void)
{
	DKAVMediaRenderer::RenderCommand rc = {DKAVMediaRenderer::RenderCommand::CommandNoop};
	renderer->mediaCommandQueue.ProcessMessage(rc);
}

void DKAVMediaPlayer::SyncBufferController(void)
{
	DKAVMediaBuffer::BufferCommand bc = {DKAVMediaBuffer::BufferCommand::CommandNoop};
	bufferController->bufferCommandQueue.ProcessMessage(bc);
}

bool DKAVMediaPlayer::Play(int count)
{
	CriticalSection guard(this->lock);

	if (mediaState.controlState == MediaState::ControlStatePlaying)
		return true;

	if (avContext && count > 0 && (currentAudioStream || currentVideoStream))
	{
		/*
		if (mediaState.progress > 0)
		{
		double duration = Duration();
		if (mediaState.progress >= duration || fabs(duration) < 0.05 || fabs(duration - mediaState.progress) < 0.05)
		mediaState.progress = 0;		// 처음부터 재생
		}
		*/

		mediaState.controlState = MediaState::ControlStatePlaying;
		if (IsSeekable())
			mediaState.playCount = count;
		else
			mediaState.playCount = 1;

		Register(mediaState);
		return true;
	}
	return false;
}

void DKAVMediaPlayer::Stop(void)
{
	CriticalSection guard(this->lock);

	mediaState.playCount = 0;
	Unregister();
}

double DKAVMediaPlayer::Duration(void) const
{
	CriticalSection guard(this->lock);
	if (avContext)
	{
		double audio = currentAudioStream ? StreamDuration(currentAudioStream->index) : 0.0;
		double video = currentVideoStream ? StreamDuration(currentVideoStream->index) : 0.0;

		double maxDuration = Max(audio, video);
		if (maxDuration > 0.0)
			return maxDuration;

		return (double)(reinterpret_cast<AVFormatContext*>(avContext)->duration) / AV_TIME_BASE;
	}
	return 0.0;
}

double DKAVMediaPlayer::StreamDuration(int index) const
{
	if (avContext && index >= 0)
	{
		if (reinterpret_cast<AVFormatContext*>(avContext)->streams[index]->duration != AV_NOPTS_VALUE)
		{
			return (double)reinterpret_cast<AVFormatContext*>(avContext)->streams[index]->duration * 
				av_q2d(reinterpret_cast<AVFormatContext*>(avContext)->streams[index]->time_base);
		}
	}
	return 0.0;
}

DKAudioSource* DKAVMediaPlayer::AudioSource(void)
{
	return audioOutput;
}

const DKAudioSource* DKAVMediaPlayer::AudioSource(void) const
{
	return audioOutput;
}

const DKTexture2D* DKAVMediaPlayer::VideoTexture(void) const
{
	const_cast<DKAVMediaPlayer*>(this)->RenderVideoInternal();
	return videoOutput;
}

void DKAVMediaPlayer::RenderVideoInternal(void)
{
	if (this->videoOutput)
	{
		DKObject<DKData> data = NULL;
		if (true)
		{
			CriticalSection guard(this->lock);
			data = this->textureData;
			this->textureData = NULL;
		}
		if (data)
		{
			const void* p = data->LockShared();
			size_t len = data->Length();
			int width = videoOutput->Width();
			int height = videoOutput->Height();

			DKASSERT_DEBUG(this->videoOutput->TextureFormat() == DKTexture::FormatRGB);
			DKASSERT_DEBUG(this->videoOutput->ComponentType() == DKTexture::TypeUnsignedByte);
			DKASSERT_DEBUG(width > 0);
			DKASSERT_DEBUG(height > 0);

			if (len >= (width * height * 3))
			{
				this->videoOutput->SetPixelData(DKRect(0,0,width,height), p);
			}
			data->UnlockShared();
		}
	}
}

DKAVMediaPlayer::MediaState DKAVMediaPlayer::State(void) const
{
	CriticalSection guard(this->lock);
	return mediaState;
}

void DKAVMediaPlayer::State(MediaState& st) const
{
	CriticalSection guard(this->lock);
	st = mediaState;
}

bool DKAVMediaPlayer::SetPosition(double pos)
{
	CriticalSection guard(this->lock);

	if (avContext == NULL)
		return false;

	if (!IsSeekable())
	{
		DKLog("[%s] source is not seekable.\n", DKLIB_FUNCTION_NAME);
		return false;
	}

	pos = Clamp<double>(pos, 0, mediaState.mediaDuration);

	if (mediaState.controlState == MediaState::ControlStatePlaying)
	{
		// mediaState 는 다른 쓰레드에서 변경될 수 있으므로 사본에 값을 저장한후 Register()를 호출해야 한다.
		MediaState st = mediaState;
		st.progress = pos;
		Register(st);
	}
	else
		mediaState.progress = pos;
	return true;
}

bool DKAVMediaPlayer::SetSpeed(double s)
{
	CriticalSection guard(this->lock);

	if (avContext == NULL)
		return false;

//	if (reinterpret_cast<AVFormatContext*>(avContext)->pb->is_streamed)
//	{
//		DKLog("[%s] source is streamed. cannot reposition.\n", DKLIB_FUNCTION_NAME);
//		return false;
//	}

	s = Max<double>(s, 0);

	mediaState.speed = s;

	if (mediaState.controlState == MediaState::ControlStatePlaying)
	{
		DKAVMediaRenderer::RenderCommand rc = {DKAVMediaRenderer::RenderCommand::CommandRescale, this,
			mediaState.minBufferLength, s};
		renderer->mediaCommandQueue.PostMessage(rc);
	}
	return true;
}

void DKAVMediaPlayer::ResizeBuffer(double minSize, double maxSize)
{
	CriticalSection guard(this->lock);

	minSize = Max<double>(minSize, 0.001);
	maxSize = Max<double>(minSize+1, maxSize);

	mediaState.minBufferLength = minSize;
	mediaState.maxBufferLength = maxSize;

	DKLog("%s buffer resized. min(%.3f) max(%.3f)\n", DKLIB_FUNCTION_NAME, mediaState.minBufferLength, mediaState.maxBufferLength);

	if (mediaState.controlState == MediaState::ControlStatePlaying)
	{
		DKAVMediaRenderer::RenderCommand rc = {DKAVMediaRenderer::RenderCommand::CommandRescale, this,
			mediaState.minBufferLength, mediaState.speed};
		renderer->mediaCommandQueue.PostMessage(rc);

		DKAVMediaBuffer::BufferCommand bc = {DKAVMediaBuffer::BufferCommand::CommandResize, this,
			mediaState.maxBufferLength};
		bufferController->bufferCommandQueue.PostMessage(bc);
	}
}

void DKAVMediaPlayer::OnBufferStateChanged(MediaState::BufferState state)
{
	CriticalSection guard(this->lock);

	DKString oldState = L"";
	DKString newState = L"";

	switch (mediaState.bufferState)
	{
	case MediaState::BufferStateStopped:
		oldState = L"Stopped";
		break;
	case MediaState::BufferStateFeeding:
		oldState = L"Feeding";
		break;
	}
	switch (state)
	{
	case MediaState::BufferStateStopped:
		newState = L"Stopped";
		break;
	case MediaState::BufferStateFeeding:
		newState = L"Feeding";
		break;
	}
	DKLog("DKAVMediaPlayer: Buffer State Changed. (%ls -> %ls)\n", (const wchar_t*)oldState, (const wchar_t*)newState);
	mediaState.bufferState = state;
}

void DKAVMediaPlayer::OnRenderSatateChanged(MediaState::RenderState state)
{
	CriticalSection guard(this->lock);

	DKString oldState = L"";
	DKString newState = L"";

	switch (mediaState.renderState)
	{
	case MediaState::RenderStateStopped:
		oldState = L"Stopped";
		break;
	case MediaState::RenderStatePlaying:
		oldState = L"Playing";
		break;
	case MediaState::RenderStateWaiting:
		oldState = L"Buffering";
		break;
	}
	switch (state)
	{
	case MediaState::RenderStateStopped:
		newState = L"Stopped";
		break;
	case MediaState::RenderStatePlaying:
		newState = L"Playing";
		break;
	case MediaState::RenderStateWaiting:
		newState = L"Buffering";
		break;
	}
	DKLog("DKAVMediaPlayer: Render State Changed. (%ls -> %ls) (time:%.3f / buffer:%.3f)\n",
		(const wchar_t*)oldState, (const wchar_t*)newState, mediaState.progress, Min(mediaState.bufferedAudio, mediaState.bufferedVideo));
	mediaState.renderState = state;

	if (state == MediaState::RenderStateStopped)
	{
		if (mediaState.playCount > 1 && IsSeekable())
		{
			MediaState st = mediaState;
			st.playCount--;
			st.progress = 0;
			DKLog("DKAVMediaPlayer Repeat (%d -> %d).\n", st.playCount+1, st.playCount);
			Register(st);
		}
		else
		{
			// 다음에 재생시 이어서 재생할 수 있도록 progress 는 초기화 안함.
			mediaState.playCount = 0;
			mediaState.controlState = MediaState::ControlStateStopped;
			DKLog("DKAVMediaPlayer Stopped (No repeat).\n");
			Unregister();
		}
	}
}

void DKAVMediaPlayer::UpdateVideoData(DKData* data)
{
	CriticalSection guard(this->lock);
	this->textureData = data;
}

void DKAVMediaPlayer::OnRender(double time, double audio, double video, double speed)
{
	CriticalSection guard(this->lock);

	if (mediaState.controlState == MediaState::ControlStatePlaying)
	{
		mediaState.progress = time;
		mediaState.audio = audio;
		mediaState.video = video;
	}
}

void DKAVMediaPlayer::OnRenderTimeRescaled(double speed, double minTime)
{
	DKLog("[DKAVMediaPlayer] Renderer time scale changed.(speed:%.3f, minReqBuffer:%.3f)\n", speed, minTime);
}

void DKAVMediaPlayer::OnBufferLengthResized(double maxTime)
{
	DKLog("[DKAVMediaPlayer] Buffer maximum length resized to %.3f\n", maxTime);
}

void DKAVMediaPlayer::OnAudioFilter(double time, void* data, size_t size, int channels, int frequency, int bits)
{
}

void DKAVMediaPlayer::OnBufferAudio(double time)
{
	CriticalSection guard(this->lock);

	mediaState.bufferedAudio = time;
}

void DKAVMediaPlayer::OnBufferVideo(double time)
{
	CriticalSection guard(this->lock);

	mediaState.bufferedVideo = time;
}

void DKAVMediaPlayer::Register(MediaState st)
{
	DKFoundation::DKObject<DKAVMediaStream> stream = DKObject<DKAVMediaStream>::New();

	// 버퍼링 쓰레드 등록
	int audioStreamIndex = -1;
	int videoStreamIndex = -1;
	if (currentAudioStream)
		audioStreamIndex = currentAudioStream->index;
	if (currentVideoStream)
		videoStreamIndex = currentVideoStream->index;

	DKAVMediaBuffer::BufferCommand bc =
	{
		DKAVMediaBuffer::BufferCommand::CommandRegister,	// command
		this,												// context
		st.maxBufferLength,									// 최대 버퍼 크기
		st.progress,										// streamPosition
		audioStreamIndex,
		videoStreamIndex,
		stream,
		IsSeekable()
	};
	bufferController->bufferCommandQueue.PostMessage(bc);

	// 렌더링 쓰레드 등록
	DKAVMediaRenderer::StreamInfo audio = {0,0,0,0,0};
	DKAVMediaRenderer::StreamInfo video = {0,0,0,0,0};
	if (currentAudioStream)
	{
		audio.context = reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->codec;
		audio.codec = reinterpret_cast<AVCodec*>(currentAudioStream->codec);
		audio.timeBase = av_q2d(reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->time_base);
		if (reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->start_time != AV_NOPTS_VALUE)
			audio.startTime = audio.timeBase * reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->start_time;
		else
			audio.startTime = 0;
		if (reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->duration != AV_NOPTS_VALUE)
			audio.duration = (double)reinterpret_cast<AVFormatContext*>(avContext)->streams[currentAudioStream->index]->duration * audio.timeBase;
		else
			audio.duration = (double)(reinterpret_cast<AVFormatContext*>(avContext)->duration) / AV_TIME_BASE;
	}
	if (currentVideoStream)
	{
		video.context = reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->codec;
		video.codec = reinterpret_cast<AVCodec*>(currentVideoStream->codec);
		video.timeBase = av_q2d(reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->time_base);
		if (reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->start_time != AV_NOPTS_VALUE)
			video.startTime = video.timeBase * reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->start_time;
		else
			video.startTime = 0;			
		if (reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->duration != AV_NOPTS_VALUE)
			video.duration = (double)reinterpret_cast<AVFormatContext*>(avContext)->streams[currentVideoStream->index]->duration * video.timeBase;
		else 
			video.duration = (double)(reinterpret_cast<AVFormatContext*>(avContext)->duration) / AV_TIME_BASE;
	}

	DKSize videoResolution = videoOutput ? videoOutput->Resolution() : DKSize(0,0);

	DKAVMediaRenderer::RenderCommand rc =
	{
		DKAVMediaRenderer::RenderCommand::CommandRegister,		// command
		this,													// context
		st.minBufferLength,										// minBufferLength
		st.speed,												// speed
		st.progress,											// start
		audio,													// audioStream
		video,													// videoStream
		videoResolution,
		stream,
		currentAudioStream ? audioOutput : NULL
	};
	renderer->mediaCommandQueue.PostMessage(rc);
	mediaState = st;
}

void DKAVMediaPlayer::Unregister(void)
{
	DKAVMediaRenderer::RenderCommand rc = {DKAVMediaRenderer::RenderCommand::CommandUnregister, this};
	renderer->mediaCommandQueue.PostMessage(rc);

	DKAVMediaBuffer::BufferCommand bc = {DKAVMediaBuffer::BufferCommand::CommandUnregister, this};
	bufferController->bufferCommandQueue.PostMessage(bc);

	mediaState.controlState = MediaState::ControlStateStopped;
}

bool DKAVMediaPlayer::IsSeekable(void) const
{
	if (avContext)
	{
		return reinterpret_cast<AVFormatContext*>(avContext)->pb->seekable & AVIO_SEEKABLE_NORMAL;
	}
	return false;
}

void DKAVMediaPlayer::SetVolume(float f)
{
	audioVolume = Min<float>(f, 0.0);
	audioOutput->SetGain(audioVolume);
}

float DKAVMediaPlayer::Volume(void) const
{
	return audioVolume;
}

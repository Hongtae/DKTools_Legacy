#include "DKAVMediaRenderer.h"
#include "DKAVMediaStream.h"
#include "DKAVMediaPlayer.h"
#include "FFmpeg/FFmpeg.h"

using namespace DKFoundation;
using namespace DKFramework;

#define DKLIB_AVMEDIA_AUDIO_BUFFER_COUNT		3		// 최대 오디오 버퍼 (3중 버퍼링)
#define DKLIB_AVMEDIA_AUDIO_BUFFER_TIME			1		// 오디오 버퍼 최대 크기 (1초)

namespace DKFramework
{
	namespace Private
	{
		////////////////////////////////////////////////////////////////////////////////
		// AudioConverter
		// 오디오 채널 변환
		// 2011-07-02: FFmpeg 의 ReSample 기능이 8 채널까지 지원하게 됨. (기존 채널 변환 코드 제거)
		////////////////////////////////////////////////////////////////////////////////
		class AudioConverter
		{
		public:
			AudioConverter(const AVCodecContext*c, int)
				: codec(c)
				, swrContext(NULL)
				, frequency(0)
				, resampledBuffer(NULL)
				, resampleMaxCount(0)
				, channelLayout(AV_CH_LAYOUT_MONO)
				, sampleFormat(AV_SAMPLE_FMT_NONE)
			{
				this->channelLayout = codec->channel_layout;
				this->sampleFormat = av_get_packed_sample_fmt(codec->sample_fmt);
				if (this->sampleFormat == AV_SAMPLE_FMT_NONE)
					this->sampleFormat = AV_SAMPLE_FMT_S16;

				// osx 에서 alGetEnumValue 가 문제가 있어서 16비트로 고정함.
				if (av_get_bytes_per_sample(this->sampleFormat) > 2)
					this->sampleFormat = AV_SAMPLE_FMT_S16;

				this->frequency = codec->sample_rate;
				int channels = codec->channels;

				int sampleBits = av_get_bytes_per_sample(this->sampleFormat) * 8;
				int format = DKAudioSource::Format(sampleBits, channels);
				if (format <= 0 && sampleBits > 16)	// 16비트로 변경해서 다시 시도
				{
					sampleBits = 16;
					this->sampleFormat = AV_SAMPLE_FMT_S16;
					format = DKAudioSource::Format(sampleBits, channels);
				}
				if (format <= 0 && channels > 2)	// 스테레오 또는 모노로 변경함
				{
					channels = 2;
					this->channelLayout = av_get_default_channel_layout(channels);
					format = DKAudioSource::Format(sampleBits, channels);
				}

				DKASSERT_DEBUG(channels > 0);
				DKASSERT_DEBUG(format != 0);

				DKASSERT_DEBUG(this->frequency > 0);
				DKASSERT_DEBUG(this->sampleFormat != AV_SAMPLE_FMT_NONE);
				DKASSERT_DEBUG(av_sample_fmt_is_planar(this->sampleFormat) == 0);

				this->sampleSource.channel_layout = 0;
				this->sampleSource.sample_fmt = AV_SAMPLE_FMT_NONE;
				this->sampleSource.sample_rate = 0;
			}
			~AudioConverter(void)
			{
				if (swrContext)
					swr_free(&swrContext);
				if (resampledBuffer)
					av_freep(&resampledBuffer[0]);
				av_freep(&resampledBuffer);
			}
			void InjectSilence(double t)		// 공백 넣음
			{
				UpdateResampleContext(codec);
				DKASSERT_DEBUG(swrContext != NULL);

				size_t numInject = this->frequency * t;
				swr_inject_silence(swrContext, numInject);
			}
			void DropSamples(double t)			// 시간t 만큼 빼버림
			{
				UpdateResampleContext(codec);
				DKASSERT_DEBUG(swrContext != NULL);

				size_t dropSamples = this->frequency * t;
				swr_drop_output(swrContext, dropSamples);
			}
			int Resample(const AVFrame* frame, DKArray<uint8_t>& output, int64_t& pts, double& length)
			{
				UpdateResampleContext(codec);
				DKASSERT_DEBUG(swrContext != NULL);

				int numChannels = av_get_channel_layout_nb_channels(this->channelLayout);
				// 변환 할 샘플 수 계산.
				int numSamples = av_rescale_rnd(swr_get_delay(swrContext, codec->sample_rate) +
					frame->nb_samples, this->frequency, codec->sample_rate, AV_ROUND_UP);

				if (resampledBuffer)
				{
					if (resampleMaxCount < numSamples)
					{
						av_freep(resampledBuffer[0]);
						av_samples_alloc(resampledBuffer, &resampleLinesize, numChannels, numSamples, this->sampleFormat, 1);
						resampleMaxCount = numSamples;
					}
				}
				else
				{
					av_samples_alloc_array_and_samples(&resampledBuffer, &resampleLinesize, numChannels, numSamples, this->sampleFormat, 1);
					resampleMaxCount = numSamples;
				}

				pts = swr_next_pts(swrContext, pts);
				int ret = swr_convert(swrContext, resampledBuffer, numSamples, (const uint8_t**)frame->extended_data, frame->nb_samples);
				if (ret <= 0)
				{
					length = 0.0;
					DKLog("swr_convert result: %d\n", ret);
				}
				else
				{
					length = double(ret) / double(this->frequency);

					int bufSize = av_samples_get_buffer_size(&resampleLinesize, numChannels, ret, this->sampleFormat, 1);
					if (bufSize > 0)
						output.Add(resampledBuffer[0], bufSize);
				}
				return ret;
			}
			size_t BytesOfTimeLength(double t) const
			{
				int base = av_get_channel_layout_nb_channels(this->channelLayout) * av_get_bytes_per_sample(this->sampleFormat);
				size_t oneSec = this->frequency * base;
				DKASSERT_DEBUG(oneSec > 0);

				size_t s = oneSec * t;
				size_t tmp = s % base;
				if (tmp > 0)
					s += base - tmp;
				return s;
			}
			double TimeLengthOfBytes(size_t s) const
			{
				int base = av_get_channel_layout_nb_channels(this->channelLayout) * av_get_bytes_per_sample(this->sampleFormat);
				size_t oneSec = this->frequency * base;
				DKASSERT_DEBUG(oneSec > 0);

				return static_cast<double>(s) / static_cast<double>(oneSec);
			}
			int Frequency(void) const						{ return frequency; }
			int Channels(void) const						{ return av_get_channel_layout_nb_channels(this->channelLayout); }
			int Bits(void) const							{ return av_get_bytes_per_sample(this->sampleFormat) * 8; }

		private:
			void UpdateResampleContext(const AVCodecContext* ctxt)
			{
				if (swrContext == NULL)
					swrContext = swr_alloc();

				if (ctxt->channel_layout != this->sampleSource.channel_layout ||
					ctxt->sample_rate != this->sampleSource.sample_rate ||
					ctxt->sample_fmt != this->sampleSource.sample_fmt)
				{
					this->sampleSource.channel_layout = ctxt->channel_layout;
					this->sampleSource.sample_fmt = ctxt->sample_fmt;
					this->sampleSource.sample_rate = ctxt->sample_rate;

					av_opt_set_int(swrContext, "in_channel_layout", ctxt->channel_layout, 0);
					av_opt_set_int(swrContext, "in_sample_rate", ctxt->sample_rate, 0);
					av_opt_set_sample_fmt(swrContext, "in_sample_fmt", ctxt->sample_fmt, 0);

					av_opt_set_int(swrContext, "out_sample_rate", this->frequency, 0);
					av_opt_set_int(swrContext, "out_channel_layout", this->channelLayout, 0);
					av_opt_set_sample_fmt(swrContext, "out_sample_fmt", this->sampleFormat, 0);

					if (swr_init(swrContext) < 0)
						DKERROR_THROW("swr_init failed!");

					DKLog(
						"AudioConverter Resample Format:(%s -> %s)\n"
						"AudioConverter Resample Channels:(%d -> %d)\n"
						"AudioConverter Resample Frequency:(%d -> %d)\n",
						av_get_sample_fmt_name(this->sampleSource.sample_fmt),
						av_get_sample_fmt_name(this->sampleFormat),
						av_get_channel_layout_nb_channels(this->sampleSource.channel_layout),
						av_get_channel_layout_nb_channels(this->channelLayout),
						this->sampleSource.sample_rate,
						this->frequency);
				}
			}

			const AVCodecContext* codec;
			SwrContext* swrContext;

			AVSampleFormat	sampleFormat;
			int				frequency;
			int				channelLayout;
			uint8_t**		resampledBuffer;
			int				resampleMaxCount;
			int				resampleLinesize;

			// AVCodecContext 가 디코딩 중에 바뀔수 있다. 그럴경우 resampleContext 를 다시 생성해야 한다.
			// 아래 구조체에 resampleContext 생성할때의 AVCodecContext 정보를 저장함. 중간에 바뀌면 재생성.
			struct
			{
				AVSampleFormat	sample_fmt;
				int				sample_rate;
				int				channel_layout;
			} sampleSource;
		};

		////////////////////////////////////////////////////////////////////////////////
		// VideoConverter
		// 비디오 변환 (AVFrame 에서 OpenGL 텍스쳐에 업데이트)
		////////////////////////////////////////////////////////////////////////////////
		class VideoConverter
		{
		public:
			VideoConverter(const AVCodecContext* c, int w, int h)
				: codec(c)
				, frame(NULL)
				, buffer(NULL)
				, context(NULL)
				, width(w)
				, height(h)
			{
				DKASSERT_DEBUG(codec != NULL);
				DKASSERT_DEBUG(width > 0 && height > 0);

				int numBytes = avpicture_get_size(PIX_FMT_RGB24, width, height);
				buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
				frame = avcodec_alloc_frame();
				context = sws_getContext(codec->width, codec->height, codec->pix_fmt, width, height, PIX_FMT_RGB24, SWS_FAST_BILINEAR, 0, 0, 0);
				avpicture_fill((AVPicture*)frame, buffer, PIX_FMT_RGB24, width, height);
			}
			~VideoConverter(void)
			{
				av_free(frame);
				av_free(buffer);
				sws_freeContext(context);
			}
			DKObject<DKData> Convert(const AVFrame* input) const
			{
				DKASSERT_DEBUG(context);
				DKASSERT_DEBUG(codec);
				DKASSERT_DEBUG(frame);
				DKASSERT_DEBUG(buffer);

				DKASSERT_DEBUG(input != NULL);

				uint8_t* data[4] = { frame->data[0] + width * 3 * (height - 1), 0, 0, 0 };
				int stride[4] = { -frame->linesize[0], 0, 0, 0 };

				// 네이티브 포맷에서 RGB 로 변환
				if (sws_scale(context, input->data, input->linesize, 0, codec->height, data, stride) > 0)
				{
					return DKBuffer::Create(frame->data[0], avpicture_get_size(PIX_FMT_RGB24, width, height)).SafeCast<DKData>();
				}
				return NULL;
			}
		private:
			const AVCodecContext* codec;
			SwsContext* context;		// 프레임 변환에 사용되는 인코더 (sws)
			AVFrame* frame;				// RGB 값 임시로 저장
			uint8_t* buffer;			// 프레임 변환할때 사용되는 버퍼.
			int width;
			int height;
		};

		class MediaStreamContext
		{
		public:
			MediaStreamContext(const DKAVMediaRenderer::StreamInfo& audioStream, const DKAVMediaRenderer::StreamInfo& videoStream, const DKSize& texRes)
				: audio(audioStream)
				, video(videoStream)
				, decodedVideoFrame(NULL)
				, audioConverter(NULL)
				, videoConverter(NULL)
				, nextAudioPTS(0)
				, nextVideoPTS(0)
				, gotPicture(false)
				, audioStreamFinish(true)
				, videoStreamFinish(true)
				, dropVideoPacket(false)
				, lastAudioSync(0)
				, dropPacketTimeInterval(2.0)
			{
				if (audio.context)
				{
					DKASSERT(audio.codec);
					DKASSERT(audio.context->codec_type == AVMEDIA_TYPE_AUDIO);

					if (avcodec_open2(audio.context, audio.codec, NULL) < 0)
						DKERROR_THROW("Cannot open codec!");

					audioConverter = new AudioConverter(audio.context, 8);

					const size_t audioFrameSize = 192000;	// 1 second of 48khz 32bit audio
					decodedAudioBuffer.Reserve(audioFrameSize);
					audioStreamFinish = false;

					DKASSERT_DEBUG(audioConverter != NULL);
				}

				if (video.context)
				{
					DKASSERT(video.codec);
					DKASSERT(video.context->codec_type == AVMEDIA_TYPE_VIDEO);

					if (avcodec_open2(video.context, video.codec, NULL) < 0)
						DKERROR_THROW("Cannot open codec!");

					int width = texRes.width;
					int height = texRes.height;

					DKASSERT(width > 0 && height > 0);

					videoConverter = new VideoConverter(video.context, width, height);

					decodedVideoFrame = avcodec_alloc_frame();
					videoStreamFinish = false;

					DKASSERT_DEBUG(videoConverter != NULL);
				}
			}
			~MediaStreamContext(void)
			{
				if (audioConverter)
					delete audioConverter;
				if (videoConverter)
					delete videoConverter;

				audioConverter = NULL;
				videoConverter = NULL;

				if (decodedVideoFrame)
					av_free(decodedVideoFrame);

				if (audio.context)
				{
					DKASSERT_DEBUG(audio.codec);
					DKASSERT_DEBUG(audio.context->codec_type == AVMEDIA_TYPE_AUDIO);

					avcodec_flush_buffers(audio.context);
					avcodec_close(audio.context);
				}
				if (video.context)
				{
					DKASSERT_DEBUG(video.codec);
					DKASSERT_DEBUG(video.context->codec_type == AVMEDIA_TYPE_VIDEO);

					avcodec_flush_buffers(video.context);
					avcodec_close(video.context);
				}
			}
			void ResetTime(void)
			{
				nextAudioPTS = 0;
				nextVideoPTS = 0;
				gotPicture = false;
				dropVideoPacket = false;
				lastAudioSync = 0;
				decodedAudioBuffer.Clear();

				audioStreamFinish = audio.codec == NULL;
				videoStreamFinish = video.codec == NULL;
			}

			DKAVMediaRenderer::StreamInfo	audio;
			DKAVMediaRenderer::StreamInfo	video;
			AVFrame*						decodedVideoFrame;		// 변환된 비디오 프레임
			DKArray<uint8_t>				decodedAudioBuffer;		// 변환된 오디오 버퍼
			AudioConverter*					audioConverter;			// 오디오 채널,비트 변환
			VideoConverter*					videoConverter;			// AVFrame -> texture converter
			double							nextAudioPTS;			// 다음번 갱신되어야 할 타임
			double							nextVideoPTS;			// 다음번 갱신되어야 할 타임 (경과시간+시작시간 이 크면 갱신)
			bool							gotPicture;				// 변환된 비디오 프레임이 대기중
			bool							audioStreamFinish;		// 오디오 스트림 종료됨
			bool							videoStreamFinish;		// 비디오 스트림 종료됨
			bool							dropVideoPacket;		// 비디오 패킷 드롭 (시간 지난것들)
			double							lastAudioSync;
			double							dropPacketTimeInterval;		// 드롭할 패킷의 시간차
		};

		struct MediaStreamInfo
		{
			DKAVMediaPlayer*					context;
			DKObject<DKAudioSource>				audioOutput;
			DKObject<DKData>					videoOutput;
			DKObject<MediaStreamContext>		streamContext;
			DKObject<DKAVMediaStream>			stream;
			double								minLength;			// 렌더링 하기 최소 버퍼 시간
			double								progress;			// 재생 시간 (seek 하게되면 이 값을 변경해야함)
			double								speed;				// 재생속도
			double								audioTime;
			double								videoTime;
			bool								audioBufferEOF;
			bool								videoBufferEOF;
			bool								waiting;			// 버퍼 기다리는중
		};
	}
}

using namespace DKFramework;
using namespace DKFramework::Private;

DKAVMediaRenderer::DKAVMediaRenderer(void)
{
	playbackThread = DKThread::Create(DKFunction(this, &DKAVMediaRenderer::PlaybackThreadProc)->Invocation());
	if (playbackThread == NULL)
		DKERROR_THROW("Playback Thread Creation Failed.");

	DKLog("DKAVMediaRenderer:0x%x Created.\n", this);
}

DKAVMediaRenderer::~DKAVMediaRenderer(void)
{
	if (playbackThread->IsAlive())
	{
		RenderCommand rc = {RenderCommand::CommandQuit};
		mediaCommandQueue.PostMessage(rc);
		playbackThread->WaitTerminate();
	}
	playbackThread = NULL;

	DKLog("DKAVMediaRenderer:0x%x Destroyed.\n", this);
}

void DKAVMediaRenderer::RenderMediaStream(MediaStreamInfo& media, double timeDelta)
{
	if (media.streamContext->audioStreamFinish && media.streamContext->videoStreamFinish)		// 모든 재생이 끝났음.
	{
		double audioDuration = (media.streamContext->audio.context) ? media.streamContext->audio.duration : 0.0;
		double videoDuration = (media.streamContext->video.context) ? media.streamContext->video.duration : 0.0;
		double progress = Max<double>(audioDuration, videoDuration);
		// 종료전 마지막 상태 갱신함.
		media.context->OnRender(Max<double>(media.progress, progress), media.audioTime, media.videoTime, media.speed);

		RenderCommand rc = {RenderCommand::CommandUnregister, media.context};
		mediaCommandQueue.PostMessage(rc);

		DKLog("[DKAVMediaRenderer] Context(%x) Rendering Stopped. (stream finished)\n", media.context);
		return;
	}

	double sync = 0;
	if (media.stream->GetSyncRequest(sync))
	{
		DKLog("DKAVMediaRenderer: Time Sync (%.3f -> %.3f)\n", media.progress, sync);
		media.progress = sync;
		media.streamContext->ResetTime();
		return;
	}

	media.stream->SetRenderTime(media.progress);

	////////////////////////////////////////////////////////////////////////////////
	// 오디오 패킷 디코딩
	// 버퍼링 기다리는 중에도 오디오 패킷은 미리 디코딩 해놓는다. DKLIB_AVMEDIA_AUDIO_BUFFER_TIME 길이만큼만.
	if (media.streamContext->audio.context && !media.streamContext->audioStreamFinish)
	{
		AVCodecContext* codecContext = media.streamContext->audio.context;
		DKArray<uint8_t>& outputAudio = media.streamContext->decodedAudioBuffer;
		// 필요한 최소 버퍼 길이
		const size_t audioBufferMinLength = media.streamContext->audioConverter->BytesOfTimeLength(DKLIB_AVMEDIA_AUDIO_BUFFER_TIME);

		AVPacket packet;
		while (outputAudio.Count() < audioBufferMinLength && media.stream->PopAudioPacket(packet))
		{
			if (DKAVMediaStream::IsFlush(packet))
			{
				outputAudio.Clear();
				avcodec_flush_buffers(codecContext);
				DKLog("DKAVMediaRenderer Context(0x%x) flush audio buffer.\n", media.context);
			}
			else if (DKAVMediaStream::IsEof(packet))
			{
				media.audioBufferEOF = true;
				DKLog("DKAVMediaRenderer Context(0x%x) Audio EOF.\n", media.context);
			}
			else
			{
				AVPacket packetCopy = packet;	// size, data 를 수정하므로 복사해야한다.
				while ( packetCopy.size > 0 )
				{
					AVFrame frame;
					int gotFrame = 0;
					int decoded = avcodec_decode_audio4(codecContext, &frame, &gotFrame, &packetCopy);
					if (decoded < 0)
					{
						DKLog("DKAVMediaRenderer ERROR: avcodec_decode_audio4 error?\n");
						break;
					}
					packetCopy.size -= decoded;
					packetCopy.data += decoded;

					if (gotFrame)
					{
						int64_t pts = frame.pts;
						if (pts == AV_NOPTS_VALUE)
							pts = frame.pkt_pts;
						if (pts == AV_NOPTS_VALUE)
							pts = frame.pkt_dts;

						if (outputAudio.IsEmpty() && pts == AV_NOPTS_VALUE)		// 시간을 알수 없는 패킷.. 무시함
						{
							DKLog("DKAVMediaRenderer Warning: Unknown PTS packet! (ignored)\n");
						}
						else
						{
							double start = media.streamContext->nextAudioPTS;
							double length = 0;

							if (pts != AV_NOPTS_VALUE)
								start = media.streamContext->audio.timeBase * pts - media.streamContext->audio.startTime;

							int64_t pts_offset = 0;
							media.streamContext->audioConverter->Resample(&frame, outputAudio, pts_offset, length);
							if (pts_offset > 0)
							{
								start += media.streamContext->audio.timeBase * pts_offset;
								DKLog("pts offset: %lld\n", pts_offset);
							}
							// timeOffset : 가지고 있는 버퍼의 시간 길이
							double timeOffset = media.streamContext->audioConverter->TimeLengthOfBytes(outputAudio.Count());
							media.streamContext->nextAudioPTS = start - timeOffset;				// 시간 재설정

							if (start + length < media.progress)
							{
								double d = media.progress - (start + length);
								// 시간 d 만큼 버림.
								media.streamContext->audioConverter->DropSamples(d);
								DKLog("DKAVMediaRenderer dropping audio sample %f sec\n", d);
							}
						}
					}				 
				}
				// 패킷 해제
				av_free_packet(&packet);
			}
		}
	}

	// 버퍼링 여부 결정
	if (media.minLength > 0.0 && !media.stream->HasEOF())
	{
		double effectiveBufferLen = media.minLength;
		if (media.streamContext->audio.context && media.streamContext->audio.duration > 0)
			effectiveBufferLen = Min(effectiveBufferLen, media.streamContext->audio.duration - media.progress);
		if (media.streamContext->video.context && media.streamContext->video.duration > 0)
			effectiveBufferLen = Min(effectiveBufferLen, media.streamContext->video.duration - media.progress);

		double audioBufferTime = media.stream->GetAudioBufferTime();
		double videoBufferTime = media.stream->GetVideoBufferTime();
		double maxBufferTime = Max(audioBufferTime, videoBufferTime);

		if (media.waiting)		// 버퍼 대기중
		{
			if (media.progress + effectiveBufferLen > maxBufferTime)		// 버퍼가 더 필요
			{
				return;
			}
			else		// 재생 상태로 변경
			{
				media.waiting = false;
				if (media.audioOutput)
					media.audioOutput->Play();
				media.context->OnRenderSatateChanged(DKAVMediaPlayer::MediaState::RenderStatePlaying);
			}
		}
		else		// 재생중 (버퍼가 모두 소진될때까지 재생한다)
		{
			bool audioReady = true; 
			bool videoReady = true;

			if (media.streamContext->audio.context && media.stream->GetAudioBufferTime() < media.progress + media.minLength)
			{
				if (media.streamContext->decodedAudioBuffer.IsEmpty() &&
					media.audioOutput->QueuedBuffers() == 0 &&
					media.streamContext->nextAudioPTS < media.progress &&
					media.stream->GetAudioPacketCount() == 0)
					audioReady = false; 
			} 
			if (media.streamContext->video.context && media.stream->GetVideoBufferTime() < media.progress + media.minLength)
			{ 
				if (media.stream->GetVideoPacketCount() == 0) 
					videoReady = false; 
			}
			if (audioReady == false && videoReady == false)		// 버퍼링 모드로 전환
			{
				media.waiting = true;
				if (media.audioOutput)
					media.audioOutput->Pause();
				media.context->OnRenderSatateChanged(DKAVMediaPlayer::MediaState::RenderStateWaiting);
				return;
			}
		}		
	}
	else if (media.waiting)
	{
		media.waiting = false;
		if (media.audioOutput)
			media.audioOutput->Play();
		media.context->OnRenderSatateChanged(DKAVMediaPlayer::MediaState::RenderStatePlaying);
	}

	////////////////////////////////////////////////////////////////////////////////
	// 오디오 피딩
	// DKLIB_AVMEDIA_AUDIO_BUFFER_TIME 만큼 데이터를 OpenAL 에 넣는다.
	if (media.streamContext->audio.context && !media.streamContext->audioStreamFinish)
	{
		DKArray<unsigned char>& outputAudio = media.streamContext->decodedAudioBuffer;

		// 전체 버퍼 갯수 알아오기
		size_t queuedBuffers = media.audioOutput->QueuedBuffers();

		if (queuedBuffers == 0 && media.streamContext->nextAudioPTS > media.progress)
		{
			// 아직 오디오 시작하면 안된다.
		}
		else
		{
			if (queuedBuffers < DKLIB_AVMEDIA_AUDIO_BUFFER_COUNT)		// 버퍼가 큐에 별로 없음. 버퍼 공급
			{
				size_t audioBufferMinLength = media.streamContext->audioConverter->BytesOfTimeLength(DKLIB_AVMEDIA_AUDIO_BUFFER_TIME);

				// 오디오 버퍼 크기를 일정하게 만들려면, 오디오 데이터가 최소 버퍼링 크기를 넘어서야 OpenAL 에 넣어야 한다.
				if (outputAudio.Count() > 0 && (outputAudio.Count() >= audioBufferMinLength || media.audioBufferEOF))
				{
					size_t bufferSize = Min<size_t>(audioBufferMinLength, outputAudio.Count());		// EOF 일때는 더 작은 크기를 사용할 수 있음.

					// 오디오 필터링
					media.context->OnAudioFilter(media.streamContext->nextAudioPTS, (uint8_t*)outputAudio, bufferSize,
						media.streamContext->audioConverter->Channels(),
						media.streamContext->audioConverter->Frequency(),
						media.streamContext->audioConverter->Bits());

					// 버퍼 공급
					media.audioOutput->EnqueueBuffer(media.streamContext->audioConverter->Frequency(),
						media.streamContext->audioConverter->Bits(),
						media.streamContext->audioConverter->Channels(),
						(uint8_t*)outputAudio, bufferSize, media.streamContext->nextAudioPTS);

					// bufferSize 만큼 앞으로 땡김.
					outputAudio.Remove(0, bufferSize);

					double length = media.streamContext->audioConverter->TimeLengthOfBytes(bufferSize);
					media.streamContext->nextAudioPTS += length;

					// 상태확인
					if (media.audioOutput->State() != DKAudioSource::StatePlaying)
						media.audioOutput->Play();

					int err = DKAudioSource::ErrorCode();
					if (err != 0)
					{
						DKLog("DKAVMediaRenderer AudioError: %ls\n", (const wchar_t*)DKAudioSource::ErrorString(err));
					}
				}
			}
			else if (queuedBuffers > 0)
			{
				// 큐에 버퍼가 있음. 상태확인
				if (media.audioOutput->State() != DKAudioSource::StatePlaying)
					media.audioOutput->Play();
			}
		}
		// 상태확인 (종료 확인)
		DKAudioSource::AudioState state = media.audioOutput->State();
		if (state != DKAudioSource::StatePlaying && media.audioBufferEOF && queuedBuffers == 0)
		{
			DKLog("Audio Stream Finished.\n");
			media.streamContext->audioStreamFinish = true;
		}
		else
		{
			double audioTime = media.audioOutput->TimePosition();
			if (audioTime > 0)
			{
				double audioDiff = media.progress - audioTime;

				double syncInterval = Max<double>(0.3, 0.3 * media.speed);	// 이 시간 이내에는 다시 동기화 하지 않음 (맥에서는 비동기식이라 바로 적용이 되지 않는다)
				double syncTimeDiff = Max<double>(0.1, 0.1 * media.speed);	// 다시 동기화할 시간 간격
				if (fabs(audioDiff) > syncTimeDiff && (media.progress - media.streamContext->lastAudioSync) > syncInterval)
				{
					double playingOffset = media.audioOutput->TimeOffset();
					float timeOffset = Clamp<float>(playingOffset + audioDiff, 0, DKLIB_AVMEDIA_AUDIO_BUFFER_TIME - 0.1);	// 최대값을 DKLIB_AVMEDIA_AUDIO_BUFFER_TIME 로 잡으면 OpenAL 버퍼가 꼬임 (맥 버그?)
					media.audioOutput->SetTimeOffset(timeOffset);

					DKLog("[DKAVMediaRenderer] Audio sync %.3f (%.3f/%.3f) (offset:%.3f)\n", audioDiff, audioTime, media.progress, timeOffset);

					int err = DKAudioSource::ErrorCode();
					if (err)
					{
						DKLog("DKAVMediaRenderer AudioSource Error: %ls\n", (const wchar_t*)DKAudioSource::ErrorString(err));
					}

					// 큐에 있는 버퍼들중 시간 지난거 모두 버린다.
					const size_t expiredBufferLength = media.streamContext->audioConverter->BytesOfTimeLength(DKLIB_AVMEDIA_AUDIO_BUFFER_TIME);
					while (media.streamContext->nextAudioPTS + DKLIB_AVMEDIA_AUDIO_BUFFER_TIME < media.progress && outputAudio.Count() > expiredBufferLength)
					{
						outputAudio.Remove(0, expiredBufferLength);
						DKLog("[DKAVMediaRenderer] flushing unqueued audio buffer %u bytes.\n", expiredBufferLength);
						media.streamContext->nextAudioPTS += DKLIB_AVMEDIA_AUDIO_BUFFER_TIME;
					}
					media.streamContext->lastAudioSync = media.progress;					
				}				
			}
			else
				audioTime = media.streamContext->nextAudioPTS;

			media.audioTime = audioTime;			
			//DKLog("media.autioTime = %f\n", media.audioTime);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	// 비디오 디코딩, 출력
	// 하나의 프레임이 될때까지 패킷을 읽어서 디코딩 하며, 디코딩한 프레임의 재생시간이 되기 전엔 더이상 읽지 않는다.
	if (media.streamContext->video.context && media.progress >= media.streamContext->nextVideoPTS && !media.streamContext->videoStreamFinish)
	{
		if (media.streamContext->gotPicture)
		{
			if (media.videoOutput)
				media.context->UpdateVideoData(media.videoOutput);

			media.videoOutput = NULL;
			media.streamContext->gotPicture = false;
			media.videoTime = media.streamContext->nextVideoPTS;
		}

		AVPacket packet;
		while (media.stream->PopVideoPacket(packet))
		{
			if (DKAVMediaStream::IsFlush(packet))
			{
				avcodec_flush_buffers(media.streamContext->video.context);
				DKLog("[DKAVMediaRenderer] Context(%x) flush video buffer.\n", media.context);
			}
			else if (DKAVMediaStream::IsEof(packet))
			{
				media.videoBufferEOF = true;
				DKLog("[DKAVMediaRenderer] Context(%x) Video EOF.\n", media.context);
			}
			else
			{
				if (media.streamContext->dropVideoPacket)
				{
					// 시간 지난 패킷들 모두 제거
					double pts = 0;

					if (packet.pts != AV_NOPTS_VALUE)
						pts = (double)packet.pts * media.streamContext->video.timeBase - media.streamContext->video.startTime;
					else if (packet.dts != AV_NOPTS_VALUE)
						pts = (double)packet.dts * media.streamContext->video.timeBase - media.streamContext->video.startTime;

					if (pts > media.progress)
					{
						media.streamContext->dropVideoPacket = false;
						DKLog("DKAVMediaRenderer video packet dropped. (%.2f -> %.2f)\n", media.streamContext->nextVideoPTS, media.progress);
					}					
				}
				if (media.streamContext->dropVideoPacket == false)
				{					
					int gotPicture = 0;

					int ret = avcodec_decode_video2(media.streamContext->video.context, media.streamContext->decodedVideoFrame, &gotPicture, &packet);

					if (ret < 0)	// error?
					{
						DKLog("avcodec_decode_video2 Error???????\n");
						media.streamContext->gotPicture = 0;
						break;
					}
					else
					{
						if (gotPicture)
						{
							AVFrame* frame = media.streamContext->decodedVideoFrame;
							int64_t timeStamp = av_frame_get_best_effort_timestamp(frame);
							if (timeStamp == AV_NOPTS_VALUE)
								timeStamp = frame->pkt_pts;
							if (timeStamp == AV_NOPTS_VALUE)
								timeStamp = frame->pkt_dts;

							double pts = 0;
							if (timeStamp != AV_NOPTS_VALUE)
							{
								pts = media.streamContext->video.timeBase * timeStamp - media.streamContext->video.startTime;
							}
							else
							{
								DKLog("DKAVMediaRenderer warning: video has no pts!\n");
								pts = media.progress;
							}

							// 프레임 pts 가 현재시간과 media.streamContext->dropPacketTimeInterval 이상 느리다면 패킷 드롭
							if (pts > 0 && media.progress > pts + media.streamContext->dropPacketTimeInterval)
							{
								media.streamContext->dropVideoPacket = true;		// 패킷 드롭
							}
							else if (pts >= media.progress)			// 이미지를 그린다.
							{
								media.streamContext->nextVideoPTS = pts;
								media.streamContext->gotPicture = true;
							}
							else
							{
								DKLog("DKAVMediaRenderer video frame dropped. (%.2f -> %.2f)\n", pts, media.progress);
							}
						}
					}
				}
				av_free_packet(&packet);
				if (media.streamContext->gotPicture)
				{
					DKASSERT_DEBUG(media.streamContext->videoConverter);
					media.videoOutput = media.streamContext->videoConverter->Convert(media.streamContext->decodedVideoFrame);
					break;
				}
			}
		}
		if (!media.streamContext->gotPicture && media.videoBufferEOF)
		{
			DKLog("Video Stream Finished.\n");
			media.streamContext->videoStreamFinish = true;
		}	
	}

	// 재생 상태 출력
	media.context->OnRender(media.progress, media.audioTime, media.videoTime, media.speed);
	media.progress += timeDelta;
}

void DKAVMediaRenderer::PlaybackThreadProc(void)
{
//	DKContextScopeBinder<DKOpenGLContext> videoBinder(DKOpenGLContext::SharedInstance());
	DKContextScopeBinder<DKOpenALContext> audioBinder(DKOpenALContext::SharedInstance());

	DKArray<MediaStreamInfo>	playlist;			// 재생 목록들
	DKTimer	timer;					// 타이머
	float updateInterval = 0.001f;

	while ( 1 )
	{
		DKObject<MessageQueue::Receiver> recp = NULL;
		if (playlist.IsEmpty())
			recp = this->mediaCommandQueue.GetMessage(true);
		else
			recp = this->mediaCommandQueue.GetMessageTimeOut(updateInterval - timer.Elapsed());
		if (recp)
		{
			RenderCommand& mesg = recp->Content();
			if (mesg.command == RenderCommand::CommandQuit)
			{
				// 모든 목록의 소스를 제거하고 종료준비
				for (int i = 0; i < playlist.Count(); i++)
				{
					MediaStreamInfo& media = playlist.Value(i);
					media.audioOutput->Stop();
					DKLog("[DKAVMediaRenderer] Context:%x Unregistered.\n", media.context);
					media.context->OnRenderSatateChanged(DKAVMediaPlayer::MediaState::RenderStateStopped);					
				}
				playlist.Clear();
				recp->SetResult(0);
				break;		// 쓰레드 종료!
			}
			////////////////////////////////////////////////////////////////////////////////
			// 그외 메시지들
			int result = 0;
			switch (mesg.command)
			{
			case RenderCommand::CommandNoop:		// 아무일도 안함. (응답확인용)
				break;
			case RenderCommand::CommandUnregister:		// 목록에서 제거
				for (int i = 0; i < playlist.Count(); i++)
				{
					MediaStreamInfo& media = playlist.Value(i);
					if (media.context == recp->Content().context)
					{
						if (media.audioOutput)
							media.audioOutput->Stop();
						media.audioOutput = NULL;
						media.videoOutput = NULL;

						DKLog("[DKAVMediaRenderer] Context:%x Unregistered.\n", media.context);
						media.context->OnRenderSatateChanged(DKAVMediaPlayer::MediaState::RenderStateStopped);
						playlist.Remove(i);
						break;
					}
				}
				break;
			case RenderCommand::CommandRegister:		// 재생 목록에 추가
				if (recp->Content().context)
				{
					// 목록에 존재하는지 검사. (존재하면 지운다. 지우지 않으면 코덱 충돌-이미 열어놨는데 또 열어서 실패함.)
					for (int i = 0; i < playlist.Count(); i++)
					{
						MediaStreamInfo& media = playlist.Value(i);
						if (media.context == recp->Content().context)
						{
							if (media.audioOutput)
								media.audioOutput->Stop();
							media.audioOutput = NULL;
							media.videoOutput = NULL;

							DKLog("[DKAVMediaRenderer] Context:%x Unregistered.\n", media.context);
							playlist.Remove(i);
							break;
						}
					}

					try
					{
						MediaStreamInfo media = 
						{
							recp->Content().context,
							recp->Content().audioOutput,
							NULL,
							DKOBJECT_NEW MediaStreamContext(recp->Content().audioStream, recp->Content().videoStream, recp->Content().videoSize),
							recp->Content().stream,
							Max<double>(recp->Content().minLength, 0.001),
							recp->Content().start,
							Max<double>(recp->Content().speed, 0),
							0,
							0,
							false,
							false,
							true
						};

						DKASSERT_DEBUG( media.streamContext->audio.context ? media.audioOutput != NULL : media.audioOutput == NULL);
						DKASSERT_DEBUG( media.streamContext->video.context ? media.streamContext->videoConverter != NULL : media.streamContext->videoConverter == NULL);

						if (media.speed < 0.001)
							media.speed = 0;

						if (media.streamContext->audio.context)
						{
							DKASSERT_DEBUG(media.audioOutput != NULL);

							if (media.speed > 0)
							{
								media.audioOutput->SetPitch(media.speed);
								media.speed = media.audioOutput->Pitch();
							}
							else
							{
								media.audioOutput->Pause();
							}
						}

						playlist.Add(media);
						DKLog("[DKAVMediaRenderer] Context:%x Registered. (speed:%.3f, buffer:%.3f)\n", media.context, media.speed, media.minLength);
						media.context->OnRenderSatateChanged(DKAVMediaPlayer::MediaState::RenderStateWaiting);
					}
					catch (DKError& err)
					{
						DKLog("DKAVMediaRenderer received exception while registering context: 0x%x\n", recp->Content().context);

						RenderCommand rc = {RenderCommand::CommandUnregister,recp->Content().context};
						mediaCommandQueue.PostMessage(rc);

						if (!DKFoundation::IsDebuggerPresent())
							err.PrintDescriptionWithStackFrames();
					}
				}
				break;
			case RenderCommand::CommandRescale:
				if (recp->Content().context)
				{
					for (int i = 0; i < playlist.Count(); i++)
					{
						MediaStreamInfo& media = playlist.Value(i);
						if (media.context == recp->Content().context)
						{
							double oldLength = media.minLength;
							double oldSpeed = media.speed;

							media.speed = Max<double>(recp->Content().speed, 0);
							media.minLength = Max<double>(recp->Content().minLength, 0.001);
							media.streamContext->dropVideoPacket = false;

							if (media.speed < 0.001)
								media.speed = 0;

							DKLog("[DKAVMediaRenderer] Context:%x Rescaled (Speed:%.2f -> %.2f, Buffer:%.3f -> %.3f).\n",
								media.context, oldSpeed, media.speed, oldLength, media.minLength);

							if (media.streamContext->audio.context)
							{
								DKASSERT_DEBUG(media.audioOutput != NULL);
								if (media.speed > 0)
								{
									media.audioOutput->SetPitch(media.speed);
									media.speed = media.audioOutput->Pitch();
								}
								else
								{
									media.audioOutput->Pause();
								}
							}

							media.context->OnRenderTimeRescaled(media.speed, media.minLength);
							break;
						}
					}
				}
				break;
			default:
				DKLog("[DKAVMediaRenderer] Warning:Unknown command:%x\n", mesg.command);
				break;
			}
			// 모든 메시지에 응답
			recp->SetResult(result);
		}
		else		// if (bMesgReceived)
		{
			double elapsed = timer.Reset();

			// 스트리밍 한다.
			for (int i = 0; i < playlist.Count(); i++)
			{
				MediaStreamInfo& media = playlist.Value(i);
				if (media.speed > 0.001)
					this->RenderMediaStream(media, elapsed * media.speed);
			}
			DKThread::Yield();
		}
	}
	DKLog("[DKAVMediaRenderer] (thread:%x) Playback Terminated.\n", DKThread::CurrentThreadId());
}

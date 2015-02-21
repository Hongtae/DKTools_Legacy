#include "FFmpeg/FFmpeg.h"
#include "DKAVMediaStream.h"
#include "DKAVMediaBuffer.h"
#include "DKAVMediaPlayer.h"

using namespace DKFoundation;
using namespace DKFramework;

namespace DKFramework
{
	namespace Private
	{
		namespace
		{
			struct StreamBufferInfo
			{
				DKAVMediaPlayer*			context;
				int							audioStreamIndex;	// CommandRegister
				int							videoStreamIndex;	// CommandRegister
				double						maxBufferLength;	// CommandRegister, 패킷 크기가 이 시간을 넘어서면 버퍼링 중지.
				DKObject<DKAVMediaStream>	stream;				// CommandRegister,
				double						position;
				bool						seekable;
				bool						requestSync;		// true 면 시간 다시 맞춤
			};
			double PacketTime(AVFormatContext* ctxt, AVPacket& pkt, double defaultValue)
			{
				if (pkt.pts != AV_NOPTS_VALUE)
				{
					double timeBase = av_q2d(ctxt->streams[pkt.stream_index]->time_base);
					int64_t startPTS = ctxt->streams[pkt.stream_index]->start_time != AV_NOPTS_VALUE ?
						ctxt->streams[pkt.stream_index]->start_time : 0;
					return static_cast<double>(pkt.pts - startPTS) * timeBase;
				}
				else if (pkt.dts != AV_NOPTS_VALUE)
				{
					double timeBase = av_q2d(ctxt->streams[pkt.stream_index]->time_base);
					int64_t startPTS = ctxt->streams[pkt.stream_index]->start_time != AV_NOPTS_VALUE ?
						ctxt->streams[pkt.stream_index]->start_time : 0;
					return static_cast<double>(pkt.dts - startPTS) * timeBase;
				}
				return defaultValue;
			}
			bool SeekStream(AVFormatContext* ctxt, double pos)
			{
				// 버퍼에 있는것 먼저 비교함 (seek 를 수행하면 EOF 가 되어버리는 미디어가 있다. 예:swf)
				// AVFormatContext 는 opaque 형식으로 될거라 추후엔 이 방법을 사용할수 없게 될것임.
				for (AVPacketList* pkt = ctxt->packet_buffer; pkt != NULL; pkt = pkt->next)
				{
					AVPacket& packet = pkt->pkt;

					double ts = PacketTime(ctxt, packet, -1);
					if (ts >= 0.0)
					{
						if (fabs(pos - ts) < 0.5)
							return true;
					}
				}

				int64_t seekTo = static_cast<int64_t>(pos * AV_TIME_BASE); // + ctxt->start_time;
				int64_t seekMin = seekTo;
				int64_t seekMax = seekTo;
				if (avformat_seek_file(ctxt, -1, seekMin, seekTo, seekMax, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD) < 0)
				{
					DKLog("[DKAVMediaBuffer] Error: avformat_seek_file(%f) failed.\n", pos);
					// 특정 포맷에서 0 프레임으로 seek 하는게 실패할 경우가 있다. 그럴땐 flags 를 0 으로 다시 검색함.
					if (av_seek_frame(ctxt, -1, static_cast<int64_t>(pos * AV_TIME_BASE), 0) < 0)
					{
						DKLog("[DKAVMediaBuffer] Error: av_seek_frame(%f) failed.\n", pos);
						return false;
					}
					return true;
				}
				return true;
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private;

DKAVMediaBuffer::DKAVMediaBuffer(void)
{
	bufferingThread = DKThread::Create(DKFunction(this, &DKAVMediaBuffer::BufferingProc)->Invocation());

	DKASSERT_DEBUG(bufferingThread != NULL);
}

DKAVMediaBuffer::~DKAVMediaBuffer(void)
{
	if (bufferingThread->IsAlive())
	{
		BufferCommand bc = {BufferCommand::CommandQuit};
		bufferCommandQueue.PostMessage(bc);
		bufferingThread->WaitTerminate();
	}
}

void DKAVMediaBuffer::BufferingProc(void)
{
	//	DKThread::ThreadId currentThreadId = DKThread::CurrentThreadId();

	DKArray<StreamBufferInfo> bufferList;

	DKTimer timer;
	float updateInterval = 0.001;

	DKLog("DKAVMediaBuffer:%x Initialized.\n", this);

	while ( 1 )
	{
		DKObject<MessageQueue::Receiver> recp = NULL;

		if (bufferList.IsEmpty())		// 버퍼링 상태가 아니면 그냥 메시지를 기다린다.
			recp= this->bufferCommandQueue.GetMessage(true);
		else
			recp = this->bufferCommandQueue.GetMessageTimeOut(updateInterval);
		if (recp)
		{
			BufferCommand& mesg = recp->Content();
			if (mesg.command == BufferCommand::CommandQuit)
			{
				recp->SetResult(0);
				break;		// 쓰레드 종료!
			}
			////////////////////////////////////////////////////////////////////////////////
			// 그외 메시지들
			bool result = false;
			switch (mesg.command)
			{
			case BufferCommand::CommandNoop:		// 아무일도 안함. (응답확인용)
				result = true;
				break;
			case BufferCommand::CommandRegister:
				if (recp->Content().context)
				{
					// 이미 있으면 찾아서 제거
					for (int i = 0; i < bufferList.Count(); i++)
					{
						if (bufferList.Value(i).context == recp->Content().context)
						{
							bufferList.Remove(i);
							break;
						}
					}

					StreamBufferInfo buffer = 
					{
						recp->Content().context,
						recp->Content().audioStreamIndex,
						recp->Content().videoStreamIndex,
						recp->Content().maxBufferLength,
						recp->Content().stream,
						Max<double>(recp->Content().position, 0),
						recp->Content().seekable,
						true
					};

					AVFormatContext* avContext = reinterpret_cast<AVFormatContext*>(buffer.context->avContext);

					if (buffer.seekable)
					{
						if (SeekStream(avContext, buffer.position) == false)
						{
							DKLog("DKAVMediaBuffer:%x Context:%x Seek:%f failed.\n", this, buffer.context, buffer.position);
						}
					}

					bufferList.Add(buffer);
					//DKLog("DKAVMediaBuffer:%x Context:%x Registered. (maxBuffer:%.3f)\n", this, buffer.context, buffer.maxBufferLength);
					//DKLog("DKAVMediaBuffer:%x has %d Media Source Objects.\n", this, bufferList.Count());
					buffer.context->OnBufferStateChanged(DKAVMediaPlayer::MediaState::BufferStateFeeding);
					result = true;
				}
				break;
			case BufferCommand::CommandUnregister:
			case BufferCommand::CommandUnregisterEOF:
			case BufferCommand::CommandUnregisterError:
				if (recp->Content().context)
				{
					for (int i = 0; i < bufferList.Count(); i++)
					{
						StreamBufferInfo& buffer = bufferList.Value(i);
						if (buffer.context == recp->Content().context)
						{
							bufferList.Remove(i);
							result = true;

							if (mesg.command == BufferCommand::CommandUnregisterEOF)
								DKLog("DKAVMediaBuffer:%x Context:%x Unregistered. (with EOF)\n", this, recp->Content().context);
							else if (mesg.command == BufferCommand::CommandUnregisterError)
								DKLog("DKAVMediaBuffer:%x Context:%x Unregistered. (with Error!)\n", this, recp->Content().context);
							else
								DKLog("DKAVMediaBuffer:%x Context:%x Unregistered.\n", this, recp->Content().context);

							//DKLog("DKAVMediaBuffer:%x has %d Media Source Objects.\n", this, bufferList.Count());
							buffer.context->OnBufferStateChanged(DKAVMediaPlayer::MediaState::BufferStateStopped);
							break;
						}
					}
				}
				break;
			case BufferCommand::CommandResize:
				if (recp->Content().context)
				{
					for (int i = 0; i < bufferList.Count(); i++)
					{
						StreamBufferInfo& buffer = bufferList.Value(i);
						if (buffer.context == recp->Content().context)
						{
							buffer.maxBufferLength = Max<double>(recp->Content().maxBufferLength, 1);
							result = true;

							//DKLog("DKAVMediaBuffer:%x Context:%x MaxBufferLength:%.3f.\n", this, recp->Content().context, buffer.maxBufferLength);
							buffer.context->OnBufferLengthResized(buffer.maxBufferLength);
							break;
						}
					}
				}
				break;
			}
			// 모든 메시지에 응답
			recp->SetResult(result);
		}
		else		// 버퍼링
		{
			for (int i = 0; i < bufferList.Count(); i++)
			{
				StreamBufferInfo& buffer = bufferList.Value(i);

				AVFormatContext* avContext = (AVFormatContext*)buffer.context->avContext;

				double renderTime = buffer.stream->GetRenderTime();
				double delayToSeek = Max<double>(buffer.maxBufferLength, 5.0);
				double delay = renderTime - (buffer.position + delayToSeek);

				bool requireBuffer = (buffer.position - renderTime < (buffer.maxBufferLength + 0.1));
				bool requireSeek = (buffer.seekable && delay > 0.1) && buffer.requestSync == false;

				if (requireSeek)
				{
					DKLog("[DKAVMediaBuffer] feeding delayed by %f sec, request seek!\n", delay);
					// 위치 재조정..
					if (SeekStream(avContext, renderTime) == false)
					{
						DKLog("DKAVMediaBuffer:%x Context:%x Seek:%f failed.\n", this, buffer.context, buffer.position);
					}
					buffer.requestSync = true;
				}
				else if (buffer.requestSync || requireBuffer)
				{
					AVPacket packet;
					int ret = av_read_frame(avContext, &packet);
					if (ret < 0)
					{
						//if (url_ferror(avContext->pb))
						if (avContext->pb->error)
						{
							// 에러!
							BufferCommand bc = {DKAVMediaBuffer::BufferCommand::CommandUnregisterEOF, buffer.context};
							this->bufferCommandQueue.PostMessage(bc);
						}
						else	// 끝까지 읽었다.
						{
							// 종료 패킷 넣음.
							double duration = (double)(avContext->duration) / AV_TIME_BASE;
							if (buffer.audioStreamIndex >= 0 && avContext->streams[buffer.audioStreamIndex]->duration != AV_NOPTS_VALUE)
								duration = Max(duration, (double)avContext->streams[buffer.audioStreamIndex]->duration * av_q2d(avContext->streams[buffer.audioStreamIndex]->time_base));
							if (buffer.videoStreamIndex >= 0 && avContext->streams[buffer.videoStreamIndex]->duration != AV_NOPTS_VALUE)
								duration = Max(duration, (double)avContext->streams[buffer.videoStreamIndex]->duration * av_q2d(avContext->streams[buffer.videoStreamIndex]->time_base));

							double audioTime = buffer.stream->GetAudioBufferTime();
							double videoTime = buffer.stream->GetVideoBufferTime();

							buffer.stream->SetAudioBufferTime(Max<double>(audioTime, duration));
							buffer.stream->SetVideoBufferTime(Max<double>(videoTime, duration));
							buffer.stream->SetEOF();

							// 큐에서 제거
							DKLog("DKAVMediaBuffer:%x Context:%x EOF reached.\n", this, buffer.context);
							BufferCommand bc = {DKAVMediaBuffer::BufferCommand::CommandUnregisterEOF, buffer.context};
							this->bufferCommandQueue.PostMessage(bc);
						}
					}
					else
					{
						bool dropPacket = true;

						if (packet.stream_index == buffer.audioStreamIndex || packet.stream_index == buffer.videoStreamIndex)
						{
							double ts = PacketTime(avContext, packet, -1.0);
							
							if (buffer.requestSync)
							{
								if (ts >= 0.0)
								{
									buffer.position = ts;
									buffer.stream->Reset();
									buffer.stream->SetSyncRequest(buffer.position);
									buffer.stream->SetFlush();
									buffer.requestSync = false;
								}
								renderTime = ts;
							}

							double validTime = renderTime - 0.01;	// 이 시간보다 오래된건 그냥 버림

							if (packet.stream_index == buffer.audioStreamIndex)
							{
								double ts = PacketTime(avContext, packet, buffer.stream->GetAudioBufferTime());
								if (ts > validTime)		// 유효한 패킷
								{
									dropPacket = false;
									buffer.stream->PushAudioPacket(packet);
								}
								// 시간 갱신
								buffer.stream->SetAudioBufferTime(ts);
								buffer.context->OnBufferAudio(ts);
								buffer.position = Max<double>(buffer.position, ts);
							}
							else if (packet.stream_index == buffer.videoStreamIndex)
							{
								double ts = PacketTime(avContext, packet, buffer.stream->GetVideoBufferTime());
								if (ts > validTime)		// 유효한 패킷
								{
									dropPacket = false;
									buffer.stream->PushVideoPacket(packet);
								}
								// 시간 갱신
								buffer.stream->SetVideoBufferTime(ts);
								buffer.context->OnBufferVideo(ts);
								buffer.position = Max<double>(buffer.position, ts);
							}
						}
						if (dropPacket)
							av_free_packet(&packet);
					}
				}
			}
			DKThread::Yield();
		}
	}
	DKLog("DKAVMediaBuffer:%x Terminated.\n", this);
}

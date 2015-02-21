#include "FFmpeg/FFmpeg.h"
#include "DKAVMediaStream.h"

using namespace DKFoundation;
using namespace DKFramework;

AVPacket DKAVMediaStream::flushPacket = DKAVMediaStream::GetDefaultPacket("FLUSH");
AVPacket DKAVMediaStream::eofPacket = DKAVMediaStream::GetDefaultPacket("EOF");

AVPacket DKAVMediaStream::GetDefaultPacket(const char* name)
{
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = (uint8_t*)name;
	pkt.size = 0;
	return pkt;
}

DKAVMediaStream::DKAVMediaStream(void)
	: renderTime(0)
	, audioBufferTime(0)
	, videoBufferTime(0)
	, syncRequest(false)
	, streamEnd(false)
{
}

DKAVMediaStream::~DKAVMediaStream(void)
{
	Reset();
}

bool DKAVMediaStream::PopAudioPacket(AVPacket& pkt)
{
	CriticalSection section(lock);

	if (audioQueue.PopFront(pkt))
		return true;
	return false;
}

bool DKAVMediaStream::PopVideoPacket(AVPacket& pkt)
{
	CriticalSection section(lock);

	if (videoQueue.PopFront(pkt))
		return true;
	return false;
}

void DKAVMediaStream::PushAudioPacket(AVPacket& pkt)
{
	CriticalSection section(lock);

	if (av_dup_packet(&pkt) >= 0)
		audioQueue.PushBack(pkt);
}

void DKAVMediaStream::PushVideoPacket(AVPacket& pkt)
{
	CriticalSection section(lock);

	if (av_dup_packet(&pkt) >= 0)
		videoQueue.PushBack(pkt);
}

void DKAVMediaStream::SetEOF(void)
{
	CriticalSection section(lock);
	audioQueue.PushBack(eofPacket);
	videoQueue.PushBack(eofPacket);
	streamEnd = true;
}

void DKAVMediaStream::SetFlush(void)
{
	CriticalSection section(lock);
	audioQueue.PushBack(flushPacket);
	videoQueue.PushBack(flushPacket);
}

void DKAVMediaStream::Reset(void)
{
	CriticalSection section(lock);

	for (size_t i = 0; i < audioQueue.Count(); i++)
	{
		AVPacket& pkt = audioQueue[i];
		if (pkt.data != flushPacket.data && pkt.data != eofPacket.data)
			av_free_packet(&pkt);
	}
	for (size_t i = 0; i < videoQueue.Count(); i++)
	{
		AVPacket& pkt = videoQueue[i];
		if (pkt.data != flushPacket.data && pkt.data != eofPacket.data)
			av_free_packet(&pkt);
	}
	audioQueue.Clear();
	videoQueue.Clear();

	renderTime = 0;
	audioBufferTime = 0;
	videoBufferTime = 0;
	syncRequest = false;
	streamEnd = false;
}

size_t DKAVMediaStream::GetAudioPacketCount(void) const
{
	return audioQueue.Count();
}

size_t DKAVMediaStream::GetVideoPacketCount(void) const
{
	return videoQueue.Count();
}

void DKAVMediaStream::SetRenderTime(double t)
{
	CriticalSection section(lock);

	if (!syncRequest)
		renderTime = t;
}

void DKAVMediaStream::SetAudioBufferTime(double t)
{
	audioBufferTime = t;
}

void DKAVMediaStream::SetVideoBufferTime(double t)
{
	videoBufferTime = t;
}

double DKAVMediaStream::GetRenderTime(void) const
{
	return renderTime;
}

double DKAVMediaStream::GetAudioBufferTime(void) const
{
	return audioBufferTime;
}

double DKAVMediaStream::GetVideoBufferTime(void) const
{
	return videoBufferTime;
}

void DKAVMediaStream::SetSyncRequest(double t)
{
	CriticalSection section(lock);

	syncRequest = true;
	renderTime = t;
}

bool DKAVMediaStream::GetSyncRequest(double& t)
{
	CriticalSection section(lock);

	if (syncRequest)
	{
		t = renderTime;
		syncRequest = false;
		return true;
	}
	return false;
}

bool DKAVMediaStream::IsEof(AVPacket& pkt)
{
	return pkt.data == eofPacket.data;
}

bool DKAVMediaStream::IsFlush(AVPacket& pkt)
{
	return pkt.data == flushPacket.data;
}

bool DKAVMediaStream::HasEOF(void) const
{
	return streamEnd;
}

#pragma once

#include <DK.h>

struct AVPacket;

namespace DKFramework
{
	class DKAVMediaStream
	{
	public:
		typedef DKFoundation::DKSpinLock Lock;
		typedef DKFoundation::DKCriticalSection<Lock> CriticalSection;

		DKAVMediaStream(void);
		virtual ~DKAVMediaStream(void);

		size_t GetAudioPacketCount(void) const;
		size_t GetVideoPacketCount(void) const;

		//void RemoveAllAudioPackets(void);
		//void RemoveAllVideoPackets(void);

		bool PopAudioPacket(AVPacket& pkt);
		bool PopVideoPacket(AVPacket& pkt);
		void PushAudioPacket(AVPacket& pkt);
		void PushVideoPacket(AVPacket& pkt);

		void Reset(void);

		void SetEOF(void);
		void SetFlush(void);

		void SetRenderTime(double t);
		void SetAudioBufferTime(double t);
		void SetVideoBufferTime(double t);

		double GetRenderTime(void) const;
		double GetAudioBufferTime(void) const;
		double GetVideoBufferTime(void) const;

		void SetSyncRequest(double t);
		bool GetSyncRequest(double& t);

		static bool IsEof(AVPacket& pkt);
		static bool IsFlush(AVPacket& pkt);

		bool HasEOF(void) const;

	private:
		static AVPacket GetDefaultPacket(const char* name);

		Lock				lock;
		DKFoundation::DKQueue<AVPacket>		audioQueue;
		DKFoundation::DKQueue<AVPacket>		videoQueue;
		double				renderTime;
		double				audioBufferTime;
		double				videoBufferTime;
		bool				syncRequest;
		bool				streamEnd;

		static AVPacket	flushPacket;
		static AVPacket	eofPacket;
	};	
}

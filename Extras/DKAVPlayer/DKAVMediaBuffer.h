#pragma once
#include <DK.h>

////////////////////////////////////////////////////////////////////////////////
//
// DKAVMediaBuffer
//
// 미디어 버퍼링을 해주는 객체.
// DKAVMediaPlayer 객체들은 이 객체를 공유할 수 있다.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKAVMediaStream;
	class DKAVMediaPlayer;
	class DKAVMediaBuffer
	{
		friend class DKAVMediaPlayer;
	public:
		DKAVMediaBuffer(void);
		virtual ~DKAVMediaBuffer(void);

	private:
		struct BufferCommand
		{
			enum Command
			{
				CommandRegister = 0,	// 스트림 설정 스트림을 바꿀때도 사용됨, 스트림이 바뀌면 기존 버퍼링 된거 모두 제거됨.
				CommandUnregister,		// 버퍼링 종료
				CommandUnregisterEOF,	// 파일 끝까지 읽어서 종료됨 (내부에서 사용함)
				CommandUnregisterError,	// 에러로 인하여 종료됨 (내부에서 사용함)
				CommandResize,			// 버퍼링 크기 변경
				CommandNoop,			// 응답 확인
				CommandQuit,			// 쓰레드 종료
			};
			Command												command;
			DKAVMediaPlayer*									context;
			double												maxBufferLength;	// kCmdRegister, kCmdResize 패킷 크기가 이 시간을 넘어서면 버퍼링 중지.
			double												position;			// kCmdRegister
			int													audioStreamIndex;	// kCmdRegister
			int													videoStreamIndex;	// kCmdRegister
			DKFoundation::DKObject<DKAVMediaStream>				stream;				// kCmdRegister,
			bool												seekable;
		};
		typedef DKFoundation::DKMessageQueue<bool, BufferCommand> MessageQueue;
		MessageQueue bufferCommandQueue;			// 쓰레드와 메시지를 주고받기 위한 큐
		void BufferingProc(void);
		DKFoundation::DKObject<DKFoundation::DKThread> bufferingThread;
	};
}

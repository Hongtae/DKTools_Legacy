#include "StdAfx.h"
#include "TestAnimator.h"

TestAnimator::TestAnimator(void)
	: animation(NULL)
	, repeat(0)
	, frameTime(0)
	, playing(false)
{
}

TestAnimator::~TestAnimator(void)
{
}

bool TestAnimator::GetTransform(const NodeId& nid, DKTransformUnit& out)
{
	if (animation)
	{
		return animation->GetNodeTransform(nid, frameTime, out);
	}
	return false;
}

void TestAnimator::UpdateFrame(float frame)
{
	bool stopAnimation = false;
	if (playing && animation)
	{
		frameTime = 0;		// 0~1 사이의 상대 시간값
		float duration = Max<float>(animation->Duration(), 0.0);
		if (duration > 0)
		{
			if (speed > 0)
			{
				frameTime = frame / duration;
				int c = floor(frameTime);
				if (c > repeat)		// 반복 재생 횟수 초과
				{
					stopAnimation = true;
					frameTime = 1.0;
					this->frame = duration * repeat;
				}
				else
				{
					frameTime = frameTime - c;
				}
			}
			else
			{
				float ftime = frame;
				float t = (duration - frame) / duration;
				if (t > 1.0)
				{
					int c = floor(t);
					if (c > repeat)
					{
						stopAnimation = true;
						ftime = 0.0f;
						this->frame = 0;
					}
					else
					{
						ftime += duration * c;
					}
				}
				frameTime = ftime / duration;
			}
		}
		else		// duration 이 잘못됐음.
		{
			stopAnimation = true;
			if (speed < 0)
				frameTime = 0.0;
			else
				frameTime = 1.0;
		}
#if 0
		/////
		if (stopAnimation)
		{
			DKLog("Animation stopped:(repeat:%d, speed:%.3f, dur:%.3f, frame:%.3f, time:%.3f)\n", repeat, speed, duration, frame, frameTime * duration);
		}
		else
		{
			DKLog("Animation status:(repeat:%d, speed:%.3f, dur:%.3f, frame:%.3f, time:%.3f)\n", repeat, speed, duration, frame, frameTime * duration);
		}
		////
#endif
	}

	if (stopAnimation)
	{
		Stop();
	}
}

void TestAnimator::SetAnimation(DKAnimation* ani)
{
	if (animation != ani)
	{
		Stop();		// 기존 애니메이션을 중지시킴 (중지 이벤트 발송)

		this->animation = ani;

		SetFrame(0);
	}
}

DKAnimation* TestAnimator::Animation(void)
{
	return animation;
}

const DKAnimation* TestAnimator::Animation(void) const
{
	return animation;
}

void TestAnimator::Play(void)
{
	if (playing == false)
	{
		if (animation)			// 애니메이션이 있다면 재생 이벤트 발송
		{
			playing = true;
			frameTime = Clamp<float>(frameTime, 0.0f, 1.0f);
			SetFrame(frameTime * Duration());
		}
	}
}

void TestAnimator::Stop(void)
{
	if (playing)
	{
		playing = false;

		if (animation)
		{
			frameTime = Clamp<float>(frameTime, 0.0f, 1.0f);
		}
	}
}

void TestAnimator::SetRepeat(unsigned int r)
{
	repeat = r;
}

bool TestAnimator::IsPlaying(void) const
{
	return playing && animation != NULL;
}

float TestAnimator::TimePosition(void) const
{
	return frameTime * Duration();
}

float TestAnimator::Duration(void) const
{
	if (animation)
		return Max<float>(animation->Duration(), 0.0);
	return 0;
}

float TestAnimator::TimeRemains(void) const
{
	if (animation)
	{
		float totalDuration = animation->Duration() * (repeat + 1);

		if (totalDuration > 0)
		{
			return totalDuration - TimePosition();
		}
	}
	return 0;
}

unsigned int TestAnimator::Repeat(void) const
{
	return repeat;
}

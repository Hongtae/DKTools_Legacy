#include "StdAfx.h"
#include "Equalizer.h"
#include <memory.h>

Equalizer::Equalizer(void)
	: maxLevelValue(0)
{
	for (int i = 0; i < 10; i++)
	{
		leftLevels[i] = 0;
		rightLevels[i] = 0;
	}
}

Equalizer::~Equalizer(void)
{
	Unload();
}

void Equalizer::OnLoaded(void)
{
	this->SetBlendState(DKBlendState::defaultAlpha);
}

void Equalizer::OnUnload(void)
{
	audioFilter.audioSource = NULL;
	audioSource = NULL;
}

void Equalizer::OnMouseDown(int deviceId, int nButtonId, const DKPoint& pos) 
{

}

void Equalizer::OnMouseUp(int deviceId, int nButtonId, const DKPoint& pos)
{

}

void Equalizer::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)  
{

}

void Equalizer::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
}

void Equalizer::OnMouseHover(int deviceId) 
{
}

void Equalizer::OnMouseLeave(int deviceId) 
{
}

void Equalizer::OnUpdate(double timeDelta, DKTimeTick tick, const DKDateTime& timeCurrent) 
{
	if (audioSource)
	{

		if (audioSource->State() == DKAudioPlayer::AudioState::StatePlaying)
		{
			memcpy(leftLevels, audioFilter.leftLevels, sizeof(leftLevels));
			memcpy(rightLevels, audioFilter.rightLevels, sizeof(rightLevels));
			maxLevelValue = audioFilter.maxLevelValue;
		}
		else
		{
			for (int i = 0; i < 10; i++)
			{
				leftLevels[i] -= timeDelta * 180;
				if (leftLevels[i] < 0) leftLevels[i] = 0;
				rightLevels[i] -= timeDelta * 180;
				if (rightLevels[i] < 0) rightLevels[i] = 0;
			}
		}

		SetRedraw();
	}
}

void Equalizer::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(0.2, 0.2, 0.2, 0.5));

	if (maxLevelValue > 0 && audioSource)
	{
		if (audioSource->Channels() == 1)
		{
			DKRect levelRect = DKRect(0.02, 0.05, 0.96, 0.9);
			renderer.RenderSolidRect(levelRect, DKMatrix3::identity, DKColor(0,0,0,1.0));

			float graphWidth = levelRect.size.width / 10.0;

			for (int i = 0; i < 10; i++)
			{
				renderer.RenderSolidRect(DKRect(levelRect.origin.x + graphWidth * i, levelRect.origin.y, graphWidth, levelRect.size.height * Clamp<float>(leftLevels[i] / maxLevelValue, 0.0, 1.0)),
					DKMatrix3::identity, DKColor(1.0f, 1.0f - (0.1f * i), 0.1f, 1.0f));
			}
		}
		else
		{
			DKRect leftLevelRect = DKRect(0.02, 0.05, 0.46, 0.9);
			DKRect rightLevelRect = DKRect(0.52, 0.05, 0.46, 0.9);

			renderer.RenderSolidRect(leftLevelRect, DKMatrix3::identity, DKColor(0,0,0,1.0));
			renderer.RenderSolidRect(rightLevelRect, DKMatrix3::identity, DKColor(0,0,0,1.0));

			float leftGraphWidth = leftLevelRect.size.width / 10.0;
			for (int i = 0; i < 10; i++)
			{
				renderer.RenderSolidRect(DKRect(leftLevelRect.origin.x + leftGraphWidth * i, leftLevelRect.origin.y, leftGraphWidth, leftLevelRect.size.height * Clamp<float>(leftLevels[i] / maxLevelValue, 0.0, 1.0)),
					DKMatrix3::identity, DKColor(1.0f, 1.0f - (0.1f * i), 0.1f, 1.0f));
			}
			float rightGraphWidth = rightLevelRect.size.width / 10.0;
			for (int i = 0; i < 10; i++)
			{
				renderer.RenderSolidRect(DKRect(rightLevelRect.origin.x + rightGraphWidth * i, rightLevelRect.origin.y, rightGraphWidth, rightLevelRect.size.height * Clamp<float>(rightLevels[i] / maxLevelValue, 0.0, 1.0)),
					DKMatrix3::identity, DKColor(1.0f, 1.0f - (0.1f * i), 0.1f, 1.0f));
			}	
		}
	}
}

void Equalizer::OnKeyDown(int deviceId, DKVirtualKey lKey)
{
}

void Equalizer::OnKeyUp(int deviceId, DKVirtualKey lKey)  
{
}

void Equalizer::OnTextInput(int deviceId, const DKString& str) 
{
}

void Equalizer::OnTextInputCandidate(int deviceId, const DKString& str)
{
}

void Equalizer::Open(const DKString& file)
{
	audioSource = DKAudioPlayer::Create(file);
	if (audioSource)
	{
		audioSource->SetStreamFilter(DKFunction(&audioFilter, &TestAudioFilter::OnProcessStream));
		audioSource->SetBufferingTime(0.0);	// 최소값 설정
	}

	audioFilter.audioSource = audioSource;
}

void Equalizer::Play(void)
{
	if (audioSource)
		audioSource->Play();
}

void Equalizer::Stop(void)
{
	if (audioSource)
		audioSource->Stop();
}

void Equalizer::Pause(void)
{
	if (audioSource)
		audioSource->Pause();
}

DKAudioPlayer::AudioState Equalizer::State(void) const
{
	if (audioSource)
		return audioSource->State();
	return DKAudioPlayer::AudioState::StateUnknown;
}

void Equalizer::SetPosition(float x, float y, float z)
{
	if (audioSource)
		audioSource->AudioSource()->SetPosition(DKVector3(x,y,z));
}

#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class TestAnimator : public DKAnimationController
{
public:
	TestAnimator(void);
	~TestAnimator(void);

	void UpdateFrame(float frame) override;
	bool GetTransform(const NodeId& nid, DKTransformUnit& out) override;

	// 애니메이션 설정 (animation 이 NULL 이면 리셋)
	// 애니메이션을 새로 설정하면 기존 애니메이션은 중지하게 된다. 새로 설정 후 PlayAnimation()을 호출해야 재생 시작함.
	// rescale: true 이면 애니메이션의 뼈대정보의 기본 행렬(local-TM)을 사용한다.
	void SetAnimation(DKAnimation* ani);
	DKAnimation* Animation(void);
	const DKAnimation* Animation(void) const;

	// 애니메이션 제어
	void Play(void) override;				// 애니메이션 재생을 시작함
	void Stop(void) override;				// 애니메이션 재생을 중지함

	void SetRepeat(unsigned int repeat);	// 반복 횟수 설정

	bool IsPlaying(void) const override;
	float TimePosition(void) const;			// 현재 애니메이션 시간 (0 ~ duration)
	float TimeRemains(void) const;
	float Duration(void) const override;				// 애니메이션 길이

	unsigned int Repeat(void) const;

private:
	DKFoundation::DKObject<DKAnimation> animation;

	// 재생 관련
	float			frameTime;
	unsigned int	repeat;
	bool			playing;
};

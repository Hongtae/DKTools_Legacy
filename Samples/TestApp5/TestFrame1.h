#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class TestFrame1 : public DKFrame
{
public:
	TestFrame1(void);
	~TestFrame1(void);

	void OnLoaded(void);
	void OnUnload(void);
	void OnContentResized(void);

	void OnMouseDown(int deviceId, int buttonId, const DKPoint& pos);				// 마우스 버튼이 눌려짐
	void OnMouseUp(int deviceId, int buttonId, const DKPoint& pos);					// 마우스 버튼이 올라감
	void OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta);		// 마우스가 이동할때 호출됨.
	void OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta);	// 마우스 휠이 움직임
	void OnMouseHover(int deviceId);												// 마우스가 프레임 영역에 들어옴
	void OnMouseLeave(int deviceId);												// 마우스가 프레임 영역에서 벗어남

	void OnUpdate(double timeDelta, DKTimeTick tick, const DKDateTime& timeCurrent);
	void OnRender(DKRenderer& renderer) const;

	void OnKeyDown(int deviceId, DKVirtualKey key);					// 키가 눌려짐
	void OnKeyUp(int deviceId, DKVirtualKey key);					// 키가 올라감
	void OnTextInput(int deviceId, const DKString& str);			// 글자가 입력됨.
	void OnTextInputCandidate(int deviceId, const DKString& str);	// 글자 입력중

private:
	void RemovePickingConstraint(void);

public:
	double					timeDelta;
	DKDateTime				timeCurrent;
	DKTimeTick				updateTick;

	DKObject<DKFont>		fontRegular;

	DKCamera				camera;
	DKVector3				cameraTarget;
	
	DKObject<DKScene>		scene;
	DKMap<int, DKPoint>		mousePositions;

	DKObject<DKGeneric6DofConstraint>	pickConstraints;
	DKCollisionObject*		pickedObject;
	float					pickDistance;

	DKObject<DKFrame>		displayButton;
};

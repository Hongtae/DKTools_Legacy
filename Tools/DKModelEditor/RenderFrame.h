#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class RenderFrame : public DKFrame
{
public:
	RenderFrame(void);
	~RenderFrame(void);

protected:
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
	
	void OnKeyDown(int deviceId, DKVirtualKey key);									// 키가 눌려짐
	void OnKeyUp(int deviceId, DKVirtualKey key);									// 키가 올라감
	void OnTextInput(int deviceId, const DKFoundation::DKString& str);				// 글자가 입력됨.
	void OnTextInputCandidate(int deviceId, const DKFoundation::DKString& str);		// 글자 입력중

public:
	void SetModel(DKModel* model);
	void Lock(void) const;
	void Unlock(void) const;
private:
	static DKObject<DKStaticMesh> CreateGridLinesModel(int div, DKMaterial* material);
	static DKObject<DKStaticMesh> CreateBoundSphereModel(int div, DKMaterial* material);
	static DKObject<DKStaticMesh> CreateBoxModel(DKMaterial* material);
	static DKObject<DKStaticMesh> CreateWireframeModel(const DKArray<DKLine>& lines, DKMaterial* material);
	static DKObject<DKStaticMesh> CreateWireframeModel(const DKArray<DKVector3>& points, const DKArray<unsigned short>& indices, DKMaterial* material);

	// 정보 찍기
	bool		mouseLButtonDown;
	bool		mouseRButtonDown;
	bool		mouseWButtonDown;
	DKPoint		mousePos;
	double		timeElapsed;
	DKDateTime	timeCurrent;	
	
	DKObject<DKFont>		defaultFont;
	DKObject<DKMaterial>	defaultMaterial;
	DKObject<DKModel>		targetObject;

	DKObject<DKScene>		modelScene;

	DKCamera	camera;
	DKVector3	cameraTarget;
	float		cameraNear;
	float		cameraFar;

	DKSpinLock			lockRenderer;
};

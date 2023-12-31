#pragma once
#include "Module.h"
#include <Math/float2.h>
#include <Math/float3.h>
#include <Math/float4x4.h>
#include <Geometry/Frustum.h>

class Model;

class ModuleCamera :
    public Module
{
public:
	ModuleCamera();
	~ModuleCamera();

	bool Init();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();
	void SetFOV(float horizontalFOV);
	void SetAspectRatio(float aspectRatio);
	void SetPlaneDistances(float nearPlane, float farPlane);
	void SetPosition(float x, float y, float z);
	//void SetOrientation();
	void LookAt(float3 target_pos);
	void ManageKeyboardInput();
	void ManageMouseInput();
	void CameraRotation();
	void CameraPan();
	void CameraZoom();
	void CameraOrbit(const Model& model);
	void FocusGeometry(const Model& model);
	const float3* GetPosition() const { return &frustum->pos; };
	const float4x4& GetProjectionMatrix();
	const float4x4& GetViewMatrix();

	inline const float GetCameraSpeed() const { return cameraSpeed; }
	inline const float2 GetMouseSensitivity() const { return mouseSensitivity; }
	inline const float GetZoomSensitivity() const { return zoomSensitivity; }
	inline const float2 GetPanSensitivity() const { return panSensitivity; }

	inline void SetCameraSpeed(float newCameraSpeed) { cameraSpeed = newCameraSpeed; }
	inline void SetMouseSensitivity(const float2 &newMouseSensitivity) { mouseSensitivity = newMouseSensitivity; }
	inline void SetZoomSensitivity(float newZoomSensitivity) { zoomSensitivity = newZoomSensitivity; }
	inline void SetPanSensitivity(float2 newPanSensitivity) { panSensitivity = newPanSensitivity; }

private:
	float cameraSpeed = 0;
	float deltaTime = 0, lastFrame = 0;
	float yaw = 0, pitch = 0;
	float verticalPan = 0, horizontalPan = 0;
	float2 mouseSensitivity = float2::zero, panSensitivity = float2::zero;
	float zoomSensitivity = 0;
	Frustum* frustum = nullptr;
	float4x4 camera_matrix = float4x4::zero;
};


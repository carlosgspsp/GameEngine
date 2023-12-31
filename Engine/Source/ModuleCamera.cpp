#include "ModuleCamera.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRenderExercise.h"
#include "SDL.h"
#include "Math/MathFunc.h"
#include "Geometry/AABB.h"
#include "Model.h"
#include "Geometry/Sphere.h"


ModuleCamera::ModuleCamera()
{
	frustum = new Frustum;
}

ModuleCamera::~ModuleCamera()
{
	delete frustum;
}


bool ModuleCamera::Init()
{

	float2 screenSize = App->GetWindow()->GetScreenSize();

	frustum->type = FrustumType::PerspectiveFrustum;
	frustum->pos = float3(0.0f, 0.25f, 0.5f);
	frustum->nearPlaneDistance = 0.01f;
	frustum->farPlaneDistance = 1000.0f;
	frustum->verticalFov = math::pi / 4.0f;
	frustum->horizontalFov = 2.0f * atanf(tanf(frustum->verticalFov * 0.5f) * screenSize.x / screenSize.y);
	float angle = RadToDeg(frustum->horizontalFov);

	LookAt(float3(0.0f, 0.0f, 0.0f));

	angle = RadToDeg(frustum->verticalFov);

	deltaTime = 0.0f;
	lastFrame = 0.0f;

	mouseSensitivity = float2(0.1f, 0.1f);
	panSensitivity = float2(0.005f, 0.005f);
	zoomSensitivity = 1.4f;
	yaw = -90.0f;
	pitch = -RadToDeg(float3(0.0, 0.0, 1.0).AngleBetween(frustum->pos));
	cameraSpeed = 0.1f;
	verticalPan = 0.0f;
	horizontalPan = 0.0f;

	return true;
}


const float4x4& ModuleCamera::GetProjectionMatrix() {
	float4x4 proj_matrix = frustum->ProjectionMatrix();
	return proj_matrix;
}

const float4x4& ModuleCamera::GetViewMatrix() {
	float4x4 view_matrix = frustum->ViewMatrix();
	return view_matrix;
}

void ModuleCamera::LookAt(float3 target_pos) {

	float3 forward = (target_pos - frustum->pos).Normalized();
	float3 right = Cross(forward, float3::unitY).Normalized();
	float3 up = Cross(right, forward).Normalized();

	frustum->front = forward;
	frustum->up = up;

	camera_matrix = float4x4(right[0], up[0], -forward[0], frustum->pos[0], right[1], up[1], -forward[1], frustum->pos[1], right[2], up[2], -forward[2], frustum->pos[2], 0, 0, 0, 1);

	yaw = 90.0f;
	pitch = 0.0f;
}


void ModuleCamera::SetFOV(float horizontalFOV) {


	float2 screenSize = App->GetWindow()->GetScreenSize();


	float aspect_normal = screenSize.x / screenSize.y;
	frustum->horizontalFov = DegToRad(horizontalFOV);

	frustum->verticalFov = 2.0f * atanf(tanf(frustum->horizontalFov * 0.5f) * (1.0f / aspect_normal));

}

void ModuleCamera::SetPlaneDistances(float nearPlane, float farPlane) {
	frustum->nearPlaneDistance = nearPlane;
	frustum->farPlaneDistance = farPlane;
}

void ModuleCamera::SetAspectRatio(float aspectRatio) {
	frustum->verticalFov = 2.0f * atanf(tanf(frustum->horizontalFov * 0.5f)) * (1.0f / aspectRatio);
}

void ModuleCamera::SetPosition(float x, float y, float z) {
	frustum->pos = float3(x, y, z);
}

void ModuleCamera::ManageKeyboardInput() {
	if (App->GetInput()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN) {
		cameraSpeed *= 2.0f;
	}
	if (App->GetInput()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_UP) {
		cameraSpeed /= 2.0f;
	}
	if (App->GetInput()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
		frustum->pos += frustum->front * cameraSpeed * deltaTime;
	}
	if (App->GetInput()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
		frustum->pos -= frustum->front * cameraSpeed * deltaTime;
	}
	if (App->GetInput()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		frustum->pos -= frustum->WorldRight() * cameraSpeed * deltaTime;
	}
	if (App->GetInput()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		frustum->pos += frustum->WorldRight() * cameraSpeed * deltaTime;
	}
	if (App->GetInput()->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT) {
		frustum->pos += float3::unitY * cameraSpeed * deltaTime;
	}
	if (App->GetInput()->GetKey(SDL_SCANCODE_E) == KEY_REPEAT) {
		frustum->pos -= float3::unitY * cameraSpeed * deltaTime;
	}
	if (App->GetInput()->GetKey(SDL_SCANCODE_F) == KEY_DOWN) {
		FocusGeometry(*App->GetModuleRenderExercise()->GetModel());
	}

}

void ModuleCamera::ManageMouseInput() {
	if (App->GetInput()->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT && App->GetInput()->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT) {
		CameraOrbit(*App->GetModuleRenderExercise()->GetModel());
	}
	else if (App->GetInput()->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT) {
		CameraRotation();
	}
	else if (App->GetInput()->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
		CameraPan();
	}
	else if (App->GetInput()->GetMouseWheel().y != 0) {
		CameraZoom();
	}

}
void ModuleCamera::CameraRotation() {

	float2 mousePosition = App->GetInput()->GetMousePosition();
	float2 lastMousePosition = App->GetInput()->GetLastMousePosition();
	float2 offset(lastMousePosition - mousePosition);

	offset.x *= mouseSensitivity.x;
	offset.y *= mouseSensitivity.y;

	yaw = offset.x;
	pitch = offset.y;


	float3 newfront, newup;

	float3x3 rotationX = float3x3::RotateAxisAngle(frustum->WorldRight(), DegToRad(pitch));
	float3x3 rotationY = float3x3::RotateAxisAngle(float3::unitY, DegToRad(yaw));
	float3x3 rotation = rotationY.Mul(rotationX);

	newfront = rotation.MulDir(frustum->front.Normalized());
	newup = rotation.MulDir(frustum->up.Normalized());
	frustum->front = newfront.Normalized();
	frustum->up = newup.Normalized();
	
	}

void ModuleCamera::CameraPan() {
	float2 mousePosition = App->GetInput()->GetMousePosition();
	float2 lastMousePosition = App->GetInput()->GetLastMousePosition();
	float2 offset(lastMousePosition - mousePosition);

	offset.x *= panSensitivity.x;
	offset.y *= panSensitivity.y;

	horizontalPan -= offset.x;
	verticalPan += offset.y;

	frustum->pos += float3::unitY * offset.y;
	frustum->pos += frustum->WorldRight() * offset.x;

}


void ModuleCamera::CameraZoom() {
	float2 mouseWheel = App->GetInput()->GetMouseWheel();
	frustum->pos += frustum->front * zoomSensitivity * mouseWheel.y/10.0f;
}


void ModuleCamera::FocusGeometry(const Model& model) {

	Sphere sphere = model.GetAABB()->MinimalEnclosingSphere();
	float3 center = sphere.Centroid();
	SetPosition(center.x, center.y, center.z-1);
	LookAt(center);
	SetPosition(center.x, center.y, center.z-(sphere.r * 3.5f));

}


void ModuleCamera::CameraOrbit(const Model& model) {
	
	Sphere sphere = model.GetAABB()->MinimalEnclosingSphere();
	float radius = frustum->pos.Distance(sphere.Centroid());
	frustum->pos = sphere.Centroid();

	float2 mousePosition = App->GetInput()->GetMousePosition();
	float2 lastMousePosition = App->GetInput()->GetLastMousePosition();
	float2 offset(lastMousePosition - mousePosition);

	offset.x *= mouseSensitivity.x;
	offset.y *= mouseSensitivity.y;
	yaw = offset.x;
	pitch = offset.y;

	float3 newfront, newup;
	float3x3 rotationX = float3x3::RotateAxisAngle(frustum->WorldRight(), DegToRad(pitch));
	float3x3 rotationY = float3x3::RotateAxisAngle(float3::unitY, DegToRad(yaw));
	float3x3 rotation = rotationY.Mul(rotationX);
	
	newfront = rotation.MulDir(frustum->front.Normalized());
	newup = rotation.MulDir(frustum->up.Normalized());
	frustum->front = newfront.Normalized();
	frustum->up = newup.Normalized();

	frustum->Translate(-frustum->front * radius);

}
update_status ModuleCamera::PreUpdate()
{
	float currentFrame = SDL_GetTicks();
	deltaTime = (currentFrame - lastFrame)/100.0f;
	lastFrame = currentFrame;
	return UPDATE_CONTINUE;
}

// Called every draw update
update_status ModuleCamera::Update()
{
	ManageKeyboardInput();
	ManageMouseInput();
	return UPDATE_CONTINUE;
}

update_status ModuleCamera::PostUpdate()
{
	return UPDATE_CONTINUE;
}


bool ModuleCamera::CleanUp() 
{
	return true;
}
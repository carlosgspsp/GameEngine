#include ".\backends\imgui_impl_sdl2.h"
#include ".\backends\imgui_impl_opengl3.h"
#include "ModuleEditor.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleOpenGL.h"
#include "ModuleCamera.h"
#include "ModuleRenderExercise.h"
#include "DirectXTex/DirectXTex.h"
#include "SDL.h"
#include "Model.h"
#include <.\GL\glew.h>
#include "imgui.h"
#include "MathGeoLib.h"
#include "Mesh.h"



#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

ModuleEditor::ModuleEditor() {
	logs = new ImGuiTextBuffer;

}
ModuleEditor::~ModuleEditor() {
	delete logs;
}

bool ModuleEditor::Init()
{

	ImGui::CreateContext();

	io = &(ImGui::GetIO()); (void)io;
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	//io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	   // Enable ImGui outside Viewports


	ImGui_ImplSDL2_InitForOpenGL(App->GetWindow()->window, App->GetOpenGL()->context);
	ImGui_ImplOpenGL3_Init("#version 460");


	width = App->GetWindow()->GetScreenSize().x;
	height = App->GetWindow()->GetScreenSize().y;

	cameraSpeed = App->GetCamera()->GetCameraSpeed();
	panSensitivity = App->GetCamera()->GetPanSensitivity();
	mouseSensitivity = App->GetCamera()->GetMouseSensitivity();
	zoomSensitivity = App->GetCamera()->GetZoomSensitivity();

	return true;
}

update_status ModuleEditor::Update() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->GetWindow()->window);
	ImGui::NewFrame();



	static bool demo = true;
	if (demo) {
		ImGui::ShowDemoWindow(&demo);
	}


	//ImGui::Begin("DockSpace");

	/*ImGuiID dockspace_id = ImGui::GetID("")
	ImGui::DockSpace();
	ImGui::End();
	*/
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);


	bool about_popup = false;
	static bool mainMenu_window = true;
	if (mainMenu_window) {
		ImGui::Begin("Main Menu", &mainMenu_window, ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("General"))
			{
				if (ImGui::MenuItem("GitHub"))
				{
					App->RequestBrowser("https://github.com/carlosgspsp/GameEngine");
				}
				if (ImGui::MenuItem("About"))
				{

					about_popup = true;



				}



				if (ImGui::MenuItem("Quit"))
				{
					return UPDATE_STOP;
				}
				ImGui::EndMenu();
			}


			ImGui::EndMenuBar();
		}

		if (about_popup) {
			ImGui::OpenPopup("About");
		}
		if (ImGui::BeginPopup("About")) {

			ImGui::Text("Noob Engine");
			ImGui::Text("Description: Game Engine created for UPC project");
			ImGui::Text("Author: Carlos Garcia Segura");
			ImGui::Text("Libraries: ");
			ImGui::Text("License: MIT");

			if (ImGui::Button("CLOSE", ImVec2(200, 0))) {
				ImGui::CloseCurrentPopup();
				about_popup = false;
			}

			ImGui::EndPopup();
		}

		if (ImGui::CollapsingHeader("Configuration"))
		{
			if (ImGui::TreeNode("Options"))
			{
				if (ImGui::CollapsingHeader("Application"))
				{
					char title[25];
					sprintf_s(title, 25, "Framerate %.1f", App->GetFrameRate()->at(App->GetFrameRate()->size() - 1));
					ImGui::PlotHistogram("##FrameRate", &(App->GetFrameRate()->at(0)), App->GetFrameRate()->size(), 0, title, 0.0f, 100.0f, ImVec2(310.0f, 100.0f));

					sprintf_s(title, 25, "Milliseconds %.1f", App->GetMilliseconds()->at(App->GetMilliseconds()->size() - 1));
					ImGui::PlotHistogram("##Milliseconds", &(App->GetMilliseconds()->at(0)), App->GetMilliseconds()->size(), 0, title, 0.0f, 40.0f, ImVec2(310.0f, 100.0f));
				}

				if (ImGui::CollapsingHeader("Window"))
				{

					if (ImGui::Checkbox("Fullscreen", &fullScreen)) {
						App->GetWindow()->SetFullScreen(fullScreen);

					}
					ImGui::SameLine();
					if (ImGui::Checkbox("Resizable", &resizable)) {
						App->GetWindow()->SetResizable(resizable);

					}
					
					if (resizable) {

						if (ImGui::SliderInt("Width", &width, 1, 3840)) {

							SDL_SetWindowSize(App->GetWindow()->window, width, height);
							App->GetOpenGL()->WindowResized(width, height);
						}
						if (ImGui::SliderInt("Height", &height, 1, 2160)) {
							SDL_SetWindowSize(App->GetWindow()->window, width, height);
							App->GetOpenGL()->WindowResized(width, height);

						}
					}

					
				}
				if (ImGui::CollapsingHeader("Camera")) {
					//ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0));
					if (ImGui::InputFloat("Camera Speed", &cameraSpeed, 0.05f, 2.0f)) {
						App->GetCamera()->SetCameraSpeed(cameraSpeed);
					}
					if (ImGui::InputFloat("Zoom Sensitivity", &zoomSensitivity, 0.05f, 2.0f)) {
						App->GetCamera()->SetZoomSensitivity(zoomSensitivity);
					}
					if (ImGui::InputFloat2("Camera Rotation Sensitivity", mouseSensitivity.ptr())) {
						App->GetCamera()->SetMouseSensitivity(mouseSensitivity);
					}
					if (ImGui::InputFloat2("Pan Sensitivity", panSensitivity.ptr())) {
						App->GetCamera()->SetMouseSensitivity(panSensitivity);
					}
				}


				ImGui::TreePop();
			}

		}


		if (ImGui::CollapsingHeader("Properties"))
		{
			if (ImGui::TreeNode("Geometry"))
			{
				const std::vector<Mesh*>* meshes = App->GetModuleRenderExercise()->GetModel()->GetMeshes();
				for (int i = 0; i < meshes->size(); i++) {
					ImGui::Separator();
					ImGui::Text("Mesh name: %s", meshes->at(i)->GetName()->c_str());
					ImGui::Text("Indices: %i", meshes->at(i)->GetIndexCount());
					ImGui::Text("Vertices: %i", meshes->at(i)->GetVertexCount());
					ImGui::Text("Triangles: %i", meshes->at(i)->GetIndexCount() / 3);
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Textures"))
			{

				const tinygltf::Model* model = App->GetModuleRenderExercise()->GetModel()->GetSrcModel();
				std::vector<tinygltf::Image> images = model->images;
				std::vector<DirectX::ScratchImage*> scrImages = App->GetModuleRenderExercise()->GetModel()->GetScrImages();

				for (int i = 0; i < images.size(); i++) {
					ImGui::Separator();
					ImGui::Text("File name: %s", images[i].name.c_str());
					ImGui::Text("Width: %i", scrImages[i]->GetMetadata().width);
					ImGui::SameLine();
					ImGui::Text("Height: %i", scrImages[i]->GetMetadata().height);
				}



				ImGui::TreePop();
			}
		}


		if (ImGui::CollapsingHeader("Hardware"))
		{

			SDL_version version;
			SDL_VERSION(&version);

			GLfloat total_vram, available_vram, usage;
			glGetFloatv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &total_vram);
			glGetFloatv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &available_vram);


			usage = (total_vram - available_vram) / total_vram;

			ImGui::Text("SDL Version: %u.%u.%u", version.major, version.minor, version.patch);
			ImGui::Text("OpenGL Supported Version: %s", glGetString(GL_VERSION));
			ImGui::Text("Glew Version: %s", glewGetString(GLEW_VERSION));
			ImGui::Text("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
			ImGui::Text("DirectXTex Version: %u", DIRECTX_TEX_VERSION);
			ImGui::Text("ImGui Version: %s", ImGui::GetVersion());
			ImGui::Separator();
			ImGui::Text("CPUs: %i (Cache: %.1fkb)", SDL_GetCPUCount(), SDL_GetCPUCacheLineSize());
			ImGui::Text("System RAM: %.1fGB ", SDL_GetSystemRAM());
			ImGui::Separator();
			ImGui::Text("GPU Vendor: %s", glGetString(GL_VENDOR));
			ImGui::Text("GPU Brand: %s", glGetString(GL_RENDERER));
			ImGui::Text("VRAM Budget: %.1fMB", (total_vram / 1024.0f));
			ImGui::Text("VRAM Usage: %.2f%%", usage * 100.0f);
			ImGui::Text("VRAM Available: %.2fMB", available_vram / 1024);
		}


		ImGui::End();

	}

	static bool console_window = true;
	if (console_window) {
		static bool autoscroll = true;
		ImGui::Begin("Console", &console_window);

		ImGui::Checkbox("AutoScroll", &autoscroll);
		ImGui::Separator();
		ImGui::BeginChild("##ConsolePanel", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::TextUnformatted(logs->begin(), logs->end());

		if (autoscroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();



		ImGui::End();
	}






	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}


	return UPDATE_CONTINUE;

}


void ModuleEditor::AddLog(char str[]) {
	logs->appendf(str);
}

bool ModuleEditor::CleanUp() {

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}
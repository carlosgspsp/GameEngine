#pragma once
#include "Module.h"
#include "Globals.h"


class Model;

class ModuleRenderExercise :
    public Module
{
public:
	ModuleRenderExercise();
	~ModuleRenderExercise();

	bool Init();
	//update_status PreUpdate();
	update_status Update();
	//update_status PostUpdate();
	bool CleanUp();

	Model* GetModel() const;


private:
	
	unsigned program_id = 0, texture_id = 0;
	void RenderWorld();
	
	ModuleCamera* camera = nullptr;
	Model* model = nullptr;
};


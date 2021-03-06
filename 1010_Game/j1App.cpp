#include <iostream> 

#include "p2Defs.h"
#include "p2Log.h"

#include "j1Window.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Scene.h"
#include "j1App.h"
#include "j1Fonts.h"
#include "j1Data.h"
#include "j1FadeScene.h"
#include "j1UserInterface.h"
#include "Brofiler\Brofiler.h"

// Constructor
j1App::j1App(int argc, char* args[]) : argc(argc), args(args)
{
	PERF_START(perfTimer);

	want_to_save = want_to_load = false;

	input = new j1Input();
	win = new j1Window();
	render = new j1Render();
	tex = new j1Textures();
	audio = new j1Audio();
	scene = new j1Scene();
	//map = new j1Map();
	//pathfinding = new j1PathFinding();
	font = new j1Fonts();
	gui = new j1UserInterface();
	//entityManager = new j1EntityManager();
	//collision = new j1Collision();
	data = new j1Data();
	fade = new j1FadeScene();

	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	AddModule(input);
	AddModule(win);
	AddModule(tex);
	AddModule(audio);
	//AddModule(map);
	//AddModule(pathfinding);
	AddModule(font);
	//AddModule(entityManager);
	//AddModule(collision);
	AddModule(gui);
	AddModule(data);

	// Scene and fade right before render
	AddModule(scene);
	AddModule(fade);

	// render last to swap buffer
	AddModule(render);

	PERF_PEEK(perfTimer);
}

// Destructor
j1App::~j1App()
{
	// release modules
	p2List_item<j1Module*>* item = modules.end;

	while(item != NULL)
	{
		RELEASE(item->data);
		item = item->prev;
	}

	modules.clear();
}

void j1App::AddModule(j1Module* module)
{
	module->Init();
	modules.add(module);
}

// Called before render is available
bool j1App::Awake()
{
	PERF_START(perfTimer);

	pugi::xml_document	config_file;
	pugi::xml_node		config;
	pugi::xml_node		app_config;

	bool ret = false; 
	save_game = "save_game.xml";
	load_game = "save_game.xml";

	config = LoadConfig(config_file);

	if(config.empty() == false)
	{
		// self-config
		ret = true;
		app_config = config.child("app");
		name.create(app_config.child("title").child_value());
		organization.create(app_config.child("organization").child_value());
		
		save_game.create(app_config.child("save").child_value());	// @Carles
		load_game.create(app_config.child("load").child_value());	// @Carles

		fpsCap = app_config.attribute("fpsCap").as_uint();
		mustCapFPS = app_config.attribute("mustCap").as_bool();
	}

	if(ret == true)
	{
		p2List_item<j1Module*>* item;
		item = modules.start;

		while(item != NULL && ret == true)
		{
			ret = item->data->Awake(config.child(item->data->name.GetString()));
			item = item->next;
		}
	}

	PERF_PEEK(perfTimer);

	return ret;
}

// Called before the first frame
bool j1App::Start()
{
	PERF_START(perfTimer);

	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;

	while(item != NULL && ret == true)
	{
		ret = item->data->Start();
		item = item->next;
	}

	gameTimer.Start();

	PERF_PEEK(perfTimer);

	return ret;
}

// Called each loop iteration
bool j1App::Update()
{
	bool ret = true;
	PrepareUpdate();

	if(input->GetWindowEvent(WE_QUIT) == true)
		ret = false;

	if(ret == true)
		ret = PreUpdate();

	if(ret == true)
		ret = DoUpdate();

	if(ret == true)
		ret = PostUpdate();

	FinishUpdate();

	if (mustShutDown)
		ret = false;

	return ret;
}

// ---------------------------------------------
pugi::xml_node j1App::LoadConfig(pugi::xml_document& config_file) const
{
	pugi::xml_node ret;

	pugi::xml_parse_result result = config_file.load_file("config.xml");

	if(result == NULL)
		LOG("Could not load map xml file config.xml. pugi error: %s", result.description());
	else
		ret = config_file.child("config");

	return ret;
}

// ---------------------------------------------
void j1App::PrepareUpdate()
{
	BROFILER_CATEGORY("App FinishUpdate", Profiler::Color::Gray);

	if (App->input->GetKey(SDL_SCANCODE_F11) == KEY_DOWN)
		mustCapFPS = !mustCapFPS;

	totalFrameCount++;
	currFPS++;
	dt = frameTimer.ReadSec();

	// Restart timers
	frameTimer.Start();
	delayTimer.Start();

	/*
	frame_count++;
	last_sec_frame_count++;

	perfTimer.Start();*/
}

// ---------------------------------------------
void j1App::FinishUpdate()
{
	BROFILER_CATEGORY("App FinishUpdate", Profiler::Color::Gray);

	if(want_to_save == true)
		SavegameNow();

	if(want_to_load == true)
		LoadGameNow();
	
	FramerateLogic();
}

// Call modules before each loop iteration
bool j1App::PreUpdate()
{
	BROFILER_CATEGORY("App PreUpdate", Profiler::Color::GreenYellow);

	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;
	j1Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->PreUpdate();
	}

	return ret;
}

// Call modules on each loop iteration
bool j1App::DoUpdate()
{
	BROFILER_CATEGORY("App AllUpdates", Profiler::Color::Yellow);

	//if (App->scene->gamePaused == true)
		//dt = 0.0f;

	bool ret = true;
	p2List_item<j1Module*>* item;
	item = modules.start;
	j1Module* pModule = NULL;

	for (item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if (pModule->active == false) {
			continue;
		}

		ret = item->data->Update(dt);
	}

	return ret;
}

// Call modules after each loop iteration
bool j1App::PostUpdate()
{
	BROFILER_CATEGORY("App PostUpdate", Profiler::Color::YellowGreen);

	bool ret = true;
	p2List_item<j1Module*>* item;
	j1Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->PostUpdate();
	}

	return ret;
}

// Called before quitting
bool j1App::CleanUp()
{
	PERF_START(perfTimer);

	bool ret = true;

	p2List_item<j1Module*>* item;
	item = modules.end;

	while (item != NULL && ret == true)
	{
		ret = item->data->CleanUp();
		item = item->prev;
	}

	PERF_PEEK(perfTimer);

	return ret;
}

// ---------------------------------------
int j1App::GetArgc() const
{
	return argc;
}

// ---------------------------------------
const char* j1App::GetArgv(int index) const
{
	if(index < argc)
		return args[index];
	else
		return NULL;
}

// ---------------------------------------
const char* j1App::GetTitle() const
{
	return title.GetString();
}

// ---------------------------------------
const char* j1App::GetOrganization() const
{
	return organization.GetString();
}

// Load / Save
void j1App::LoadGame()
{
	// we should be checking if that file actually exist
	// from the "GetSaveGames" list
	want_to_load = true;
}

// ---------------------------------------
void j1App::SaveGame() const
{
	// we should be checking if that file actually exist
	// from the "GetSaveGames" list ... should we overwrite ?

	want_to_save = true;
}

// ---------------------------------------
void j1App::GetSaveGames(p2List<p2SString>& list_to_fill) const
{
	// need to add functionality to file_system module for this to work
}

pugi::xml_node j1App::GetSaveData()
{
	pugi::xml_node root;

	pugi::xml_parse_result result = save_gamedata.load_file(load_game.GetString());

	if (result != NULL)
	{
		LOG("Loading new Game State from %s...", load_game.GetString());

		root = save_gamedata.child("game_state");
		return root;
	}
	else
		LOG("Could not parse game state xml file %s. pugi error: %s", load_game.GetString(), result.description());
	return root;
}

bool j1App::LoadGameNow()
{
	bool ret = false;

	pugi::xml_node root = GetSaveData();

	if(root != NULL)
	{
		LOG("Loading new Game State from %s...", load_game.GetString());

		p2List_item<j1Module*>* item = modules.start;
		ret = true;

		while(item != NULL && ret == true)
		{
			if (item->data->IsEnabled())
				ret = item->data->Load(root.child(item->data->name.GetString()));
			item = item->next;
		}

		save_gamedata.reset();
		if(ret == true)
			LOG("...finished loading");
		else
			LOG("...loading process interrupted with error on module %s", (item != NULL) ? item->data->name.GetString() : "unknown");
	}

	want_to_load = false;
	return ret;
}

bool j1App::SavegameNow() const
{
	bool ret = true;

	LOG("Saving Game State to %s...", save_game.GetString());

	// xml object were we will store all data
	pugi::xml_document datadoc;
	pugi::xml_node root;
	
	root = datadoc.append_child("game_state");

	p2List_item<j1Module*>* item = modules.start;

	data->Save(root.append_child("data"));
	/*while(item != NULL && ret == true)
	{
		if(item->data->IsEnabled())
			ret = item->data->Save(root.append_child(item->data->name.GetString()));
		item = item->next;
	}*/

	if(ret == true)
	{
		datadoc.save_file(save_game.GetString());
		LOG("... finished saving", );
	}
	else
		LOG("Save process halted from an error in module %s", (item != NULL) ? item->data->name.GetString() : "unknown");

	datadoc.reset();
	want_to_save = false;
	return ret;
}

void j1App::FramerateLogic()
{
	if (secTimer.Read() > 1000) {
		secTimer.Start();
		prevFPS = currFPS;
		currFPS = 0;
	}

	avgFPS = float(totalFrameCount) / gameTimer.ReadSec();
	gameTime = gameTimer.ReadSec();
	lastFrameMs = frameTimer.Read();

	if (debugMode) {
		App->win->SetTitle(DebugTitle().GetString());
	}
	else {
		App->win->SetTitle(DefaultTitle().GetString());
	}

	if (mustCapFPS) {
		int delayTime = (1000 / fpsCap) - lastFrameMs;
		if (delayTime > 0) {
			SDL_Delay((Uint32)delayTime);
			//LOG("We waited for %u and got back in %f", delayTime, delayTimer.ReadMs());
		}
	}
}

p2SString j1App::DebugTitle()	// @Carles
{
	/*iPoint playerPos;
	if (App->entityManager->player != nullptr) {
		playerPos = { (int)App->entityManager->player->GetPosition().x, (int)App->entityManager->player->GetPosition().y, };
	}
	else {
		playerPos = { 0, 0 };
	}*/

	title.create("%s (FPS: %i / Av.FPS: %.2f / MsPF: %02u ms / fpsCap: %i / Vsync: %i / Play Time: %.3f / Camera: %dx%d)",
		name.GetString(),
		prevFPS,
		avgFPS,
		lastFrameMs,
		(int)mustCapFPS,
		(int)App->render->vSync,
		gameTime,
		App->render->camera.x, App->render->camera.y);

	return title;
}

p2SString j1App::DefaultTitle()	// @Carles
{
	title.create("%s (FPS: %i / Av.FPS: %.2f / MsPF: %02u ms / fpsCap: %i / Vsync: %i)",
		name.GetString(),
		prevFPS,
		avgFPS,
		lastFrameMs,
		(int)mustCapFPS,
		(int)App->render->vSync);

	return title;
}
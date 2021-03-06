#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include "j1Figure.h"
#include "j1PerfTimer.h"

class Text;
struct SDL_Texture;

struct Line {
	int col;
	int row;
	int index = 0;
};

struct Grid {
	iPoint position;
	Cell* cells[10][10];
};

enum class scene_type {
	NONE = -1,
	MAIN_MENU,
	SETTINGS,
	CREDITS,

	GAME,

	MAX_SCENES
};

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
	bool Awake(pugi::xml_node& config);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	bool deleteLines();

	bool checkPosibilities();

	bool detectLines();

	void createFigures();

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool Load(pugi::xml_node & data);

	bool Save(pugi::xml_node & data) const;

	bool checkFigures();

	bool isValid(iPoint cell, j1Figure* figure, bool fill = true);

private:
	void CheckInputs();
	void RegisterButtonData(pugi::xml_node&, SDL_Rect* button);
	void ChangeScene(scene_type scene);
	int UpdateScoreboard();

public:
	scene_type scene;
	SDL_Texture* texture_bricks;
	p2List<SDL_Rect*> piece_colors;
	Text* difficultyTxt;
	int difficulty = 1;
	bool randomScore;
	bool versionA = true;

private:
	//Default UI data list
	SDL_Rect panelNormal;
	SDL_Rect panelShort;
	SDL_Rect panelLong;
	SDL_Rect panelSquare;
	SDL_Rect window;
	SDL_Rect leftArrow;
	SDL_Rect rightArrow;
	SDL_Rect* button;

	//Specific UI Data
	SDL_Rect title;
	SDL_Rect webpage;
	SDL_Rect restart;
	SDL_Rect* sound;
	SDL_Rect* version;

	//Game
	Grid grid;
	p2List<j1Figure*> figures;
	p2List<Line> lines;
	int cell_size;
	int cell_offset;
	j1PerfTimer del_time;
	bool check = false;
	bool deleting = false;
	p2SString image_string;
	int dif_prov[3][3] = { {60,90,100},{ 40,80,100},{30,70,100} };

	//Score
	Text* scoreTxt;
	int score = 0;
	int scoreGain = 10;
	int maxPoints, minPoints;
};

#endif // __j1SCENE_H__
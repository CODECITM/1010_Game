#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"
#include "j1Figure.h"

struct SDL_Texture;


struct Grid {
	iPoint position;
	Cell* cells[10][10];
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

	bool checkPosibilities();

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool Load(pugi::xml_node & data);

	bool Save(pugi::xml_node & data) const;

	bool checkFigures();

	bool isValid(iPoint cell, j1Figure* figure, bool fill = true);

private:
	Grid grid;
	p2List<j1Figure*> figures;
	int cell_size;

};

#endif // __j1SCENE_H__
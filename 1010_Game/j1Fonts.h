#ifndef __j1FONTS_H__
#define __j1FONTS_H__

#include "j1Module.h"
#include "SDL\include\SDL_pixels.h"

#define DEFAULT_FONT "open_sans/OpenSans-Regular.ttf"
#define DEFAULT_FONT_SIZE 12
#define WHITE_FONT { 255, 255, 255, 255 }
#define BLACK_FONT { 0, 0, 0, 255 }

struct SDL_Texture;
struct _TTF_Font;

class j1Fonts : public j1Module
{
public:

	j1Fonts();

	// Destructor
	virtual ~j1Fonts();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before quitting
	bool CleanUp();

	// Load Font
	_TTF_Font* const Load(const char* path, int size = 12);

	// Create a surface from text
	SDL_Texture* Print(const char* text, SDL_Color color = WHITE_FONT, _TTF_Font* font = NULL);

	bool CalcSize(const char* text, int& width, int& height, _TTF_Font* font = NULL) const;

public:
	p2List<_TTF_Font*>	fonts;
	_TTF_Font*			defaultFont;
	_TTF_Font*			titleFont;
	_TTF_Font*			textFont;

private:
	p2SString folder;
};


#endif // __j1FONTS_H__
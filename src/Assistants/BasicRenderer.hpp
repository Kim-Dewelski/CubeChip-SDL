/*
	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

#pragma warning(push)
#pragma warning(disable : 26819) // C fallthrough warning disabled
#include <SDL.h>
#pragma warning(pop)

#include <string>

class BasicRenderer {
	SDL_Window*   window{};
	SDL_Renderer* renderer{};
	SDL_Texture*  texture{};

	const std::string emuName;
	const std::string emuVersion;
		  std::string windowTitle;

public:
	explicit BasicRenderer(const Sint32, const Sint32);
	~BasicRenderer();

	static bool showErrorBoxSDL(std::string_view);
	static bool showErrorBox(std::string_view, std::string_view);

	void changeTitle(std::string_view);
	void createWindow();
	void createRenderer();
	void createTexture(const Sint32, const Sint32);

	Sint32 window_W;
	Sint32 window_H;

	float   aspect{};
	Sint32  ppitch{};
	Uint32* pixels{};

	void lockTexture();
	void unlockTexture();
	void renderPresent();

	void getWindowSize(const bool);

	void setTextureAlpha(std::size_t);
	void setTextureBlend(SDL_BlendMode);
	void setAspectRatio(float);

	void quitWindow();
	void quitRenderer();
	void quitTexture();
};

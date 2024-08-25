#pragma region License
/*
License (OLC-3)
~~~~~~~~~~~~~~~

Copyright 2024 Joshua Sigona <sigonasr2@gmail.com>

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions or derivations of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions or derivative works in binary form must reproduce the above
copyright notice. This list of conditions and the following	disclaimer must be
reproduced in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may
be used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS	"AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL,	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

Portions of this software are copyright © 2024 The FreeType
Project (www.freetype.org). Please see LICENSE_FT.txt for more information.
All rights reserved.
*/
#pragma endregion
#pragma once

#include "olcPixelGameEngine.h"
#include <functional>
#include "HamsterNet.h"

class HamsterGame;
class Menu{
	class Button{
		vf2d pos;
		std::string buttonImg;
		std::string highlightButtonImg;
		Pixel textCol;
		Pixel highlightTextCol;
		std::function<void(Button&self)>onClick;
	public:
		std::string buttonText;
		Button(const vf2d pos,const std::string&buttonText,const std::string&buttonImg,const std::string&highlightButtonImg,const Pixel textCol,const Pixel highlightTextCol,const std::function<void(Button&self)>onClick={});
		const bool IsHovered(const vf2d&offset)const;
		void OnClick();
		void Draw(HamsterGame&game,const vf2d&pos,std::optional<std::reference_wrapper<Button>>selectedButton={})const;
	};
	enum MenuType{
		INITIALIZE,
		TITLE_SCREEN,
		MAIN_MENU,
		GRAND_PRIX,
		SINGLE_RACE,
		OPTIONS,
		GAMEPLAY,
		GAMEPLAY_RESULTS,
		AFTER_RACE_MENU,
		PAUSE,
		LOADING,
		QUIT,
	};
	enum TransitionType{
		SHIFT_LEFT,
		SHIFT_RIGHT,
		SHIFT_UP,
		SHIFT_DOWN,
		FADE_OUT,
	};
	enum MenuState{
		NORMAL,
		TRANSITIONING,
	};
	MenuState currentState{NORMAL};
	TransitionType currentTransition{FADE_OUT};
	MenuType currentMenu{INITIALIZE};
	MenuType nextMenu{TITLE_SCREEN};
	float menuTimer{};
	const float MENU_TRANSITION_REFRESH_RATE{0.1f};
	float menuTransitionRefreshTimer{MENU_TRANSITION_REFRESH_RATE};
	float originalMenuTimer{};
	vi2d oldLayerPos{};
	vi2d newLayerPos{};
	int colorNumb{1};
	bool loading{false};
	float loadingPct{0.f};
	std::vector<Button>menuButtons;
	std::vector<Button>newMenuButtons;
	std::string selectedMap{"StageI.tmx"};
	std::optional<int>selectedButton;
	int lastHovered{};
	void Transition(const TransitionType type,const MenuType gotoMenu,const float transitionTime);
	void Draw(HamsterGame&game,const MenuType menu,const vi2d pos);
	void DrawTransition(HamsterGame&game);
	void OnMenuTransition();
	std::vector<Button>GetMenuButtons(const MenuType type);
	bool ignoreInputs{false};
	std::vector<LeaderboardEntry>loadedLeaderboard;
public:
	void UpdateAndDraw(HamsterGame&game,const float fElapsedTime);
	void OnLevelLoaded();
	void UpdateLoadingProgress(const float pctLoaded);
	void OnTextEntryComplete(const std::string&text);
};
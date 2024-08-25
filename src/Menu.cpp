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

#include "Menu.h"
#include "HamsterGame.h"
#include "util.h"
#include "Hamster.h"

void Menu::UpdateAndDraw(HamsterGame&game,const float fElapsedTime){
	menuTransitionRefreshTimer-=fElapsedTime;

	for(int i{0};const Button&b:menuButtons){
		if(b.IsHovered(oldLayerPos+game.SCREEN_FRAME.pos))selectedButton=i;
		i++;
	}

	if(menuButtons.size()>0&&menuTimer==0.f){
		if(game.GetKey(W).bPressed||game.GetKey(UP).bPressed||game.GetKey(A).bPressed||game.GetKey(LEFT).bPressed){
			if(selectedButton.value()-1<0)selectedButton=menuButtons.size()-1;
			else selectedButton.value()--;
		}
		if(game.GetKey(S).bPressed||game.GetKey(DOWN).bPressed||game.GetKey(D).bPressed||game.GetKey(RIGHT).bPressed){
			if(selectedButton.value()+1>=menuButtons.size())selectedButton=0;
			else selectedButton.value()++;
		}
		if(game.GetKey(ENTER).bPressed||game.GetKey(SPACE).bPressed||menuButtons[selectedButton.value()].IsHovered(oldLayerPos+game.SCREEN_FRAME.pos)&&game.GetMouse(Mouse::LEFT).bPressed){
			menuButtons[selectedButton.value()].OnClick();
		}
	}

	if(menuTimer>0.f){
		menuTimer-=fElapsedTime;
		if(menuTimer<=0.f){
			menuTimer=0.f;
			currentMenu=nextMenu;
			oldLayerPos={};
			newLayerPos=game.SCREEN_FRAME.pos;
			OnMenuTransition();
		}
	}

	game.EnableLayer(0U,menuTimer>0.f);

	switch(currentMenu){
		case INITIALIZE:{
			Transition(FADE_OUT,TITLE_SCREEN,1.f);
		}break;
		case TITLE_SCREEN:{
			if(game.GetKey(SPACE).bPressed||game.GetKey(ENTER).bPressed||game.GetMouse(Mouse::LEFT).bPressed){
				Transition(SHIFT_LEFT,MAIN_MENU,0.5f);
			}
		}break;
		case MAIN_MENU:{
		}break;
		case GAMEPLAY:{
			game.UpdateGame(fElapsedTime);
			game.DrawGame();
		}break;
		case GAMEPLAY_RESULTS:{
			game.DrawGame();
		}break;
		case LOADING:{
			if(loading)game.ProcessMap();
		}break;
	}

	if(menuTimer>0.f){
		DrawTransition(game);
		if(menuTransitionRefreshTimer<=0.f){
			menuTransitionRefreshTimer=MENU_TRANSITION_REFRESH_RATE;
		}
	}else{
		game.SetDrawTarget(nullptr);
		Draw(game,currentMenu,game.SCREEN_FRAME.pos);
	}
	game.border.Update(fElapsedTime);
}
void Menu::Transition(const TransitionType type,const MenuType gotoMenu,const float transitionTime){
	if(menuTimer>0.f)return;
	menuTimer=originalMenuTimer=transitionTime;
	nextMenu=gotoMenu;
	currentTransition=type;
	newMenuButtons=GetMenuButtons(gotoMenu);
}
std::vector<Menu::Button>Menu::GetMenuButtons(const MenuType type){
	std::vector<Menu::Button>buttons;
	switch(type){
		case MAIN_MENU:{
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-32.f},"Grand Prix","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](){Transition(SHIFT_LEFT,GRAND_PRIX,0.5f);});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,0.f},"Single Race","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](){Transition(SHIFT_UP,SINGLE_RACE,0.5f);});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,32.f},"Options","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](){Transition(SHIFT_RIGHT,OPTIONS,0.5f);});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,64.f},"Quit","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](){Transition(SHIFT_DOWN,QUIT,0.5f);});
		}break;
		case GRAND_PRIX:{
			//Add more buttons up here!
			buttons.emplace_back(vf2d{54.f,HamsterGame::SCREEN_FRAME.size.y-24.f},"< Back","button3.png","highlight_button3.png",Pixel{145,199,255},Pixel{145,199,255},[this](){Transition(SHIFT_RIGHT,MAIN_MENU,0.5f);});
		}break;
		case SINGLE_RACE:{
			//Add more buttons up here!
			buttons.emplace_back(vf2d{54.f,HamsterGame::SCREEN_FRAME.size.y-24.f},"< Back","button4.png","highlight_button4.png",Pixel{220,185,155},Pixel{180,140,152},[this](){Transition(SHIFT_DOWN,MAIN_MENU,0.5f);});
		}break;
		case OPTIONS:{
			//Add more buttons up here!
			buttons.emplace_back(vf2d{54.f,HamsterGame::SCREEN_FRAME.size.y-24.f},"< Back","button2.png","highlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](){Transition(SHIFT_LEFT,MAIN_MENU,0.5f);});
		}break;
	}
	return buttons;
}
void Menu::OnMenuTransition(){
	selectedButton.reset();
	menuButtons.clear();
	newMenuButtons.clear();
	menuButtons=GetMenuButtons(currentMenu);
	switch(currentMenu){
		case LOADING:{
			colorNumb=util::random()%8+1;
			loading=true;
			loadingPct=0.f;
			HamsterGame::Game().LoadRace(selectedMap);
		}break;
		case QUIT:{
			HamsterGame::Game().QuitGame();
		}break;
	}
	if(menuButtons.size()>0)selectedButton=0;
}
void Menu::DrawTransition(HamsterGame&game){
	if(currentTransition==FADE_OUT){
		if(menuTimer>=originalMenuTimer/2){//Fading out from old scene.
			game.SetDrawTarget(1U);
			Draw(game,currentMenu,game.SCREEN_FRAME.pos);
			game.SetDrawTarget(nullptr);
			game.Clear(BLACK);
			if(menuTransitionRefreshTimer<=0.f)game.SetLayerTint(0U,{255,255,255,uint8_t(util::lerp(255,0,(menuTimer-originalMenuTimer/2)/(originalMenuTimer/2)))});
		}else{//Fading into new scene.
			game.SetDrawTarget(1U);
			Draw(game,nextMenu,game.SCREEN_FRAME.pos);
			game.SetDrawTarget(nullptr);
			game.Clear(BLACK);
			if(menuTransitionRefreshTimer<=0.f)game.SetLayerTint(0U,{255,255,255,uint8_t(util::lerp(0,255,menuTimer/(originalMenuTimer/2)))});
		}
	}else{
		if(menuTransitionRefreshTimer<=0.f){
			switch(currentTransition){
				case SHIFT_LEFT:{
					oldLayerPos=vi2d{int(util::lerp(-game.SCREEN_FRAME.size.x,0,menuTimer/originalMenuTimer)),0};
					newLayerPos=vi2d{int(util::lerp(0,game.SCREEN_FRAME.size.x,menuTimer/originalMenuTimer)),0};
				}break;
				case SHIFT_RIGHT:{
					oldLayerPos=vi2d{int(util::lerp(game.SCREEN_FRAME.size.x,0,menuTimer/originalMenuTimer)),0};
					newLayerPos=vi2d{int(util::lerp(0,-game.SCREEN_FRAME.size.x,menuTimer/originalMenuTimer)),0};
				}break;
				case SHIFT_UP:{
					oldLayerPos=vi2d{0,int(util::lerp(-game.SCREEN_FRAME.size.y,0,menuTimer/originalMenuTimer))};
					newLayerPos=vi2d{0,int(util::lerp(0,game.SCREEN_FRAME.size.y,menuTimer/originalMenuTimer))};
				}break;
				case SHIFT_DOWN:{
					oldLayerPos=vi2d{0,int(util::lerp(game.SCREEN_FRAME.size.y,0,menuTimer/originalMenuTimer))};
					newLayerPos=vi2d{0,int(util::lerp(0,-game.SCREEN_FRAME.size.y,menuTimer/originalMenuTimer))};
				}break;
			}
		}
		game.SetDrawTarget(1U);
		game.SetLayerOffset(1U,oldLayerPos+game.SCREEN_FRAME.pos);
		Draw(game,currentMenu,oldLayerPos+game.SCREEN_FRAME.pos);
		game.SetDrawTarget(nullptr);
		game.SetLayerTint(0U,WHITE);
		game.SetLayerOffset(0U,newLayerPos+game.SCREEN_FRAME.pos);
		Draw(game,nextMenu,newLayerPos+game.SCREEN_FRAME.pos);
	}
	game.SetDrawTarget(nullptr);
}

void Menu::Draw(HamsterGame&game,const MenuType menu,const vi2d pos){
	game.Clear(BLANK);
	const auto DrawButtons=[this,&game](const vf2d&offset){
		for(int i{0};const Button&b:menuButtons){
			if(selectedButton.has_value())b.Draw(game,oldLayerPos+game.SCREEN_FRAME.pos,menuButtons[selectedButton.value()]);
			else b.Draw(game,oldLayerPos+game.SCREEN_FRAME.pos);
			if(selectedButton.has_value())b.Draw(game,oldLayerPos+game.SCREEN_FRAME.pos,menuButtons[selectedButton.value()]);
			else b.Draw(game,oldLayerPos+game.SCREEN_FRAME.pos);
		}
		for(int i{0};const Button&b:newMenuButtons){
			if(selectedButton.has_value())b.Draw(game,newLayerPos+game.SCREEN_FRAME.pos,menuButtons[selectedButton.value()]);
			else b.Draw(game,newLayerPos+game.SCREEN_FRAME.pos);
		}
	};

	switch(menu){
		case TITLE_SCREEN:{
			game.FillRectDecal(pos,game.SCREEN_FRAME.size,{111,150,255});
			game.DrawDecal(pos,game.GetGFX("welcometo.png").Decal());
			if(menuTransitionRefreshTimer>0.f)game.DrawDecal(pos,game.GetGFX("hamsterplanet1.png").Decal());
			if(fmod(game.GetRuntime(),2.f)<1.f)game.DrawDecal(pos,game.GetGFX("hamsterplanet2.png").Decal());
			else game.DrawDecal(pos,game.GetGFX("hamsterplanet3.png").Decal());
			game.border.Draw();
		}break;
		case MAIN_MENU:{
			game.DrawPartialDecal(vi2d{pos},game.SCREEN_FRAME.size,game.GetGFX("background1.png").Decal(),vf2d{}+int(game.GetRuntime()*4),game.SCREEN_FRAME.size);
			game.DrawRotatedDecal(pos,game.GetGFX("button.png").Decal(),0.f,game.GetGFX("button.png").Sprite()->Size()/2);
			DrawButtons(pos);
			game.border.Draw();
		}break;
		case GRAND_PRIX:{
			game.DrawPartialDecal(vi2d{pos},game.SCREEN_FRAME.size,game.GetGFX("background5.png").Decal(),vf2d{}+int(game.GetRuntime()*4),game.SCREEN_FRAME.size);
			DrawButtons(pos);
			game.border.Draw();
		}break;
		case SINGLE_RACE:{
			game.DrawPartialDecal(vi2d{pos},game.SCREEN_FRAME.size,game.GetGFX("background4.png").Decal(),vf2d{}+int(game.GetRuntime()*4),game.SCREEN_FRAME.size);
			DrawButtons(pos);
			game.border.Draw();
		}break;
		case OPTIONS:{
			game.DrawPartialDecal(vi2d{pos},game.SCREEN_FRAME.size,game.GetGFX("background2.png").Decal(),vf2d{}+int(game.GetRuntime()*4),game.SCREEN_FRAME.size);
			DrawButtons(pos);
			game.border.Draw();
		}break;
		case QUIT:{
			game.DrawPartialDecal(vi2d{pos},game.SCREEN_FRAME.size,game.GetGFX("background3.png").Decal(),vf2d{}+int(game.GetRuntime()*4),game.SCREEN_FRAME.size);
		}break;
		case GAMEPLAY:{
			game.DrawGame();
		}break;
		case GAMEPLAY_RESULTS:{
			game.DrawGame();
		}break;
		case LOADING:{
			game.DrawPartialDecal(vi2d{pos},game.SCREEN_FRAME.size,game.GetGFX("background3.png").Decal(),vf2d{}+int(game.GetRuntime()*4),game.SCREEN_FRAME.size);
			game.FillRectDecal(pos+vf2d{32.f,game.SCREEN_FRAME.size.y-64.f}+vf2d{2.f,2.f},vf2d{loadingPct*(game.SCREEN_FRAME.size.x-64),32.f},BLACK);
			game.GradientFillRectDecal(pos+vf2d{32.f,game.SCREEN_FRAME.size.y-64.f},vf2d{loadingPct*(game.SCREEN_FRAME.size.x-64),32.f},{250,177,163},{255,224,194},{255,224,194},{250,177,163});
			int animationFrame{3};
			if(fmod(game.GetRuntime(),2.f)<0.5f)animationFrame=0;
			else if(fmod(game.GetRuntime(),2.f)<1.f)animationFrame=2;
			else if(fmod(game.GetRuntime(),2.f)<1.5f)animationFrame=0;
			game.DrawPartialRotatedDecal(pos+vf2d{32.f,game.SCREEN_FRAME.size.y-64.f}+vf2d{loadingPct*(game.SCREEN_FRAME.size.x-64)-10.f,8.f},game.GetGFX(std::format("hamster{}.png",colorNumb)).Decal(),0.f,{16.f,16.f},vf2d{float(animationFrame*32),0.f},{32.f,32.f});
		}break;
	}
}

void Menu::OnLevelLoaded(){
	loading=false;
	
	HamsterGame::Game().mapImage.Decal()->Update();

	Powerup::Initialize(HamsterGame::Game().mapPowerupsTemp);
	Checkpoint::Initialize(HamsterGame::Game().checkpointsTemp);

	HamsterGame::Game().audio.Play(HamsterGame::Game().bgm.at(HamsterGame::Game().currentMap.value().GetData().GetBGM()),true);
	Hamster::MoveHamstersToSpawn(HamsterGame::Game().currentMap.value().GetData().GetSpawnZone());
	HamsterGame::Game().countdownTimer=3.f;
	
	HamsterGame::Game().camera.SetTarget(Hamster::GetPlayer().GetPos());

	Transition(FADE_OUT,GAMEPLAY,0.5f);
}

void Menu::UpdateLoadingProgress(const float pctLoaded){
	loadingPct=pctLoaded;
}

Menu::Button::Button(const vf2d pos,const std::string&buttonText,const std::string&buttonImg,const std::string&highlightButtonImg,const Pixel textCol,const Pixel highlightTextCol,const std::function<void()>onClick)
:pos(pos),buttonText(buttonText),buttonImg(buttonImg),highlightButtonImg(highlightButtonImg),onClick(onClick),textCol(textCol),highlightTextCol(highlightTextCol){}

const bool Menu::Button::IsHovered(const vf2d&offset)const{
	return geom2d::overlaps(HamsterGame::Game().GetMousePos(),geom2d::rect<float>(pos-HamsterGame::GetGFX(buttonImg).Sprite()->Size()/2+offset,HamsterGame::GetGFX(buttonImg).Sprite()->Size()));
}
void Menu::Button::Draw(HamsterGame&game,const vf2d&offset,std::optional<std::reference_wrapper<Button>>selectedButton)const{
	if(selectedButton.has_value()&&&selectedButton.value().get()==this){
		game.DrawRotatedDecal(pos+offset,game.GetGFX(highlightButtonImg).Decal(),0.f,game.GetGFX(highlightButtonImg).Sprite()->Size()/2);
		game.DrawRotatedStringPropDecal(pos+offset,buttonText,0.f,game.GetTextSizeProp(buttonText)/2,highlightTextCol);
	}else{
		game.DrawRotatedDecal(pos+offset,game.GetGFX(buttonImg).Decal(),0.f,game.GetGFX(buttonImg).Sprite()->Size()/2);
		game.DrawRotatedStringPropDecal(pos+offset,buttonText,0.f,game.GetTextSizeProp(buttonText)/2,textCol);
	}
}

void Menu::Button::OnClick(){
	onClick();
}
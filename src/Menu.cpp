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

Portions of this software are copyright � 2024 The FreeType
Project (www.freetype.org). Please see LICENSE_FT.txt for more information.
All rights reserved.
*/
#pragma endregion

#include "Menu.h"
#include "HamsterGame.h"
#include "util.h"
#include "Hamster.h"

void Menu::UpdateAndDraw(HamsterGame&game,const float fElapsedTime){
	if(HamsterGame::Game().IsTextEntryEnabled()||ignoreInputs)goto Drawing;
	menuTransitionRefreshTimer-=fElapsedTime;

	for(int i{0};const Button&b:menuButtons){
		if(b.IsHovered(oldLayerPos+game.SCREEN_FRAME.pos)&&selectedButton.value()!=i){
			selectedButton=i;
			HamsterGame::PlaySFX("menu_hover.wav");
		}
		i++;
	}

	if(menuButtons.size()>0&&menuTimer==0.f){
		if(game.GetKey(W).bPressed||game.GetKey(UP).bPressed||game.GetKey(A).bPressed||game.GetKey(LEFT).bPressed){
			if(selectedButton.value()-1<0)selectedButton=menuButtons.size()-1;
			else selectedButton.value()--;
			HamsterGame::PlaySFX("menu_hover.wav");
		}
		if(game.GetKey(S).bPressed||game.GetKey(DOWN).bPressed||game.GetKey(D).bPressed||game.GetKey(RIGHT).bPressed){
			if(selectedButton.value()+1>=menuButtons.size())selectedButton=0;
			else selectedButton.value()++;
			HamsterGame::PlaySFX("menu_hover.wav");
		}
		if(game.GetKey(ENTER).bPressed||game.GetKey(SPACE).bPressed||menuButtons[selectedButton.value()].IsHovered(oldLayerPos+game.SCREEN_FRAME.pos)&&game.GetMouse(Mouse::LEFT).bPressed){
			menuButtons[selectedButton.value()].OnClick();
		}
	}

	if(menuTimer>0.f){
		menuTimer-=fElapsedTime;
		if(menuTimer<=0.f){
			menuTimer=0.f;
			MenuType previousMenu=currentMenu;
			currentMenu=nextMenu;
			oldLayerPos={};
			newLayerPos=game.SCREEN_FRAME.pos;
			OnMenuTransition(previousMenu);
		}
	}

	Drawing:
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
			if(game.GetKey(ESCAPE).bPressed){
				game.net.StartPause();
				game.racePauseTime=HamsterGame::Game().GetRaceTime();
				Transition(TransitionType::FADE_OUT,PAUSE,0.1f);
			}
			game.UpdateGame(fElapsedTime);
			game.DrawGame();
		}break;
		case PAUSE:{
			if(game.GetKey(ESCAPE).bPressed)Transition(TransitionType::FADE_OUT,GAMEPLAY,0.1f);
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
	ignoreInputs=false;
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
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-32.f},"Grand Prix","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(SHIFT_LEFT,GRAND_PRIX,0.5f);});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,0.f},"Single Race","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(SHIFT_UP,SINGLE_RACE,0.5f);});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,32.f},"Options","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(SHIFT_RIGHT,OPTIONS,0.5f);});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,64.f},"Quit","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(SHIFT_DOWN,QUIT,0.5f);});
		}break;
		case GRAND_PRIX:{
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.pos+vf2d{60.f,HamsterGame::SCREEN_FRAME.size.y/2-36.f},"Grand Prix I (4 courses)","button3.png","highlight_button3.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageI.tmx";
				HamsterGame::Game().mode=HamsterGame::GameMode::GRAND_PRIX_1;
				std::queue<std::string>mapList;
				mapList.emplace("StageII.tmx");
				mapList.emplace("StageIII.tmx");
				mapList.emplace("StageIV.tmx");
				HamsterGame::Game().SetMapSetList(mapList);
				HamsterGame::Game().grandPrixPoints.clear();
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.pos+vf2d{60.f,HamsterGame::SCREEN_FRAME.size.y/2.f},"Grand Prix II (4 courses)","button3.png","highlight_button3.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageV.tmx";
				HamsterGame::Game().mode=HamsterGame::GameMode::GRAND_PRIX_2;
				std::queue<std::string>mapList;
				mapList.emplace("StageVI.tmx");
				mapList.emplace("StageVII.tmx");
				mapList.emplace("StageVIII.tmx");
				HamsterGame::Game().SetMapSetList(mapList);
				HamsterGame::Game().grandPrixPoints.clear();
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.pos+vf2d{60.f,HamsterGame::SCREEN_FRAME.size.y/2+36.f},"Grand Prix III (4 courses)","button3.png","highlight_button3.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageIX.tmx";
				HamsterGame::Game().mode=HamsterGame::GameMode::GRAND_PRIX_3;
				std::queue<std::string>mapList;
				mapList.emplace("StageIV.tmx");
				mapList.emplace("StageVII.tmx");
				mapList.emplace("StageX.tmx");
				HamsterGame::Game().SetMapSetList(mapList);
				HamsterGame::Game().grandPrixPoints.clear();
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.pos+vf2d{60.f,HamsterGame::SCREEN_FRAME.size.y/2+72.f},"Marathon (10 courses)","button3.png","highlight_button3.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageI.tmx";
				HamsterGame::Game().mode=HamsterGame::GameMode::MARATHON;
				std::queue<std::string>mapList;
				mapList.emplace("StageII.tmx");
				mapList.emplace("StageIII.tmx");
				mapList.emplace("StageIV.tmx");
				mapList.emplace("StageV.tmx");
				mapList.emplace("StageVI.tmx");
				mapList.emplace("StageVII.tmx");
				mapList.emplace("StageVIII.tmx");
				mapList.emplace("StageIX.tmx");
				mapList.emplace("StageX.tmx");
				HamsterGame::Game().SetMapSetList(mapList);
				HamsterGame::Game().grandPrixPoints.clear();
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(vf2d{54.f,HamsterGame::SCREEN_FRAME.size.y-24.f},"< Back","smallbutton3.png","highlight_smallbutton3.png",Pixel{145,199,255},Pixel{145,199,255},[this](Button&self){Transition(SHIFT_RIGHT,MAIN_MENU,0.5f);});
		}break;
		case SINGLE_RACE:{
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*1},"I - Welcome to Hamster Planet!","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageI.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*2},"II - Splitting Hairs","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageII.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*3},"III - The Stranger Lands","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageIII.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*4},"IV - Jet Jet Go!","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageIV.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*5},"V - Run Run Run!","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageV.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*6},"VI - A Twisty Maze","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageVI.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*7},"VII - Dunescape","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageVII.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*8},"VIII - Swamps of Travesty","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageVIII.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*9},"IX - Wide Chasm","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageIX.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-132.f+22.f*10},"X - Hamster Island","longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				selectedMap="StageX.tmx";
				Hamster::ClearHamsters();
				Transition(FADE_OUT,LOADING,0.5f);
			});
			buttons.emplace_back(vf2d{54.f,HamsterGame::SCREEN_FRAME.size.y-12.f},"< Back","button4.png","highlight_button4.png",Pixel{220,185,155},Pixel{180,140,152},[this](Button&self){Transition(SHIFT_DOWN,MAIN_MENU,0.5f);});
		}break;
		case PAUSE:{
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-32.f},"Continue","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(FADE_OUT,GAMEPLAY,0.1f);});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,0.f},"Retry","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(FADE_OUT,LOADING,0.5f);});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,32.f},"Main Menu","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){
				HamsterGame::Game().audio.Stop(HamsterGame::Game().bgm.at(HamsterGame::Game().currentMap.value().GetData().GetBGM()));
				Transition(FADE_OUT,MAIN_MENU,0.5f);
			});
		}break;
		case OPTIONS:{
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,-32.f},std::format("BGM: {}",int(round(HamsterGame::Game().bgmVol*100))),"button2.png","highlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				HamsterGame::Game().bgmVol=((int(round(HamsterGame::Game().bgmVol*100))+10)%110)/100.f;
				HamsterGame::Game().audio.SetVolume(HamsterGame::Game().bgm["Trevor Lentz - Guinea Pig Hero.ogg"],HamsterGame::Game().bgmVol);
				self.buttonText=std::format("BGM: {}",int(round(HamsterGame::Game().bgmVol*100)));
				HamsterGame::Game().emscripten_temp_val=std::to_string(HamsterGame::Game().bgmVol);
				#ifdef __EMSCRIPTEN__
					emscripten_idb_async_store("hamster",HamsterGame::Game().bgmVolLabel.c_str(),HamsterGame::Game().emscripten_temp_val.data(),HamsterGame::Game().emscripten_temp_val.length(),0,[](void*args){
						std::cout<<"Success!"<<std::endl;
					},
					[](void*args){
						std::cout<<"Failed"<<std::endl;
					});
				#else
					HamsterGame::Game().SaveOptions();
				#endif
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,0.f},std::format("SFX: {}",int(round(HamsterGame::Game().sfxVol*100))),"button2.png","highlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				HamsterGame::Game().sfxVol=((int(round(HamsterGame::Game().sfxVol*100))+10)%110)/100.f;
				HamsterGame::PlaySFX("wheel_boost.wav");
				self.buttonText=std::format("SFX: {}",int(round(HamsterGame::Game().sfxVol*100)));
				HamsterGame::Game().emscripten_temp_val=std::to_string(HamsterGame::Game().sfxVol);
				#ifdef __EMSCRIPTEN__
					emscripten_idb_async_store("hamster",HamsterGame::Game().sfxVolLabel.c_str(),HamsterGame::Game().emscripten_temp_val.data(),HamsterGame::Game().emscripten_temp_val.length(),0,[](void*args){
						std::cout<<"Success!"<<std::endl;
					},
					[](void*args){
						std::cout<<"Failed"<<std::endl;
					});
				#else
					HamsterGame::Game().SaveOptions();
				#endif
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,32.f},std::format("Player Name: {}",HamsterGame::Game().playerName),"longbutton2.png","longhighlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				HamsterGame::Game().TextEntryEnable(true,HamsterGame::Game().playerName);
			});
			buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{-16.f,64.f},"","smallbutton.png","smallhighlight_button.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				HamsterGame::PlaySFX("select_track_confirm_name_menu.wav");
				int colorInd{0};
				for(int ind{0};const std::string&color:HamsterGame::Game().hamsterColorNames){
					if(color==HamsterGame::Game().hamsterColor){
						colorInd=ind;
						break;
					}
					ind++;
				}
				HamsterGame::Game().hamsterColor=HamsterGame::Game().hamsterColorNames[(colorInd+1)%HamsterGame::Game().hamsterColorNames.size()];
				HamsterGame::Game().emscripten_temp_val=HamsterGame::Game().hamsterColor;
				#ifdef __EMSCRIPTEN__
					emscripten_idb_async_store("hamster",HamsterGame::Game().hamsterColorLabel.c_str(),HamsterGame::Game().emscripten_temp_val.data(),HamsterGame::Game().emscripten_temp_val.length(),0,[](void*args){
						std::cout<<"Success!"<<std::endl;
					},
					[](void*args){
						std::cout<<"Failed"<<std::endl;
					});
				#else
					HamsterGame::Game().SaveOptions();
				#endif
			});
			Button&newButton{buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{16.f,64.f},"","directionalmovement.png","directionalmovement_selected.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){
				HamsterGame::PlaySFX("select_track_confirm_name_menu.wav");
				if(HamsterGame::Game().GetSteeringMode()==HamsterGame::SteeringMode::DIRECTIONAL){
					HamsterGame::Game().SetSteeringMode(HamsterGame::SteeringMode::ROTATIONAL);
					self.buttonImg="rotationalmovement.png";
					self.highlightButtonImg="rotationalmovement_selected.png";
				}else{
					HamsterGame::Game().SetSteeringMode(HamsterGame::SteeringMode::DIRECTIONAL);
					self.buttonImg="directionalmovement.png";
					self.highlightButtonImg="directionalmovement_selected.png";
				}
				#ifdef __EMSCRIPTEN__
					HamsterGame::Game().emscripten_temp_val=std::to_string(int(HamsterGame::Game().GetSteeringMode()));
					emscripten_idb_async_store("hamster",HamsterGame::Game().steeringModeLabel.c_str(),HamsterGame::Game().emscripten_temp_val.data(),HamsterGame::Game().emscripten_temp_val.length(),0,[](void*args){
						std::cout<<"Success!"<<std::endl;
					},
					[](void*args){
						std::cout<<"Failed"<<std::endl;
					});
				#else
					HamsterGame::Game().SaveOptions();
				#endif
			})};
			if(HamsterGame::Game().GetSteeringMode()==HamsterGame::SteeringMode::ROTATIONAL){
				newButton.buttonImg="rotationalmovement.png";
				newButton.highlightButtonImg="rotationalmovement_selected.png";
			}
			buttons.emplace_back(vf2d{54.f,HamsterGame::SCREEN_FRAME.size.y-24.f},"< Back","button2.png","highlight_button2.png",Pixel{114,109,163},Pixel{79,81,128},[this](Button&self){Transition(SHIFT_LEFT,MAIN_MENU,0.5f);});
		}break;
		case GAMEPLAY_RESULTS:{
			int MAX_SIMULATION_COUNT{10000};
			HamsterGame::Game().playerDifferentialTime=0.f;
			while(MAX_SIMULATION_COUNT>0){
				bool allHamstersFinished{true};
				for(Hamster&hamster:Hamster::GetHamsters()){
					if(!hamster.CollectedAllCheckpoints())allHamstersFinished=false;
				}
				if(allHamstersFinished)break;
				HamsterGame::Game().SetElapsedTime(1/30.f);
				HamsterGame::Game().UpdateGame(1/30.f);
				HamsterGame::Game().playerDifferentialTime+=33;
				MAX_SIMULATION_COUNT--;
			}
			HamsterGame::Game().racerList.clear();
			for(size_t ind{0};Hamster&hamster:Hamster::GetHamsters()){
				HamsterGame::Game().racerList.emplace_back(std::pair<HamsterGame::FinishTime,HamsterGame::HamsterInd>{hamster.GetFinishedTime(),ind});
				ind++;
			}
			std::sort(HamsterGame::Game().racerList.begin(),HamsterGame::Game().racerList.end(),[](const std::pair<HamsterGame::FinishTime,HamsterGame::HamsterInd>&hamster1,const std::pair<HamsterGame::FinishTime,HamsterGame::HamsterInd>&hamster2){return hamster1.first<hamster2.first;});
			
			if(HamsterGame::Game().mapSetList.size()>0){
				buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,HamsterGame::SCREEN_FRAME.size.y/2-24.f},"Continue","trackselectbutton.png","highlight_trackselectbutton.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){
					selectedMap=HamsterGame::Game().mapSetList.front();
					HamsterGame::Game().mapSetList.pop();
					Transition(FADE_OUT,LOADING,1.f);
				});
			}else
			if(HamsterGame::Game().GetGameMode()==HamsterGame::GameMode::SINGLE_RACE){
				buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,HamsterGame::SCREEN_FRAME.size.y/2-40.f},"Track Select","trackselectbutton.png","highlight_trackselectbutton.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(FADE_OUT,SINGLE_RACE,1.f);});
				buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,HamsterGame::SCREEN_FRAME.size.y/2-16.f},"Retry","button.png","highlight_button.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(FADE_OUT,LOADING,1.f);});
			}else{
				buttons.emplace_back(HamsterGame::SCREEN_FRAME.size/2+vf2d{0.f,HamsterGame::SCREEN_FRAME.size.y/2-40.f},"Grand Prix Select","trackselectbutton.png","highlight_trackselectbutton.png",Pixel{165,208,96},Pixel{37,134,139},[this](Button&self){Transition(FADE_OUT,GRAND_PRIX,1.f);});
			}
		}break;
	}
	return buttons;
}
void Menu::OnMenuTransition(MenuType previousMenu){
	selectedButton.reset();
	menuButtons.clear();
	newMenuButtons.clear();
	menuButtons=GetMenuButtons(currentMenu);
	if(currentMenu!=GAMEPLAY&&currentMenu!=GAMEPLAY_RESULTS&&currentMenu!=AFTER_RACE_MENU&&currentMenu!=PAUSE)HamsterGame::Game().audio.Play(HamsterGame::Game().bgm["Trevor Lentz - Guinea Pig Hero.ogg"]);
	switch(currentMenu){
		case GAMEPLAY:{
			HamsterGame::Game().racePauseTime.reset();
			if(previousMenu==PAUSE)HamsterGame::Game().net.EndPause();
		}break;
		case GAMEPLAY_RESULTS:{
			const std::vector<int>pointTable{10,7,5,3,2,1};
			for(size_t ind{0};const auto&[finishTime,hamsterInd]:HamsterGame::Game().racerList){
				HamsterGame::Game().grandPrixPoints[hamsterInd]+=pointTable[ind];
				ind++;
			}
			HamsterGame::Game().pointsList.clear();
			for(const auto&[hamsterInd,points]:HamsterGame::Game().grandPrixPoints){
				HamsterGame::Game().pointsList.emplace_back(std::pair<HamsterGame::Points,HamsterGame::HamsterInd>{points,hamsterInd});
			}
			std::sort(HamsterGame::Game().pointsList.begin(),HamsterGame::Game().pointsList.end(),[](const std::pair<HamsterGame::Points,HamsterGame::HamsterInd>&ham1,const std::pair<HamsterGame::Points,HamsterGame::HamsterInd>&ham2){return ham1.first>ham2.first;});
			if(HamsterGame::Game().obtainedNewPB){
				HamsterGame::PlaySFX("new_record.wav");
			}
		}break;
		case LOADING:{
			colorNumb=util::random()%8+1;
			loading=true;
			loadingPct=0.f;
			HamsterGame::Game().operationsProgress=0;
			HamsterGame::Game().LoadRace(selectedMap);
		}break;
		case QUIT:{
			HamsterGame::Game().QuitGame();
		}break;
	}
	if(menuButtons.size()>0)selectedButton=0;
}
void Menu::DrawTransition(HamsterGame&game){
	if(currentTransition==SIMPLE)return;
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
			if(selectedButton.value()<game.mapNameList.size()){
				//Display PB.
				std::string mapName{game.mapNameList[selectedButton.value()]};
				std::string timerStr{"No Record!"};
				game.DrawShadowStringPropDecal({4.f,4.f},"Personal Best",WHITE,BLACK,{1.f,2.f});
				if(game.mapPBs[mapName]!=std::numeric_limits<int>::max())timerStr=util::timerStr(game.mapPBs[mapName]);
				vf2d timerStrSize{game.GetTextSize(timerStr)*vf2d{1.f,2.f}};
				game.DrawShadowRotatedStringDecal({48.f,44.f},timerStr,0.f,timerStrSize/2,WHITE,BLACK,{1.f,2.f});
				if(lastHovered!=selectedButton.value()){
					loadedLeaderboard.clear();
					loadedLeaderboard=game.net.GetLeaderboard(mapName);
					lastHovered=selectedButton.value();
				}
				if(loadedLeaderboard.size()>0){
					auto it{std::find_if(loadedLeaderboard.begin(),loadedLeaderboard.end(),[](const LeaderboardEntry&entry){return entry.time==HamsterGame::mapPBs[entry.map];})};
					std::optional<int>leaderboardInd{};
					if(it!=loadedLeaderboard.end()){
						leaderboardInd=std::distance(loadedLeaderboard.begin(),it);
					}

					game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f,4.f},"Leaderboard",YELLOW,BLACK,{1.f,1.f});
					for(int i=0;i<std::min(size_t(10),loadedLeaderboard.size());i++){
						const LeaderboardEntry&entry{loadedLeaderboard[i]};
						std::string placementStr{std::format("{}.",i+1)};
						vf2d placementStrSize{game.GetTextSizeProp(placementStr)};
						vf2d entryStrSize{game.GetTextSizeProp(entry.name)};

						int totalPixelSpaceForName{int(96-16-placementStrSize.x)};

						vf2d fillRectCorner{vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f,4.f+(i+1)*20.f}-vf2d{2.f,2.f}};

						if(i==leaderboardInd)game.FillRectDecal(fillRectCorner,vf2d{game.ScreenWidth()-fillRectCorner.x-2.f,20.f},{DARK_YELLOW.r,DARK_YELLOW.g,DARK_YELLOW.b,160});

						game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f,4.f+(i+1)*20.f},placementStr,WHITE,BLACK,{1.f,1.f});
						if(entryStrSize.x>totalPixelSpaceForName){
							float nameScaleX{totalPixelSpaceForName/entryStrSize.x};
							game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f+placementStrSize.x,4.f+(i+1)*20.f},std::format("{}",entry.name,util::timerStr(entry.time)),WHITE,BLACK,{nameScaleX,1.f});
						}else game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f+placementStrSize.x,4.f+(i+1)*20.f},std::format("{}",entry.name,util::timerStr(entry.time)),WHITE,BLACK,{1.f,1.f});
						game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f+placementStrSize.x+4.f,4.f+(i+1)*20.f+10.f},std::format("{}",util::timerStr(entry.time)),WHITE,BLACK,{1.f,1.f});
				
						std::string hamsterDisplayImg{game.ColorToHamsterImage(entry.color)};
						game.DrawPartialRotatedDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{96.f-6.f,4.f+(i+1)*20.f+2.f},game.GetGFX(hamsterDisplayImg).Decal(),0.f,{8.f,6.f},{64.f,64.f},{16.f,12.f},{-1.f,1.f});
					}
					
					if(leaderboardInd&&game.mapPBs[mapName]!=std::numeric_limits<int>::max()){
						const LeaderboardEntry&entry{loadedLeaderboard[leaderboardInd.value()]};
						std::string placementStr{std::format("{}.",leaderboardInd.value()+1)};
						vf2d placementStrSize{game.GetTextSizeProp(placementStr)};
						vf2d entryStrSize{game.GetTextSizeProp(entry.name)};

						int totalPixelSpaceForName{int(96-16-placementStrSize.x)};

						vf2d fillRectCorner{vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f,4.f+(12+1)*20.f}-vf2d{2.f,2.f}};
						game.FillRectDecal(fillRectCorner,vf2d{game.ScreenWidth()-fillRectCorner.x-2.f,20.f},{DARK_YELLOW.r,DARK_YELLOW.g,DARK_YELLOW.b,160});

						game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f,4.f+(12+1)*20.f},placementStr,WHITE,BLACK,{1.f,1.f});
						if(entryStrSize.x>totalPixelSpaceForName){
							float nameScaleX{totalPixelSpaceForName/entryStrSize.x};
							game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f+placementStrSize.x,4.f+(12+1)*20.f},std::format("{}",entry.name,util::timerStr(entry.time)),WHITE,BLACK,{nameScaleX,1.f});
						}else game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f+placementStrSize.x,4.f+(12+1)*20.f},std::format("{}",entry.name,util::timerStr(entry.time)),WHITE,BLACK,{1.f,1.f});
						game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{4.f+placementStrSize.x+4.f,4.f+(12+1)*20.f+10.f},std::format("{}",util::timerStr(entry.time)),WHITE,BLACK,{1.f,1.f});
				
						std::string hamsterDisplayImg{game.ColorToHamsterImage(entry.color)};
						game.DrawPartialRotatedDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x,0.f}+vf2d{96.f-6.f,4.f+(12+1)*20.f+2.f},game.GetGFX(hamsterDisplayImg).Decal(),0.f,{8.f,6.f},{64.f,64.f},{16.f,12.f},{-1.f,1.f});
					}
				}
			}
		}break;
		case GAMEPLAY_RESULTS:{
			const std::vector<int>pointTable{10,7,5,3,2,1};
			for(size_t ind{0};const auto&[finishTime,hamsterInd]:HamsterGame::Game().racerList){
				const Hamster&hamster{Hamster::GetHamsters()[hamsterInd]};
				if(hamster.IsPlayerControlled)game.FillRectDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2-64.f,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2-100.f+ind*16}-2.f,{164.f,12.f},{DARK_YELLOW.r,DARK_YELLOW.g,DARK_YELLOW.b,160});
				if(hamster.IsPlayerControlled&&HamsterGame::Game().obtainedNewPB){
					std::string newRecordStr{"NEW RECORD!"};
					vf2d newRecordStrSize{HamsterGame::Game().GetTextSizeProp(newRecordStr)};
					Pixel col{226,228,255};
					if(fmod(HamsterGame::Game().GetRuntime(),0.25f)<0.125f)col=WHITE;
					game.DrawShadowStringPropDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2-64.f-newRecordStrSize.x-4.f,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2-100.f+ind*16},newRecordStr,col);
				}
				game.DrawShadowStringDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2-64.f,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2-100.f+ind*16},std::format("{}.",ind+1));
				game.DrawPartialRotatedDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2-64.f+24.f,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2-100.f+ind*16+6.f},game.GetGFX(hamster.GetHamsterImage()).Decal(),0.f,{8.f,6.f},{64.f,64.f},{16.f,12.f});
				std::string timeStr{util::timerStr(finishTime)};
				vf2d timeStrSize{game.GetTextSize(timeStr)};
				game.DrawShadowStringDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2+64.f-timeStrSize.x,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2-100.f+ind*16},timeStr);
				if(HamsterGame::Game().GetGameMode()!=HamsterGame::GameMode::SINGLE_RACE)game.DrawShadowStringDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2+72.f,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2-100.f+ind*16},std::format("+{}",pointTable[ind]),YELLOW);
				ind++;
			}
			if(HamsterGame::Game().GetGameMode()!=HamsterGame::GameMode::SINGLE_RACE){
				for(size_t ind{0};const auto&[points,hamsterInd]:HamsterGame::Game().pointsList){
					const Hamster&hamster{Hamster::GetHamsters()[hamsterInd]};
					if(hamster.IsPlayerControlled)game.FillRectDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2-64.f,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2+12.f+ind*16}-2.f,{132.f,12.f},{DARK_YELLOW.r,DARK_YELLOW.g,DARK_YELLOW.b,160});
					game.DrawShadowStringDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2-64.f,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2+12.f+ind*16},std::format("{}.",ind+1));
					game.DrawPartialRotatedDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2-64.f+24.f,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2+12.f+ind*16+6.f},game.GetGFX(hamster.GetHamsterImage()).Decal(),0.f,{8.f,6.f},{64.f,64.f},{16.f,12.f});
					std::string pointsStr{std::to_string(points)};
					vf2d pointsStrSize{game.GetTextSize(pointsStr)};
					game.DrawShadowStringDecal(vf2d{game.SCREEN_FRAME.pos.x+game.SCREEN_FRAME.size.x/2+64.f-pointsStrSize.x,game.SCREEN_FRAME.pos.y+game.SCREEN_FRAME.size.y/2+12.f+ind*16},pointsStr,CYAN);
					ind++;
				}
			}
			DrawButtons(pos);
		}break;
		case AFTER_RACE_MENU:{
			DrawButtons(pos);
		}break;
		case OPTIONS:{
			game.DrawPartialDecal(vi2d{pos},game.SCREEN_FRAME.size,game.GetGFX("background2.png").Decal(),vf2d{}+int(game.GetRuntime()*4),game.SCREEN_FRAME.size);
			DrawButtons(pos);
			game.border.Draw();
		}break;
		case PAUSE:{
			game.DrawGame();
			game.FillRectDecal(game.SCREEN_FRAME.pos,game.SCREEN_FRAME.size,{0,0,0,128});
			DrawButtons(pos);
		}break;
		case QUIT:{
			game.DrawPartialDecal(vi2d{pos},game.SCREEN_FRAME.size,game.GetGFX("background3.png").Decal(),vf2d{}+int(game.GetRuntime()*4),game.SCREEN_FRAME.size);
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
	HamsterGame::Game().holdEscTimer=0.f;
	HamsterGame::Game().obtainedNewPB=false;
	HamsterGame::Game().mapImage.Decal()->Update();
	HamsterGame::Game().racePauseTime.reset();

	Powerup::Initialize(HamsterGame::Game().mapPowerupsTemp);
	Checkpoint::Initialize(HamsterGame::Game().checkpointsTemp);

	HamsterGame::Game().audio.SetVolume(HamsterGame::Game().bgm.at(HamsterGame::Game().currentMap.value().GetData().GetBGM()),HamsterGame::Game().bgmVol);
	HamsterGame::Game().audio.Stop(HamsterGame::Game().bgm["Trevor Lentz - Guinea Pig Hero.ogg"]);
	HamsterGame::Game().audio.Play(HamsterGame::Game().bgm.at(HamsterGame::Game().currentMap.value().GetData().GetBGM()),true);

	Hamster::MoveHamstersToSpawn(HamsterGame::Game().currentMap.value().GetData().GetSpawnZone());
	HamsterGame::Game().countdownTimer=3.f;
	
	HamsterGame::Game().camera.SetTarget(Hamster::GetPlayer().GetPos());

	Transition(FADE_OUT,GAMEPLAY,0.5f);
}

void Menu::UpdateLoadingProgress(const float pctLoaded){
	loadingPct=pctLoaded;
}

Menu::Button::Button(const vf2d pos,const std::string&buttonText,const std::string&buttonImg,const std::string&highlightButtonImg,const Pixel textCol,const Pixel highlightTextCol,const std::function<void(Button&self)>onClick)
:pos(pos),buttonText(buttonText),buttonImg(buttonImg),highlightButtonImg(highlightButtonImg),onClick(onClick),textCol(textCol),highlightTextCol(highlightTextCol){}

const bool Menu::Button::IsHovered(const vf2d&offset)const{
	return geom2d::overlaps(HamsterGame::Game().GetMousePos(),geom2d::rect<float>(pos-HamsterGame::GetGFX(buttonImg).Sprite()->Size()/2+offset,HamsterGame::GetGFX(buttonImg).Sprite()->Size()));
}
void Menu::Button::Draw(HamsterGame&game,const vf2d&offset,std::optional<std::reference_wrapper<Button>>selectedButton)const{
	if(selectedButton.has_value()&&&selectedButton.value().get()==this){
		game.DrawRotatedDecal(pos+offset,game.GetGFX(highlightButtonImg).Decal(),0.f,game.GetGFX(highlightButtonImg).Sprite()->Size()/2);
		if(game.IsTextEntryEnabled()&&buttonText.starts_with("Player Name")){
			std::string blinkingCursor{" "};
			if(fmod(game.GetRuntime(),1.f)<0.5f)blinkingCursor="|";
			game.DrawRotatedStringPropDecal(pos+offset,std::format("Player Name: {}{}",game.TextEntryGetString(),blinkingCursor),0.f,game.GetTextSizeProp(buttonText)/2,highlightTextCol);
			std::string helpText{"Press <ENTER> or <ESC> to finish name entry."};
			Pixel helpTextCol{WHITE};
			if(game.menu.badNameEntered){
				helpText="           Invalid name entered!          \nPlease try again, then press <ENTER> or <ESC>.";
				helpTextCol=RED;
			}
			const vf2d helpTextSize{game.GetTextSizeProp(helpText)};
			game.DrawShadowRotatedStringPropDecal(pos+offset+vf2d{0,12.f},helpText,0.f,helpTextSize/2,helpTextCol);
		}else if(buttonImg.starts_with("directionalmovement")){
			std::string helpText{"Hamster moves in direction of pressed key."};
			Pixel helpTextCol{WHITE};
			const vf2d helpTextSize{game.GetTextSizeProp(helpText)};
			game.DrawShadowRotatedStringPropDecal(pos+offset+vf2d{0,12.f},helpText,0.f,helpTextSize/2,helpTextCol);
		}else if(buttonImg.starts_with("rotationalmovement")){
			std::string helpText{"Hamster rotates with LEFT/RIGHT.\nForwards/backwards with UP/DOWN."};
			Pixel helpTextCol{WHITE};
			const vf2d helpTextSize{game.GetTextSizeProp(helpText)};
			game.DrawShadowRotatedStringPropDecal(pos+offset+vf2d{0,12.f},helpText,0.f,helpTextSize/2,helpTextCol);
		}else
		if(buttonImg=="smallbutton.png"){
			int colorInd{0};
			for(int ind{0};const std::string&color:game.hamsterColorNames){
				if(color==game.hamsterColor){
					colorInd=ind;
					break;
				}
				ind++;
			}
			game.DrawPartialRotatedDecal(pos+offset,game.GetGFX(std::format("hamster{}.png",colorInd+1)).Decal(),0.f,{8.f,6.f},{64.f,64.f},{16.f,12.f});
		}else game.DrawRotatedStringPropDecal(pos+offset,buttonText,0.f,game.GetTextSizeProp(buttonText)/2,highlightTextCol);
	}else if(buttonImg=="smallbutton.png"){
		game.DrawRotatedDecal(pos+offset,game.GetGFX(buttonImg).Decal(),0.f,game.GetGFX(buttonImg).Sprite()->Size()/2);
		int colorInd{0};
		for(int ind{0};const std::string&color:game.hamsterColorNames){
			if(color==game.hamsterColor){
				colorInd=ind;
				break;
			}
			ind++;
		}
		game.DrawPartialRotatedDecal(pos+offset,game.GetGFX(std::format("hamster{}.png",colorInd+1)).Decal(),0.f,{8.f,6.f},{64.f,64.f},{16.f,12.f});
	}else{
		game.DrawRotatedDecal(pos+offset,game.GetGFX(buttonImg).Decal(),0.f,game.GetGFX(buttonImg).Sprite()->Size()/2);
		game.DrawRotatedStringPropDecal(pos+offset,buttonText,0.f,game.GetTextSizeProp(buttonText)/2,textCol);
	}
}

void Menu::Button::OnClick(){
	onClick(*this);
}

void Menu::OnTextEntryCancelled(const std::string&text){
	for(Button&b:menuButtons){
		if(b.buttonText.starts_with("Player Name")){
			b.buttonText=std::format("Player Name: {}",HamsterGame::Game().playerName);
		}
	}
	ignoreInputs=true;
	badNameEntered=false;
}

void Menu::OnTextEntryComplete(const std::string&text){
	const bool netResponse{HamsterGame::Game().net.SetName(text.substr(0,30))};
	if(netResponse){
		HamsterGame::Game().playerName=text.substr(0,30);
		HamsterGame::Game().emscripten_temp_val=HamsterGame::Game().playerName;
		#ifdef __EMSCRIPTEN__
			emscripten_idb_async_store("hamster",HamsterGame::Game().playerNameLabel.c_str(),HamsterGame::Game().emscripten_temp_val.data(),HamsterGame::Game().emscripten_temp_val.length(),0,[](void*args){
				
				// set playerName in localStorage as well
				EM_ASM({
					window.localStorage.setItem("playerName", UTF8ToString($0));
				}, HamsterGame::Game().playerName.c_str());
				
				std::cout<<"Success!"<<std::endl;
			},
			[](void*args){
				std::cout<<"Failed"<<std::endl;
			});
		#else
			HamsterGame::Game().SaveOptions();
		#endif
		for(Button&b:menuButtons){
			if(b.buttonText.starts_with("Player Name")){
				b.buttonText=std::format("Player Name: {}",HamsterGame::Game().playerName);
			}
		}
		ignoreInputs=true;
		HamsterGame::PlaySFX("menu_set_name.wav");
		badNameEntered=false;
	}else{
		badNameEntered=true;
		HamsterGame::Game().TextEntryEnable(true,text.substr(0,30));
	}
}


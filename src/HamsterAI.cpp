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
#include "HamsterAI.h"
#include "HamsterGame.h"
#include <ranges>

bool HamsterAI::recordingMode{false};
std::optional<vi2d>HamsterAI::lastTileWalkedOn;
std::vector<HamsterAI::Action>HamsterAI::recordedActions;

void HamsterAI::Update(const float fElapsedTime){
	if(HamsterGame::Game().GetKey(HOME).bPressed){
		if(!recordingMode){
			recordingMode=true;
			recordedActions.clear();
		}else{
			HamsterGame::Game().TextEntryEnable(true,"0");
		}
	}
}

void HamsterAI::DrawOverlay(){
	if(recordingMode){
		if(HamsterGame::Game().IsTextEntryEnabled()){
			std::string displayStr{std::format("{}\n{}","0=SMART  1=NORMAL  2=DUMB",HamsterGame::Game().TextEntryGetString())};
			for(int y:std::ranges::iota_view(-1,2)){
				for(int x:std::ranges::iota_view(-1,2)){
					HamsterGame::Game().DrawStringDecal({2.f+x,2.f+y},displayStr,BLACK);
				}
			}
			HamsterGame::Game().DrawStringDecal({2.f,2.f},displayStr,YELLOW);
		}else if(fmod(HamsterGame::Game().GetRuntime(),1.f)<0.5f){
			for(int y:std::ranges::iota_view(-1,2)){
				for(int x:std::ranges::iota_view(-1,2)){
					HamsterGame::Game().DrawStringDecal({2.f+x,2.f+y},"RECORDING",BLACK);
				}
			}
			HamsterGame::Game().DrawStringDecal({2.f,2.f},"RECORDING",RED);
		}
	}
}
void HamsterAI::OnTextEntryComplete(const std::string&enteredText){
	if(recordingMode){
		std::ofstream file{std::format("{}{}.{}",HamsterGame::ASSETS_DIR,HamsterGame::Game().GetCurrentMapName(),stoi(enteredText))};
		for(const Action&action:recordedActions){
			file<<action.pos.x<<' '<<action.pos.y<<' '<<int(action.type)<<' ';
		}
		file.close();
		recordedActions.clear();
		recordingMode=false;
	}
}

const HamsterAI::ActionOptRef HamsterAI::GetCurrentAction(){
	if(actionInd<actionsToPerform.size())return ActionOptRef{actionsToPerform.at(actionInd)};
	return {};
}
const HamsterAI::ActionOptRef HamsterAI::AdvanceToNextAction(){
	actionInd++;
	return GetCurrentAction();
}

void HamsterAI::OnMove(const vi2d pos){
	const vi2d tilePos{vi2d{pos/16}*16};
	if(!lastTileWalkedOn.has_value()){
		recordedActions.emplace_back(pos,Action::MOVE);
	}else if(lastTileWalkedOn.value()!=tilePos){
		recordedActions.emplace_back(pos,Action::MOVE);
	}
	lastTileWalkedOn=tilePos;
}
void HamsterAI::OnPowerupCollection(const vi2d pos){
	recordedActions.emplace_back(pos,Action::COLLECT_POWERUP);
}
void HamsterAI::OnJetLaunch(const vi2d pos){
	recordedActions.emplace_back(pos,Action::LAUNCH_JET);
}
void HamsterAI::OnTunnelEnter(const vi2d pos){
	recordedActions.emplace_back(pos,Action::ENTER_TUNNEL);
}
void HamsterAI::OnCheckpointCollected(const vi2d pos){
	recordedActions.emplace_back(pos,Action::CHECKPOINT_COLLECTED);
}

void HamsterAI::LoadAI(const std::string&mapName,int numb){
	std::ifstream file{std::format("{}{}.{}",HamsterGame::ASSETS_DIR,HamsterGame::Game().GetCurrentMapName(),numb)};
	actionsToPerform.clear();
	while(file.good()){
		Action newAction;
		file>>newAction.pos.x;
		file>>newAction.pos.y;
		int typeNum;
		file>>typeNum;
		newAction.type=HamsterAI::Action::ActionType(typeNum);
		actionsToPerform.emplace_back(newAction);
	}
	this->type=AIType(numb%3);
	file.close();
}

const HamsterAI::AIType HamsterAI::GetAIType()const{
	return type;
}

void HamsterAI::OnJetBeginLanding(const vi2d pos){
	recordedActions.emplace_back(pos,Action::LANDING);
}


const HamsterAI::ActionOptRef HamsterAI::GetPreviousAction(){
	if(actionInd-1>0)return actionsToPerform[actionInd-1];
	return {};
}
const HamsterAI::ActionOptRef HamsterAI::RevertToPreviousAction(){
	if(actionInd-1>0)actionInd--;
	return GetCurrentAction();
}

const HamsterAI::ActionOptRef HamsterAI::PeekNextAction(){
	if(actionInd+1<actionsToPerform.size())return actionsToPerform[actionInd+1];
	return {};
}

void HamsterAI::OnBoost(const vi2d pos){
	recordedActions.emplace_back(pos,Action::BOOST);
}
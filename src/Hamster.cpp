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

#include "HamsterGame.h"
#include "Hamster.h"
#include "util.h"
#include <ranges>
#include "AnimationState.h"
#include "FloatingText.h"
#include "HamsterAI.h"

std::vector<Hamster>Hamster::HAMSTER_LIST;
const uint8_t Hamster::MAX_HAMSTER_COUNT{100U};
const uint8_t Hamster::NPC_HAMSTER_COUNT{5U};
const std::vector<std::string>Hamster::NPC_HAMSTER_IMAGES{
	"hamster.png",
};
const std::string Hamster::PLAYER_HAMSTER_IMAGE{"hamster.png"};
std::optional<Hamster*>Hamster::playerHamster;

Hamster::Hamster(const vf2d spawnPos,const std::string&img,const PlayerControlled IsPlayerControlled)
:pos(spawnPos),IsPlayerControlled(IsPlayerControlled),randomId(util::random()){
	animations=HamsterGame::GetAnimations(img);
	animations.ChangeState(internalAnimState,AnimationState::DEFAULT);
}

void Hamster::UpdateHamsters(const float fElapsedTime){
	for(Hamster&h:HAMSTER_LIST){
		h.lastSafeLocationTimer=std::max(0.f,h.lastSafeLocationTimer-fElapsedTime);
		h.animations.UpdateState(h.internalAnimState,fElapsedTime);
		h.aiNodeTime+=fElapsedTime;
		h.frictionEnabled=true;
		h.bumpTimer-=fElapsedTime;
		h.boostTimer=std::max(0.f,h.boostTimer-fElapsedTime);
		h.canCollectWheelPowerupTimer=std::max(0.f,h.canCollectWheelPowerupTimer-fElapsedTime);
		h.HandleCollision();
		switch(h.state){
			case NORMAL:{
				if(h.CanMove()){
					if(h.IsPlayerControlled){
						h.HandlePlayerControls();
					}else{
						h.HandleAIControls();
					}
				}
			}break;
			case BUMPED:{
				if(h.bumpTimer<=0.f){
					h.SetState(NORMAL);
				}
			}break;
			case DROWNING:{
				if(h.imgScale>0.f){
					h.shrinkEffectColor=BLACK;
					h.imgScale=std::max(0.f,h.imgScale-0.5f*fElapsedTime);
				}
				else{
					h.waitTimer=4.f;
					h.SetState(WAIT);
				}
			}break;
			case WAIT:{
				h.waitTimer=std::max(0.f,h.waitTimer-fElapsedTime);
				if(h.waitTimer<=0.f){
					h.imgScale=1.f;
					h.drownTimer=0.f;
					if(!h.lastSafeLocation.has_value()){
						h.lastSafeLocation=h.GetNearestSafeLocation();
					}
					h.SetPos(h.lastSafeLocation.value());
					h.SetState(NORMAL);
					h.RemoveAllPowerups();
				}
			}break;
			case BURNING:{
				if(h.imgScale>0.f){
					h.shrinkEffectColor=RED;
					h.imgScale=std::max(0.f,h.imgScale-0.5f*fElapsedTime);
				}else{
					h.waitTimer=4.f;
					h.SetState(WAIT);
				}
			}break;
			case KNOCKOUT:{
				h.knockoutTimer-=fElapsedTime;
				if(h.knockoutTimer<=0.f){
					h.SetState(NORMAL);
					h.animations.ChangeState(h.internalAnimState,AnimationState::DEFAULT);
				}
			}break;
			case BURROWING:{
				h.burrowTimer-=fElapsedTime;
				h.burrowImgShrinkTimer-=fElapsedTime;
				h.shrinkEffectColor=BLACK;
				h.imgScale=std::max(0.f,util::lerp(0,1,h.burrowImgShrinkTimer*2.f));
				if(h.burrowTimer<=0.f){
					h.burrowTimer=3.f;
					h.SetState(BURROW_WAIT);
				}
			}break;
			case BURROW_WAIT:{
				h.burrowTimer-=fElapsedTime;
				const Tunnel&enteredTunnel{HamsterGame::Game().GetTunnels().at(h.enteredTunnel)};
				const vf2d destinationTunnelPos{HamsterGame::Game().GetTunnels().at(enteredTunnel.linkedTo).worldPos+vi2d{8,8}};
				if(h.burrowTimer<=0.f){
					h.burrowTimer=1.f;
					h.SetState(SURFACING);
					h.imgScale=0.f;
					h.burrowImgShrinkTimer=0.5f;
					switch(HamsterGame::Game().GetTileFacingDirection(destinationTunnelPos)){
						case Terrain::NORTH:{
							h.targetRot=-geom2d::pi/2;
						}break;
						case Terrain::EAST:{
							h.targetRot=0.f;
						}break;
						case Terrain::SOUTH:{
							h.targetRot=geom2d::pi/2;
						}break;
						case Terrain::WEST:{
							h.targetRot=geom2d::pi;
						}break;
					}
				}
				h.pos=destinationTunnelPos.lerp(enteredTunnel.worldPos+vi2d{8,8},h.burrowTimer/3.f);
			}break;
			case SURFACING:{
				h.burrowTimer-=fElapsedTime;
				h.burrowImgShrinkTimer-=fElapsedTime;
				h.imgScale=std::min(1.f,util::lerp(1,0,h.burrowImgShrinkTimer*2.f));
				vf2d targetDirVec{0.f,-16.f};
				const Tunnel&enteredTunnel{HamsterGame::Game().GetTunnels().at(h.enteredTunnel)};
				const vf2d destinationTunnelPos{HamsterGame::Game().GetTunnels().at(enteredTunnel.linkedTo).worldPos+vi2d{8,8}};
				switch(HamsterGame::Game().GetTileFacingDirection(destinationTunnelPos)){
					case Terrain::EAST:{
						targetDirVec={16.f,0.f};
					}break;
					case Terrain::SOUTH:{
						targetDirVec={0.f,16.f};
					}break;
					case Terrain::WEST:{
						targetDirVec={-16.f,0.f};
					}break;
				}
				const vf2d walkOutTunnelDest{destinationTunnelPos+targetDirVec};
				h.pos=walkOutTunnelDest.lerp(destinationTunnelPos,h.burrowTimer);
				if(h.burrowTimer<=0.f){
					h.SetState(Hamster::NORMAL);
					h.imgScale=1.f;
				}
			}break;
		}
		if(h.state!=FLYING){
			if((h.GetTerrainStandingOn()==Terrain::OCEAN||h.GetTerrainStandingOn()==Terrain::VOID||!h.HasPowerup(Powerup::SWAMP)&&h.GetTerrainStandingOn()==Terrain::SWAMP)&&h.state!=DROWNING&&h.state!=WAIT)h.drownTimer+=fElapsedTime;
			else if((!h.HasPowerup(Powerup::LAVA)&&h.GetTerrainStandingOn()==Terrain::LAVA)&&h.state!=BURNING&&h.state!=WAIT)h.burnTimer+=fElapsedTime;
			else if(h.lastSafeLocationTimer<=0.f&&h.state==NORMAL&&!h.IsLethalTerrain(h.GetPos())){
				h.lastSafeLocationTimer=0.5f;
				h.drownTimer=0.f;
				h.burnTimer=0.f;
				h.lastSafeLocation=h.GetPos();
			}
			if(h.drownTimer>=h.DEFAULT_DROWN_TIME&&h.state!=DROWNING&&h.state!=WAIT){
				h.SetState(DROWNING);
			}
			if(h.burnTimer>=h.DEFAULT_BURN_TIME&&h.state!=BURNING&&h.state!=WAIT){
				h.SetState(BURNING);
			}
		}
		if(h.hamsterJet.has_value())h.hamsterJet.value().Update(fElapsedTime);
		h.TurnTowardsTargetDirection();
		h.MoveHamster();
		if(h.IsPlayerControlled){
			h.readyFlashTimer+=fElapsedTime;
			h.jetFuelDisplayAmt+=(h.jetFuel-h.jetFuelDisplayAmt)*4.f*fElapsedTime;
		}
		if(h.CollectedAllCheckpoints()){
			h.animations.ChangeState(h.internalAnimState,AnimationState::SIDE_VIEW);
			h.raceFinishAnimTimer+=fElapsedTime;
		}
	}
}

void Hamster::LoadHamsters(const geom2d::rect<int>startingLoc){
	HAMSTER_LIST.clear();
	playerHamster.reset();
	HAMSTER_LIST.reserve(MAX_HAMSTER_COUNT);
	if(NPC_HAMSTER_COUNT+1>MAX_HAMSTER_COUNT)throw std::runtime_error{std::format("WARNING! Max hamster count is too high! Please expand the MAX_HAMSTER_COUNT if you want more hamsters. Requested {} hamsters.",MAX_HAMSTER_COUNT)};
	playerHamster=&HAMSTER_LIST.emplace_back(vf2d{util::random_range(startingLoc.pos.x,startingLoc.pos.x+startingLoc.size.x),util::random_range(startingLoc.pos.y,startingLoc.pos.y+startingLoc.size.y)},PLAYER_HAMSTER_IMAGE,PLAYER_CONTROLLED);
	for(int i:std::ranges::iota_view(0U,NPC_HAMSTER_COUNT)){
		Hamster&npcHamster{HAMSTER_LIST.emplace_back(vf2d{util::random_range(startingLoc.pos.x,startingLoc.pos.x+startingLoc.size.x),util::random_range(startingLoc.pos.y,startingLoc.pos.y+startingLoc.size.y)},NPC_HAMSTER_IMAGES.at(util::random()%NPC_HAMSTER_IMAGES.size()),NPC)};
		int MAX_AI_FILES{100};
		int aiFileCount{0};
		while(MAX_AI_FILES>0){
			if(!std::filesystem::exists(std::format("{}{}.{}",HamsterGame::ASSETS_DIR,HamsterGame::Game().GetCurrentMapName(),aiFileCount)))break;
			aiFileCount++;
			MAX_AI_FILES--;
		}
		HamsterAI::AIType typeChosen{HamsterAI::DUMB};
		int randPct{util::random()%100};
		switch(HamsterGame::Game().GetMapDifficulty()){
			case Difficulty::EASY:{
				if(randPct<=0.4f&&randPct>0.2f)typeChosen=HamsterAI::NORMAL;
				else if(randPct<=0.2f)typeChosen=HamsterAI::SMART;
			}break;
			case Difficulty::MEDIUM:{
				typeChosen=HamsterAI::NORMAL;
				if(randPct<=1.f&&randPct>0.8f)typeChosen=HamsterAI::DUMB;
				else if(randPct<=0.2f)typeChosen=HamsterAI::SMART;
			}break;
			case Difficulty::HARD:{
				typeChosen=HamsterAI::SMART;
				if(randPct<=0.2f)typeChosen=HamsterAI::NORMAL;
			}break;
		}

		std::vector<int>possibleAIs{};
		for(int i:std::ranges::iota_view(0,aiFileCount)){
			if(i%3==int(typeChosen))possibleAIs.emplace_back(i);
		}

		if(possibleAIs.size()==0){
			std::cout<<"WARNING! No AI files for AI Type "<<int(typeChosen)<<" currently exist! Consider adding them! Rolling with whatever exists..."<<std::endl;
			for(int i:std::ranges::iota_view(0,aiFileCount)){
				possibleAIs.emplace_back(i);
			}
		}

		if(possibleAIs.size()>0)npcHamster.ai.LoadAI(HamsterGame::Game().GetCurrentMapName(),possibleAIs[util::random()%possibleAIs.size()]);
	}
}

void Hamster::DrawHamsters(TransformedView&tv){
	for(Hamster&h:HAMSTER_LIST){
		const Animate2D::FrameSequence&anim{h.animations.GetFrames(h.internalAnimState)};
		const Animate2D::FrameSequence&wheelTopAnim{h.animations.GetFrames(AnimationState::WHEEL_TOP)};
		const Animate2D::FrameSequence&wheelBottomAnim{h.animations.GetFrames(AnimationState::WHEEL_BOTTOM)};
		const Animate2D::Frame&img{h.animations.GetState(h.internalAnimState)==AnimationState::DEFAULT?anim.GetFrame(h.distanceTravelled/100.f):h.GetCurrentAnimation()};
		const Animate2D::Frame&wheelTopImg{wheelTopAnim.GetFrame(h.distanceTravelled/80.f)};
		const Animate2D::Frame&wheelBottomImg{wheelBottomAnim.GetFrame(h.distanceTravelled/80.f)};
		if(h.CollectedAllCheckpoints()){
			float animCycle{fmod(h.raceFinishAnimTimer,1.5f)};
			float facingXScale{fmod(h.raceFinishAnimTimer,3.f)>=1.5f?-1.f:1.f};
			float yHopAmt{0.f};
			if(animCycle>=0.8f){
				yHopAmt=-abs(sin(geom2d::pi*(animCycle/0.35f)))*12.f;
			}
			Pixel blendCol{PixelLerp(h.shrinkEffectColor,WHITE,h.imgScale)};
			if(h.boostTimer!=0.f&&fmod(HamsterGame::Game().GetRuntime(),0.5f)<0.25f)blendCol=RED;
			if(h.hamsterJet.has_value())h.hamsterJet.value().Draw(blendCol);
			HamsterGame::Game().SetZ(h.z+0.02f);
			tv.DrawRotatedDecal(h.pos+vf2d{0.f,h.drawingOffsetY},HamsterGame::GetGFX("shadow.png").Decal(),0.f,HamsterGame::GetGFX("shadow.png").Sprite()->Size()/2);
			HamsterGame::Game().SetZ(h.z+0.025f);
			tv.DrawPartialRotatedDecal(h.pos+vf2d{0.f,h.drawingOffsetY+yHopAmt},img.GetSourceImage()->Decal(),0.f,img.GetSourceRect().size/2,img.GetSourceRect().pos,img.GetSourceRect().size,vf2d{1.f,1.f}*h.imgScale*vf2d{facingXScale,1.f},blendCol);
			HamsterGame::Game().SetZ(0.f);
		}else{
			Pixel blendCol{PixelLerp(h.shrinkEffectColor,WHITE,h.imgScale)};
			if(h.boostTimer!=0.f&&fmod(HamsterGame::Game().GetRuntime(),0.5f)<0.25f)blendCol=RED;
			if(h.hamsterJet.has_value())h.hamsterJet.value().Draw(blendCol);
			HamsterGame::Game().SetZ(h.z+0.02f);
			if(h.HasPowerup(Powerup::WHEEL))tv.DrawPartialRotatedDecal(h.pos+vf2d{0.f,h.drawingOffsetY},wheelBottomImg.GetSourceImage()->Decal(),h.rot,wheelBottomImg.GetSourceRect().size/2,wheelBottomImg.GetSourceRect().pos,wheelBottomImg.GetSourceRect().size,vf2d{1.f,1.f}*h.imgScale,PixelLerp(h.shrinkEffectColor,blendCol,h.imgScale));
			tv.DrawPartialRotatedDecal(h.pos+vf2d{0.f,h.drawingOffsetY},img.GetSourceImage()->Decal(),h.rot,img.GetSourceRect().size/2,img.GetSourceRect().pos,img.GetSourceRect().size,vf2d{1.f,1.f}*h.imgScale,PixelLerp(h.shrinkEffectColor,blendCol,h.imgScale));
			HamsterGame::Game().SetZ(h.z+0.025f);
			if(h.HasPowerup(Powerup::WHEEL))tv.DrawPartialRotatedDecal(h.pos+vf2d{0.f,h.drawingOffsetY},wheelTopImg.GetSourceImage()->Decal(),h.rot,wheelTopImg.GetSourceRect().size/2,wheelTopImg.GetSourceRect().pos,wheelTopImg.GetSourceRect().size,vf2d{1.f,1.f}*h.imgScale,PixelLerp(h.shrinkEffectColor,{blendCol.r,blendCol.g,blendCol.b,192},h.imgScale));
			HamsterGame::Game().SetZ(0.f);
		}
	}
}

void Hamster::DrawOverlay(){
	if(GetPlayer().hamsterJet.has_value())GetPlayer().hamsterJet.value().DrawOverlay();
	
	const vf2d jetDisplayOffset{HamsterGame::SCREEN_FRAME.pos+vf2d{HamsterGame::SCREEN_FRAME.size.x,0.f}};
	Pixel jetDisplayCol{VERY_DARK_GREY};
	if(!GetPlayer().hamsterJet.has_value()){
		if(GetPlayer().HasPowerup(Powerup::JET))jetDisplayCol=WHITE;
		const Animate2D::FrameSequence&lightAnim{HamsterGame::Game().GetAnimation("hamster_jet.png",AnimationState::JET_LIGHTS)};
		const Animate2D::Frame&lightFrame{lightAnim.GetFrame(HamsterGame::Game().GetRuntime())};
		const Animate2D::FrameSequence&jetAnim{HamsterGame::Game().GetAnimation("hamster_jet.png",AnimationState::JET)};
		const Animate2D::Frame&jetFrame{jetAnim.GetFrame(HamsterGame::Game().GetRuntime())};
		HamsterGame::Game().DrawPartialRotatedDecal(jetDisplayOffset+vf2d{48.f,80.f},jetFrame.GetSourceImage()->Decal(),0.f,jetFrame.GetSourceRect().size/2,jetFrame.GetSourceRect().pos,jetFrame.GetSourceRect().size,{2.f,2.f},jetDisplayCol);
		HamsterGame::Game().DrawPartialRotatedDecal(jetDisplayOffset+vf2d{48.f,80.f},lightFrame.GetSourceImage()->Decal(),0.f,lightFrame.GetSourceRect().size/2,lightFrame.GetSourceRect().pos,jetFrame.GetSourceRect().size,{2.f,2.f},jetDisplayCol);
	}

	if(GetPlayer().HasPowerup(Powerup::JET)&&!GetPlayer().hamsterJet.has_value()){
		const std::string readyText{"READY!"};
		const vi2d textSize{HamsterGame::Game().GetTextSize(readyText)};
		for(int y:std::ranges::iota_view(-1,2)){
			for(int x:std::ranges::iota_view(-1,2)){
				if(x==0&&y==0)continue;
				HamsterGame::Game().DrawRotatedStringDecal(jetDisplayOffset+vf2d{48.f,116.f}+vi2d{x,y},readyText,0.f,textSize/2,fmod(GetPlayer().readyFlashTimer,1.5f)<=0.75f?DARK_RED:BLACK);
			}
		}
		HamsterGame::Game().DrawRotatedStringDecal(jetDisplayOffset+vf2d{48.f,116.f},readyText,0.f,textSize/2,GREEN);
		HamsterGame::Game().DrawDecal(HamsterGame::SCREEN_FRAME.pos+vf2d{HamsterGame::SCREEN_FRAME.size.x,0.f},HamsterGame::GetGFX("fuelmeter.png").Decal());
		const std::string launchText{"(SPACE)x2\nto Launch!"};
		const vi2d launchTextSize{HamsterGame::Game().GetTextSize(launchText)};
		for(int y:std::ranges::iota_view(-1,2)){
			for(int x:std::ranges::iota_view(-1,2)){
				if(x==0&&y==0)continue;
				HamsterGame::Game().DrawRotatedStringDecal(jetDisplayOffset+vf2d{48.f,224.f}+vi2d{x,y},launchText,0.f,launchTextSize/2,BLACK);
			}
		}
		HamsterGame::Game().DrawRotatedStringDecal(jetDisplayOffset+vf2d{48.f,224.f},launchText,0.f,launchTextSize/2,WHITE);
	}else{
		HamsterGame::Game().DrawPartialDecal(HamsterGame::SCREEN_FRAME.pos+vf2d{HamsterGame::SCREEN_FRAME.size.x,0.f},HamsterGame::GetGFX("fuelmeter.png").Decal(),{0,0},{96,200});
	}
	const float jetFuelBarHeight{float(HamsterGame::GetGFX("fuelbar.png").Sprite()->height)};
	HamsterGame::Game().DrawPartialDecal(jetDisplayOffset+vf2d{24.f,139.f}+vf2d{0.f,jetFuelBarHeight*(1.f-GetPlayer().jetFuelDisplayAmt)},HamsterGame::GetGFX("fuelbar.png").Decal(),{0.f,jetFuelBarHeight*(1.f-GetPlayer().jetFuelDisplayAmt)},{float(HamsterGame::GetGFX("fuelbar.png").Sprite()->width),jetFuelBarHeight*(GetPlayer().jetFuelDisplayAmt)});
	if(GetPlayer().hamsterJet.has_value()){
		const Terrain::FuelDamage jetFuelLandingCost{std::min(GetPlayer().jetFuelDisplayAmt,Terrain::GetFuelDamageTakenAndKnockoutEffect(GetPlayer().GetTerrainHoveringOver(),GetPlayer().hamsterJet.value().GetLandingSpeed()).first)};
		HamsterGame::Game().DrawPartialDecal(jetDisplayOffset+vf2d{24.f,139.f}+vf2d{0.f,jetFuelBarHeight*(1.f-GetPlayer().jetFuelDisplayAmt)},HamsterGame::GetGFX("fuelbar.png").Decal(),{0.f,jetFuelBarHeight*(1.f-GetPlayer().jetFuelDisplayAmt)},{float(HamsterGame::GetGFX("fuelbar.png").Sprite()->width),jetFuelBarHeight*jetFuelLandingCost},{1.f,1.f},{255,0,0,192});
	}
	if(GetPlayer().HasPowerup(Powerup::JET))HamsterGame::Game().DrawDecal(jetDisplayOffset+vf2d{22.f,137.f},HamsterGame::GetGFX("fuelbar_outline.png").Decal(),{1.f,1.f},GetPlayer().jetFuel<=0.2f?(fmod(GetPlayer().readyFlashTimer,1.f)<=0.5f?RED:BLACK):BLACK);
	if(GetPlayer().HasPowerup(Powerup::WHEEL)){
		for(int i:std::ranges::iota_view(0,3)){
			if(fmod(HamsterGame::Game().GetRuntime(),2.f)<1.f&&GetPlayer().boostCounter>i)HamsterGame::Game().DrawDecal(HamsterGame::SCREEN_FRAME.pos+vf2d{i*16.f+4.f,HamsterGame::SCREEN_FRAME.size.y-20.f},HamsterGame::GetGFX("boost_outline.png").Decal(),{0.125f,0.125f},GetPlayer().boostCounter>i?WHITE:BLACK);
			else HamsterGame::Game().DrawDecal(HamsterGame::SCREEN_FRAME.pos+vf2d{i*16.f+4.f,HamsterGame::SCREEN_FRAME.size.y-20.f},HamsterGame::GetGFX("boost.png").Decal(),{0.125f,0.125f},GetPlayer().boostCounter>i?WHITE:BLACK);
		}
		for(int y:std::ranges::iota_view(-1,2)){
			for(int x:std::ranges::iota_view(-1,2)){
				HamsterGame::Game().DrawStringDecal(HamsterGame::SCREEN_FRAME.pos+vf2d{3*16.f+8.f,HamsterGame::SCREEN_FRAME.size.y-12.f}+vi2d{x,y},"\"R\" - BOOST",fmod(HamsterGame::Game().GetRuntime(),2.f)<1.f?RED:BLACK);
			}
		}
		HamsterGame::Game().DrawStringDecal(HamsterGame::SCREEN_FRAME.pos+vf2d{3*16.f+8.f,HamsterGame::SCREEN_FRAME.size.y-12.f},"\"R\" - BOOST",YELLOW);
	}
}

const Animate2D::Frame&Hamster::GetCurrentAnimation()const{
	return animations.GetFrame(internalAnimState);
}

const Hamster&Hamster::GetPlayer(){
	if(!playerHamster.has_value())throw std::runtime_error{std::format("WARNING! Player is not created at this time! There should not be any code referencing the player at this moment!")};
	return *playerHamster.value();
}

const vf2d&Hamster::GetPos()const{
	return pos;
}

void Hamster::HandlePlayerControls(){
	lastTappedSpace+=HamsterGame::Game().GetElapsedTime();
	vf2d aimingDir{};
	if(HamsterGame::Game().GetKey(W).bHeld){
		aimingDir+=vf2d{0,-1};
	}
	if(HamsterGame::Game().GetKey(D).bHeld){
		aimingDir+=vf2d{1,0};
	}
	if(HamsterGame::Game().GetKey(S).bHeld){
		aimingDir+=vf2d{0,1};
	}
	if(HamsterGame::Game().GetKey(A).bHeld){
		aimingDir+=vf2d{-1,0};
	}
	if(aimingDir!=vf2d{}){
		targetRot=aimingDir.norm().polar().y;
		const vf2d currentVel{vel};
		vel=vf2d{currentVel.polar().x+((GetMaxSpeed()/GetTimeToMaxSpeed())*HamsterGame::Game().GetElapsedTime()),rot}.cart();
		vel=vf2d{std::min(GetMaxSpeed(),vel.polar().x),vel.polar().y}.cart();
		frictionEnabled=false;
	}
	if(HamsterGame::Game().GetKey(R).bPressed&&boostCounter>0){
		boostCounter--;
		boostTimer=1.f;
	}
	if(HamsterGame::Game().GetKey(SPACE).bPressed){
		if(lastTappedSpace<=0.6f&&HasPowerup(Powerup::JET)){
			SetState(FLYING);
			lastSafeLocation.reset();
			if(IsPlayerControlled)HamsterAI::OnJetLaunch(this->pos);
			hamsterJet.emplace(*this);
		}
		lastTappedSpace=0.f;
	}
	if(HamsterGame::Game().GetKey(P).bPressed){
		ObtainPowerup(Powerup::JET);
		Powerup tempJetPowerup{{},Powerup::JET};
		tempJetPowerup.OnPowerupObtain(*this);
	}
}

void Hamster::TurnTowardsTargetDirection(){
	util::turn_towards_direction(rot,targetRot,GetTurnSpeed()*HamsterGame::Game().GetElapsedTime());
}

void Hamster::MoveHamster(){
	if(IsBurrowed())return;
	SetPos(GetPos()+vel*HamsterGame::Game().GetElapsedTime());
	
	distanceTravelled+=vel.mag()*HamsterGame::Game().GetElapsedTime();
	if(state==FLYING){
		jetFuel=std::max(0.f,jetFuel-vel.mag()*HamsterGame::Game().GetElapsedTime()/100000.f);
	}

	#pragma region Handle Friction
		if(frictionEnabled){
			const vf2d currentVel{vel};
			vel=vf2d{std::max(0.f,currentVel.polar().x-GetFriction()*HamsterGame::Game().GetElapsedTime()),currentVel.polar().y}.cart();
		}
	#pragma endregion
	if(hamsterJet.has_value()&&hamsterJet.value().GetState()==HamsterJet::HAMSTER_CONTROL)hamsterJet.value().SetPos(GetPos()); //Hamster Jet lagging behind hack fix.
}

void Hamster::HandleCollision(){
	if(IsBurrowed())return;
	for(Hamster&h:HAMSTER_LIST){
		if(this==&h)continue;
		if(h.IsBurrowed())continue;
		if(abs(z-h.z)<=0.1f&&geom2d::overlaps(geom2d::circle<float>(GetPos(),GetRadius()),geom2d::circle<float>(h.GetPos(),h.GetRadius()))){
			if(geom2d::line<float>(GetPos(),h.GetPos()).length()==0.f){ //Push these two in random directions, they are on top of each other!
				float randDir{util::random(2*geom2d::pi)};
				vf2d collisionResolve1{GetPos()+vf2d{GetRadius(),randDir}.cart()};
				vf2d collisionResolve2{h.GetPos()+vf2d{h.GetRadius(),float(randDir+geom2d::pi)}.cart()};
				SetPos(collisionResolve1);
				h.SetPos(collisionResolve2);
				vel+=vf2d{GetBumpAmount(),randDir}.cart();
				vel=vf2d{std::min(GetMaxSpeed(),vel.polar().x),vel.polar().y}.cart();
				h.vel+=vf2d{h.GetBumpAmount(),float(randDir+geom2d::pi)}.cart();
				h.vel=vf2d{std::min(h.GetMaxSpeed(),h.vel.polar().x),h.vel.polar().y}.cart();
			}else{
				geom2d::line<float>collisionLine{geom2d::line<float>(GetPos(),h.GetPos())};
				float distance{collisionLine.length()};
				float totalRadii{GetRadius()+h.GetRadius()};
				float bumpDistance{totalRadii-distance};
				vf2d collisionResolve1{GetPos()+vf2d{bumpDistance/2.f,float(collisionLine.vector().polar().y+geom2d::pi)}.cart()};
				vf2d collisionResolve2{h.GetPos()+vf2d{bumpDistance/2.f,collisionLine.vector().polar().y}.cart()};
				SetPos(collisionResolve1);
				h.SetPos(collisionResolve2);
				vel+=vf2d{GetBumpAmount(),float(collisionLine.vector().polar().y+geom2d::pi)}.cart();
				vel=vf2d{std::min(GetMaxSpeed(),vel.polar().x),vel.polar().y}.cart();
				h.vel+=vf2d{h.GetBumpAmount(),collisionLine.vector().polar().y}.cart();
				h.vel=vf2d{std::min(h.GetMaxSpeed(),h.vel.polar().x),h.vel.polar().y}.cart();
			}
			bumpTimer=h.bumpTimer=0.12f;
		}
	}
	for(Powerup&powerup:Powerup::GetPowerups()){
		if(z<=0.1f&&
			(!HasPowerup(powerup.GetType())||HasPowerup(Powerup::JET)&&powerup.GetType()==Powerup::JET&&jetFuel!=1.f||HasPowerup(Powerup::WHEEL)&&boostCounter<3&&canCollectWheelPowerupTimer==0.f)
			&&geom2d::overlaps(geom2d::circle<float>(GetPos(),collisionRadius),geom2d::circle<float>(powerup.GetPos(),20.f))){
			ObtainPowerup(powerup.GetType());
			if(IsPlayerControlled)HamsterAI::OnPowerupCollection(this->pos);
			powerup.OnPowerupObtain(*this);
		}
	}
	for(Checkpoint&checkpoint:Checkpoint::GetCheckpoints()){
		if(z<=0.1f&&geom2d::overlaps(geom2d::rect<float>(checkpoint.GetPos()-vf2d{62,60},{122.f,113.f}),geom2d::circle<float>(GetPos(),collisionRadius))&&!checkpointsCollected.count(checkpoint.GetPos())){
			checkpointsCollected.insert(checkpoint.GetPos());
			FloatingText::CreateFloatingText(pos,std::format("{} / {}",checkpointsCollected.size(),Checkpoint::GetCheckpoints().size()),{WHITE,GREEN},{1.5f,2.f});
			if(IsPlayerControlled)HamsterAI::OnCheckpointCollected(this->pos);
			if(IsPlayerControlled)checkpoint.OnCheckpointCollect();
		}
	}
	if(GetState()==NORMAL){
		for(const auto&[id,tunnel]:HamsterGame::Game().GetTunnels()){
			if(geom2d::overlaps(geom2d::circle<float>(GetPos(),4),geom2d::rect<float>(tunnel.worldPos,{16,16}))){
				SetState(Hamster::BURROWING);
				burrowTimer=1.f;
				enteredTunnel=id;
				burrowImgShrinkTimer=0.5f;
				switch(HamsterGame::Game().GetTileFacingDirection(tunnel.worldPos)){
					case Terrain::NORTH:{
						targetRot=geom2d::pi/2;
					}break;
					case Terrain::EAST:{
						targetRot=geom2d::pi;
					}break;
					case Terrain::SOUTH:{
						targetRot=-geom2d::pi/2;
					}break;
					case Terrain::WEST:{
						targetRot=0.;
					}break;
				}
				if(IsPlayerControlled)HamsterAI::OnTunnelEnter(this->pos);
			}
		}
	}
}

const float Hamster::GetRadius()const{
	return collisionRadius;
}

const Terrain::TerrainType Hamster::GetTerrainStandingOn()const{
	if(FlyingInTheAir())return Terrain::ROCK;
	return HamsterGame::Game().GetTerrainTypeAtPos(GetPos());
}

const Terrain::TerrainType Hamster::GetTerrainHoveringOver()const{
	return HamsterGame::Game().GetTerrainTypeAtPos(GetPos());
}

const bool Hamster::IsTerrainStandingOnSolid()const{
	return HamsterGame::Game().IsTerrainSolid(GetPos());
}

const float Hamster::GetTimeToMaxSpeed()const{
	float finalTimeToMaxSpd{DEFAULT_TIME_TO_MAX_SPD};
	if(!HasPowerup(Powerup::ICE)&&GetTerrainStandingOn()==Terrain::ICE)finalTimeToMaxSpd*=3;
	else if(!HasPowerup(Powerup::SWAMP)&&GetTerrainStandingOn()==Terrain::SWAMP)finalTimeToMaxSpd*=1.25;
	if(hamsterJet.has_value()){
		if(hamsterJet.value().GetState()==HamsterJet::LANDING)finalTimeToMaxSpd*=2.f;
		else if(FlyingInTheAir())finalTimeToMaxSpd*=3.f;
	}
	return finalTimeToMaxSpd;
}
const float Hamster::GetMaxSpeed()const{
	float finalMaxSpd{DEFAULT_MAX_SPD};
	switch(GetTerrainStandingOn()){
		case Terrain::GRASS:{
			if(!HasPowerup(Powerup::GRASS))finalMaxSpd*=0.80f;
		}break;
		case Terrain::SAND:{
			if(!HasPowerup(Powerup::SAND))finalMaxSpd*=0.60f;
		}break;
		case Terrain::SWAMP:{
			if(HasPowerup(Powerup::SWAMP))finalMaxSpd*=0.80f;
			else finalMaxSpd*=0.50f;
		}break;
		case Terrain::SHORE:{
			finalMaxSpd*=0.80f;
		}break;
		case Terrain::OCEAN:{
			finalMaxSpd*=0.10f;
		}break;
		case Terrain::LAVA:{
			finalMaxSpd*=0.6f;
		}break;
		case Terrain::FOREST:{
			if(!HasPowerup(Powerup::FOREST))finalMaxSpd*=0.50f;
		}break;
		case Terrain::VOID:{
			finalMaxSpd*=0.10f;
		}break;
	}
	if(HasPowerup(Powerup::WHEEL))finalMaxSpd*=1.5f;
	if(boostTimer!=0.f)finalMaxSpd*=1.5f;
	if(hamsterJet.has_value()){
		if(hamsterJet.value().GetState()==HamsterJet::LANDING)finalMaxSpd*=1.5f;
		else if(FlyingInTheAir())finalMaxSpd*=8.f;
	}
	if(!IsPlayerControlled){
		if(temporaryNode.has_value()||aiNodeTime>3.f)finalMaxSpd*=0.5f; //Slow down a bit...
		switch(ai.GetAIType()){
			case HamsterAI::SMART:{
				finalMaxSpd*=0.99f;
			}break;
			case HamsterAI::NORMAL:{
				finalMaxSpd*=0.92f;
			}break;
			case HamsterAI::DUMB:{
				finalMaxSpd*=0.85f;
			}break;
		}
	}
	return finalMaxSpd;
}
const float Hamster::GetFriction()const{
	float finalFriction{DEFAULT_FRICTION};
	if(!HasPowerup(Powerup::ICE)&&GetTerrainStandingOn()==Terrain::ICE)finalFriction*=0.1f;
	else if(!HasPowerup(Powerup::SWAMP)&&GetTerrainStandingOn()==Terrain::SWAMP)finalFriction*=0.6f;
	if(hamsterJet.has_value()){
		if(hamsterJet.value().GetState()==HamsterJet::LANDING)finalFriction*=1.5f;
		else if(FlyingInTheAir())finalFriction*=8.f;
	}
	return finalFriction;
}
const float Hamster::GetTurnSpeed()const{
	float finalTurnSpd{DEFAULT_TURN_SPD};
	if(!HasPowerup(Powerup::ICE)&&GetTerrainStandingOn()==Terrain::ICE)finalTurnSpd*=0.6f;
	return finalTurnSpd;
}
const float Hamster::GetBumpAmount()const{
	float finalBumpAmt{DEFAULT_BUMP_AMT};
	return finalBumpAmt*GetMaxSpeed()/DEFAULT_MAX_SPD;
}

void Hamster::ObtainPowerup(const Powerup::PowerupType powerup){
	if(powerup==Powerup::WHEEL){
		boostCounter=3;
		canCollectWheelPowerupTimer=5.f;
	}
	powerups.insert(powerup);
	
}
const bool Hamster::HasPowerup(const Powerup::PowerupType powerup)const{
	return powerups.count(powerup);
}

void Hamster::RemoveAllPowerups(){
	powerups.clear();
}

const bool Hamster::IsLethalTerrain(const vf2d pos)const{
	return HamsterGame::Game().GetTerrainTypeAtPos(pos)==Terrain::LAVA||HamsterGame::Game().GetTerrainTypeAtPos(pos)==Terrain::OCEAN||HamsterGame::Game().GetTerrainTypeAtPos(pos)==Terrain::SWAMP||HamsterGame::Game().GetTerrainTypeAtPos(pos)==Terrain::VOID;
}

const bool Hamster::IsDrowning()const{
	return drownTimer>0.f;
}
const bool Hamster::IsBurning()const{
	return burnTimer>0.f;
}
const float Hamster::GetDrownRatio()const{
	return drownTimer/DEFAULT_DROWN_TIME;
}
const float Hamster::GetBurnRatio()const{
	return burnTimer/DEFAULT_BURN_TIME;
}

const float&Hamster::GetZ()const{
	return z;
}

void Hamster::SetPos(const vf2d pos){
	bool movedY{false};
	bool movedX{false};
	if(state==FLYING&&HamsterGame::Game().IsInBounds(vf2d{this->pos.x,pos.y})||!HamsterGame::Game().IsTerrainSolid(vf2d{this->pos.x,pos.y})){
		this->pos=vf2d{this->pos.x,pos.y};
		movedY=true;
	}
	if(state==FLYING&&HamsterGame::Game().IsInBounds(vf2d{pos.x,this->pos.y})||!HamsterGame::Game().IsTerrainSolid(vf2d{pos.x,this->pos.y})){
		this->pos=vf2d{pos.x,this->pos.y};
		movedX=true;
	}
	if (!movedY&&(state==FLYING&&HamsterGame::Game().IsInBounds(vf2d{this->pos.x,pos.y})||!HamsterGame::Game().IsTerrainSolid(vf2d{this->pos.x,pos.y}))){
		this->pos=vf2d{this->pos.x,pos.y};
		movedY=true;
	}
	if(IsPlayerControlled&&(movedX||movedY)&&HamsterGame::Game().GetTerrainTypeAtPos(this->pos)!=Terrain::TUNNEL&&state!=FLYING)HamsterAI::OnMove(this->pos);
}

void Hamster::SetZ(const float z){
	this->z=z;
}

void Hamster::OnUserDestroy(){
	HAMSTER_LIST.clear();
}

void Hamster::SetDrawingOffsetY(const float offsetY){
	drawingOffsetY=offsetY;
}

const vf2d Hamster::GetNearestSafeLocation()const{		
	using TilePos=vi2d;
	using TileDistance=int;

	const vi2d playerTile{GetPos()/16};
	geom2d::rect<int>searchRect{{-1,-1},{3,3}};
	std::optional<std::pair<TilePos,TileDistance>>closestTile;

	const auto DetermineAndUpdateClosestTile=[this,&playerTile,&closestTile](const vi2d&tile){
		if(!IsLethalTerrain(tile*16)&&!IsSolidTerrain(tile*16)){
			std::pair<TilePos,TileDistance>closest{closestTile.value_or(std::pair<TilePos,TileDistance>{{},std::numeric_limits<int>::max()})};
			int tileDist{abs(playerTile.x-tile.x)+abs(playerTile.y-tile.y)};
			if(tileDist<=closest.second)closestTile.emplace(std::pair<TilePos,TileDistance>{tile,tileDist});
		}
	};
	while(!closestTile.has_value()){
		#pragma region Top Outline Check
		{
			for(int offsetX:std::ranges::iota_view(searchRect.pos.x,searchRect.size.x)){
				const vi2d checkTile{playerTile+vi2d{offsetX,searchRect.top().end.y}};
				DetermineAndUpdateClosestTile(checkTile);
			}
		}
		#pragma endregion
		#pragma region Bottom Outline Check
		{
			for(int offsetX:std::ranges::iota_view(searchRect.pos.x,searchRect.size.x)){
				const vi2d checkTile{playerTile+vi2d{offsetX,searchRect.bottom().end.y}};
				DetermineAndUpdateClosestTile(checkTile);
			}
		}
		#pragma endregion
		#pragma region Right Outline Check
		{
			for(int offsetY:std::ranges::iota_view(searchRect.pos.y+1,searchRect.size.y-2+1)){
				const vi2d checkTile{playerTile+vi2d{searchRect.right().end.x,offsetY}};
				DetermineAndUpdateClosestTile(checkTile);
			}
		}
		#pragma endregion
		#pragma region Left Outline Check
		{
			for(int offsetY:std::ranges::iota_view(searchRect.pos.y+1,searchRect.size.y-2+1)){
				const vi2d checkTile{playerTile+vi2d{searchRect.left().end.x,offsetY}};
				DetermineAndUpdateClosestTile(checkTile);
			}
		}
		#pragma endregion
		searchRect.pos-=1;
		searchRect.size+=2;
	}
	return closestTile.value().first*16+8;
}

const bool Hamster::IsSolidTerrain(const vf2d pos)const{
	return HamsterGame::Game().IsTerrainSolid(pos);
}

void Hamster::SetJetFuel(const float amt){
	jetFuel=amt;
}

void Hamster::Knockout(){
	SetState(KNOCKOUT);
	knockoutTimer=4.f;
	animations.ChangeState(internalAnimState,AnimationState::KNOCKOUT);
}

const float Hamster::GetSpeed()const{
	return vel.mag();
}

void Hamster::SetState(const HamsterState state){
	this->state=state;
}

const bool Hamster::CollectedAllCheckpoints()const{
	return checkpointsCollected.size()==Checkpoint::GetCheckpoints().size();
}

const bool Hamster::HasCollectedCheckpoint(const Checkpoint&cp)const{
	return checkpointsCollected.contains(cp.GetPos());
}
const std::vector<Hamster>&Hamster::GetHamsters(){
	return HAMSTER_LIST;
}
const Hamster::HamsterState&Hamster::GetState()const{
	return state;
}

const bool Hamster::BurnedOrDrowned()const{
	return GetState()==WAIT;
}
const bool Hamster::CanMove()const{
	return !CollectedAllCheckpoints()&&!IsBurrowed();
}

const bool Hamster::FlyingInTheAir()const{
	return GetState()==FLYING&&hamsterJet.value().GetZ()>0.5f&&GetZ()>0.5f;
}

void Hamster::HandleAIControls(){
	vf2d aimingDir{};

	const HamsterAI::ActionOptRef&currentAction{ai.GetCurrentAction()};
	if(!currentAction.has_value())return;
	const HamsterAI::Action&action{currentAction.value().get()};

	if(aiNodeTime>GetAIAdjustNodeTime()){
		if(ai.GetPreviousAction().has_value()&&ai.GetPreviousAction().value().get().type==HamsterAI::Action::MOVE){
			ai.RevertToPreviousAction();
			chooseTemporaryNodeNext=true;
		}else{
			int MAX_SEARCH_AMT{100};
			float SEARCH_RANGE{1.f};
			while(MAX_SEARCH_AMT>0){
				temporaryNode=GetPos()+vf2d{util::random_range(-SEARCH_RANGE*16,SEARCH_RANGE*16),util::random_range(-SEARCH_RANGE*16,SEARCH_RANGE*16)};
				if(!HamsterGame::Game().IsTerrainSolid(temporaryNode.value()))break;
				SEARCH_RANGE+=1.25f;
				MAX_SEARCH_AMT--;
			}
		}
		aiNodeTime=0.f;
	}

	vf2d targetLoc{action.pos};
	if(action.type==HamsterAI::Action::MOVE)targetLoc+=GetAINodePositionVariance();
	if(temporaryNode.has_value())targetLoc=temporaryNode.value();

	vf2d diff{targetLoc-GetPos()};

	float variance{GetAINodeDistanceVariance()};
	if(action.type!=HamsterAI::Action::MOVE)variance=12.f;

	if(diff.mag()<variance){
		if(action.type==HamsterAI::Action::LAUNCH_JET){
			if(true||HasPowerup(Powerup::JET)){ //Currently ignoring whether we have a jet or not...We can cheat!
				SetState(FLYING);
				lastSafeLocation.reset();
				if(IsPlayerControlled)HamsterAI::OnJetLaunch(this->pos);
				hamsterJet.emplace(*this);
			}else{
				//TODO
				//If we don't have a Jet and it's required at this point, we must backtrack nodes until we get to a location where one is placed...
				//This will be a separate behavioral AI node later on...
			}
		}
		ai.AdvanceToNextAction();
		const HamsterAI::ActionOptRef&nextAction{ai.GetCurrentAction()};
		if(nextAction.has_value()){
			const HamsterAI::Action&futureAction{nextAction.value().get()};
			if(futureAction.type==HamsterAI::Action::MOVE){
				switch(ai.GetAIType()){
					case HamsterAI::SMART:{
						if(util::random()%100<2)ai.AdvanceToNextAction();
					}break;
					case HamsterAI::NORMAL:{
						if(util::random()%100<25)ai.AdvanceToNextAction();
					}break;
					case HamsterAI::DUMB:{
						if(util::random()%100<50)ai.AdvanceToNextAction();
					}break;
				}
			}
		}
		if(chooseTemporaryNodeNext){
			int MAX_SEARCH_AMT{100};
			float SEARCH_RANGE{1.f};
			while(MAX_SEARCH_AMT>0){
				temporaryNode=GetPos()+vf2d{util::random_range(-SEARCH_RANGE*16,SEARCH_RANGE*16),util::random_range(-SEARCH_RANGE*16,SEARCH_RANGE*16)};
				if(!HamsterGame::Game().IsTerrainSolid(temporaryNode.value()))break;
				SEARCH_RANGE+=1.25f;
				MAX_SEARCH_AMT--;
			}
		}else temporaryNode.reset();
		chooseTemporaryNodeNext=false;
		aiNodeTime=0.f;
	}
	const float moveThreshold{4.f};

	if(diff.y<-moveThreshold){
		aimingDir+=vf2d{0,-1};
	}
	if(diff.x>moveThreshold){
		aimingDir+=vf2d{1,0};
	}
	if(diff.y>moveThreshold){
		aimingDir+=vf2d{0,1};
	}
	if(diff.x<-moveThreshold){
		aimingDir+=vf2d{-1,0};
	}
	if(aimingDir!=vf2d{}){
		targetRot=aimingDir.norm().polar().y;
		vf2d currentVel{vel};
		vel=vf2d{currentVel.polar().x+((GetMaxSpeed()/GetTimeToMaxSpeed())*HamsterGame::Game().GetElapsedTime()),rot}.cart();
		vel=vf2d{std::min(GetMaxSpeed(),vel.polar().x),vel.polar().y}.cart();
		frictionEnabled=false;
	}
}

const float Hamster::GetAILandingSpeed()const{
	switch(ai.GetAIType()){
		case HamsterAI::SMART:{
			return 4.9f;
		}break;
		case HamsterAI::NORMAL:{
			return 2.9f;
		}break;
		case HamsterAI::DUMB:{
			return 1.2f;
		}break;
	}
	return 2.f;
}

const float Hamster::GetAIAdjustNodeTime()const{
	switch(ai.GetAIType()){
		case HamsterAI::SMART:{
			return 5.f;
		}break;
		case HamsterAI::NORMAL:{
			return 7.5f;
		}break;
		case HamsterAI::DUMB:{
			return 10.f;
		}break;
	}
	return 7.5f;
}

const bool Hamster::IsBurrowed()const{
	return GetState()==BURROWING||GetState()==BURROW_WAIT||GetState()==SURFACING;
}

const float Hamster::GetAINodeDistanceVariance()const{
	switch(ai.GetAIType()){
		case HamsterAI::SMART:{
			return 12.f*float(randomId%100)/100.f+12.f;
		}break;
		case HamsterAI::NORMAL:{
			return 18.f*float(randomId%100)/100.f+18.f;
		}break;
		case HamsterAI::DUMB:{
			return 24.f*float(randomId%100)/100.f+24.f;
		}break;
	}
	return 12.f*float(randomId%100)/100.f+12.f;
}

const vf2d Hamster::GetAINodePositionVariance()const{
	vf2d finalOffset{};
	float seedX{float(randomId%100)/100.f};
	float seedY{float((randomId*457)%100)/100.f};
	switch(ai.GetAIType()){
		case HamsterAI::SMART:{
			finalOffset.x=seedX*16.f-8.f;
			finalOffset.y=seedY*16.f-8.f;
		}break;
		case HamsterAI::NORMAL:{
			finalOffset.x=seedX*32.f-16.f;
			finalOffset.y=seedY*32.f-16.f;
		}break;
		case HamsterAI::DUMB:{
			finalOffset.x=seedX*48.f-24.f;
			finalOffset.y=seedY*48.f-24.f;
		}break;
	}
	return finalOffset;
}
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

#include "HamsterJet.h"
#include "Hamster.h"
#include "util.h"

HamsterJet::HamsterJet(Hamster&hamster)
	:hamster(hamster),hamsterOriginalPos(hamster.GetPos()),pos({hamster.GetPos().x-128.f,hamster.GetPos().y+32.f}),z(3.f),state(SWOOP_DOWN),timer(3.f){
	jet.Initialize("hamster_jet.png",{78,223,208},{79,81,128});
	lights.Initialize("hamster_jet.png",{245,233,130},{245,233,130});
}
void HamsterJet::Update(const float fElapsedTime){
	jet.Update(fElapsedTime);
	lights.Update(fElapsedTime);
	timer=std::max(0.f,timer-fElapsedTime);
	easeInTimer=std::max(0.f,easeInTimer-fElapsedTime);
	lastTappedSpace+=fElapsedTime;
	switch(state){
		case SWOOP_DOWN:{
			HamsterGame::Game().SetZoom(1.5f);
			z=util::lerp(0.f,3.f,std::pow(timer/3.f,2));
			vf2d originalPos{hamster.GetPos().x-128.f,hamster.GetPos().y+32.f};
			if(timer<=0.4f){
				hamster.SetPos(hamsterOriginalPos-vf2d{0.f,sin(float(geom2d::pi)*timer/0.4f)*8.f});
				hamster.SetZ(sin(float(geom2d::pi)*timer/0.4f)*0.2f);
				jetState[TOP_LEFT]=jetState[BOTTOM_LEFT]=jetState[BOTTOM_RIGHT]=jetState[TOP_RIGHT]=OFF;
			}else{
				jetState[TOP_LEFT]=jetState[BOTTOM_LEFT]=jetState[BOTTOM_RIGHT]=jetState[TOP_RIGHT]=ON;
				pos=hamster.GetPos().lerp(originalPos,std::pow(timer/3.f,4));
			}
			if(timer<=0.f){
				state=RISE_UP;
				hamster.SetPos(pos);
				this->originalPos=pos;
				targetPos=pos+vf2d{128.f,32};
				targetZ=8.f;
				timer=3.f;
			}
		}break;
		case RISE_UP:{
			jetState[TOP_LEFT]=jetState[BOTTOM_LEFT]=jetState[BOTTOM_RIGHT]=jetState[TOP_RIGHT]=ON;
			pos=targetPos.lerp(originalPos,std::sqrt(timer/3.f));
			z=util::lerp(targetZ,0.f,timer/3.f);
			hamster.SetPos(pos);
			hamster.SetZ(z+0.03f);
			if(timer<=0.f){
				state=HAMSTER_CONTROL;
				HamsterGame::Game().SetZoom(0.6f);
				easeInTimer=0.6f;
			}
		}break;
		case HAMSTER_CONTROL:{
			jetState[TOP_LEFT]=jetState[BOTTOM_LEFT]=jetState[BOTTOM_RIGHT]=jetState[TOP_RIGHT]=OFF;
			HandleJetControls();
		}break;
		case LANDING:{
			jetState[TOP_LEFT]=jetState[BOTTOM_LEFT]=jetState[BOTTOM_RIGHT]=jetState[TOP_RIGHT]=OFF;
			if(hamster.IsPlayerControlled)HandleJetControls();
			else{
				//TODO: AI controls here!
			}
			pos=hamster.GetPos();
			hamster.SetZ(hamster.GetZ()-fallSpd*fElapsedTime);
			z=hamster.GetZ();
			if(hamster.GetZ()<=0.f){
				hamster.SetZ(0.f);
				state=COMPLETE_LANDING;
				hamster.state=Hamster::NORMAL;
				HamsterGame::Game().SetZoom(1.f);
				timer=3.f;
				originalPos=hamster.GetPos();
				targetPos={hamster.GetPos().x+128.f,hamster.GetPos().y+32.f};
				Terrain::CrashSpeed crashSpd{Terrain::LIGHT};
				if(fallSpd>4.f)crashSpd=Terrain::MAX;
				else if(fallSpd>2.f)crashSpd=Terrain::MEDIUM;
				std::pair<Terrain::FuelDamage,Terrain::KnockoutOccurs>landingResult{Terrain::GetFuelDamageTakenAndKnockoutEffect(hamster.GetTerrainStandingOn(),crashSpd)};
				hamster.jetFuel=std::max(0.f,hamster.jetFuel-landingResult.first);
				if(landingResult.second)hamster.Knockout();
				if(hamster.IsTerrainStandingOnSolid())hamster.SetPos(hamster.GetNearestSafeLocation());
				if(hamster.jetFuel<=0.f)hamster.powerups.erase(Powerup::JET);
			}
		}break;
		case COMPLETE_LANDING:{
			z=util::lerp(3.f,0.f,std::pow(timer/3.f,2));
			if(timer<=0.f){
				hamster.hamsterJet.reset();
				return;
			}else{
				jetState[TOP_LEFT]=jetState[BOTTOM_LEFT]=jetState[BOTTOM_RIGHT]=jetState[TOP_RIGHT]=ON;
				pos=targetPos.lerp(originalPos,std::pow(timer/3.f,4));
			}
		}break;
	}
}
void HamsterJet::Draw(){
	float drawingOffsetY{0.f};
	hamster.SetDrawingOffsetY(0.f);
	if((state==HAMSTER_CONTROL||state==LANDING)&&z>2.f){
		HamsterGame::Game().SetZ(z/2.f);
		HamsterGame::Game().tv.DrawRotatedDecal(pos,HamsterGame::GetGFX("aimingTarget.png").Decal(),0.f,HamsterGame::GetGFX("aimingTarget.png").Sprite()->Size()/2);
	}
	if(state==HAMSTER_CONTROL){
		drawingOffsetY=util::lerp(48.f,0.f,easeInTimer/0.6f);
		hamster.SetDrawingOffsetY(util::lerp(48.f,0.f,easeInTimer/0.6f));
	}
	HamsterGame::Game().SetZ(z);
	HamsterGame::Game().tv.DrawPartialRotatedDecal(pos+vf2d{0,drawingOffsetY},jet.Decal(),0.f,{24,24},{},{48,48});
	const Animate2D::FrameSequence&flameAnim{HamsterGame::Game().GetAnimation("hamster_jet.png",HamsterGame::AnimationState::JET_FLAMES)};
	const Animate2D::Frame&flameFrame{flameAnim.GetFrame(HamsterGame::Game().GetRuntime())};
	HamsterGame::Game().SetZ(z+0.01f);
	if(jetState[TOP_LEFT])HamsterGame::Game().tv.DrawPartialRotatedDecal(pos+vf2d{0,drawingOffsetY},flameFrame.GetSourceImage()->Decal(),0.f,flameFrame.GetSourceRect().size/2,flameFrame.GetSourceRect().pos+vf2d{0,0},flameFrame.GetSourceRect().size/2);
	if(jetState[BOTTOM_LEFT])HamsterGame::Game().tv.DrawPartialRotatedDecal(pos+vf2d{0,drawingOffsetY},flameFrame.GetSourceImage()->Decal(),0.f,{24,0},flameFrame.GetSourceRect().pos+vf2d{0,24},flameFrame.GetSourceRect().size/2);
	if(jetState[BOTTOM_RIGHT])HamsterGame::Game().tv.DrawPartialRotatedDecal(pos+vf2d{0,drawingOffsetY},flameFrame.GetSourceImage()->Decal(),0.f,{0,0},flameFrame.GetSourceRect().pos+vf2d{24,24},flameFrame.GetSourceRect().size/2);
	if(jetState[TOP_RIGHT])HamsterGame::Game().tv.DrawPartialRotatedDecal(pos+vf2d{0,drawingOffsetY},flameFrame.GetSourceImage()->Decal(),0.f,{0,24},flameFrame.GetSourceRect().pos+vf2d{24,0},flameFrame.GetSourceRect().size/2);
	const Animate2D::FrameSequence&lightAnim{HamsterGame::Game().GetAnimation("hamster_jet.png",HamsterGame::AnimationState::JET_LIGHTS)};
	const Animate2D::Frame&lightFrame{lightAnim.GetFrame(HamsterGame::Game().GetRuntime())};
	HamsterGame::Game().SetZ(z+0.02f);
	HamsterGame::Game().tv.DrawPartialRotatedDecal(pos+vf2d{0,drawingOffsetY},lights.Decal(),0.f,lightFrame.GetSourceRect().size/2.f,lightFrame.GetSourceRect().pos,lightFrame.GetSourceRect().size);
	HamsterGame::Game().SetZ(0.f);
}

void HamsterJet::HandleJetControls(){
	lastTappedSpace+=HamsterGame::Game().GetElapsedTime();
	vf2d aimingDir{};
	if(HamsterGame::Game().GetKey(W).bHeld){
		aimingDir+=vf2d{0,-1};
		jetState[BOTTOM_RIGHT]=ON;
		jetState[BOTTOM_LEFT]=ON;
	}
	if(HamsterGame::Game().GetKey(D).bHeld){
		aimingDir+=vf2d{1,0};
		jetState[BOTTOM_LEFT]=ON;
		jetState[TOP_LEFT]=ON;
	}
	if(HamsterGame::Game().GetKey(S).bHeld){
		aimingDir+=vf2d{0,1};
		jetState[TOP_LEFT]=ON;
		jetState[TOP_RIGHT]=ON;
	}
	if(HamsterGame::Game().GetKey(A).bHeld){
		aimingDir+=vf2d{-1,0};
		jetState[BOTTOM_RIGHT]=ON;
		jetState[TOP_RIGHT]=ON;
	}
	if(aimingDir!=vf2d{}&&hamster.jetFuel>0.f){
		hamster.targetRot=aimingDir.norm().polar().y;
		const vf2d currentVel{hamster.vel};
		hamster.vel+=vf2d{currentVel.polar().x+(hamster.GetMaxSpeed()*HamsterGame::Game().GetElapsedTime())/hamster.GetTimeToMaxSpeed(),hamster.rot}.cart();
		hamster.vel=vf2d{std::min(hamster.GetMaxSpeed(),hamster.vel.polar().x),hamster.vel.polar().y}.cart();
		hamster.frictionEnabled=false;
	}
	if(HamsterGame::Game().GetKey(UP).bHeld){
		fallSpd=std::min(5.f,fallSpd+5.f*HamsterGame::Game().GetElapsedTime());
	}
	if(HamsterGame::Game().GetKey(DOWN).bHeld){
		fallSpd=std::max(1.f,fallSpd-5.f*HamsterGame::Game().GetElapsedTime());
	}
	if(HamsterGame::Game().GetKey(SPACE).bPressed){
		if(lastTappedSpace<=0.6f){
			state=LANDING;
		}
		lastTappedSpace=0.f;
	}
}

const HamsterJet::State HamsterJet::GetState()const{
	return state;
}

void HamsterJet::DrawOverlay()const{
	if(state==LANDING){
		HamsterGame::Game().DrawDecal(HamsterGame::SCREEN_FRAME.pos,HamsterGame::GetGFX("fallometer_outline.png").Decal());
		float meterStartY{68.f};
		float meterEndY{223.f};
		float meterHeight{meterEndY-meterStartY};
		HamsterGame::Game().DrawPartialDecal(HamsterGame::SCREEN_FRAME.pos+vf2d{0,222}-vf2d{0,(fallSpd/5.f)*meterHeight},HamsterGame::GetGFX("fallometer.png").Decal(),vf2d{0,223}-vf2d{0,(fallSpd/5.f)*meterHeight},vf2d{float(HamsterGame::GetGFX("fallometer.png").Sprite()->width),(fallSpd/5.f)*meterHeight});
	}
}

void HamsterJet::SetPos(const vf2d pos){
	this->pos=pos;
}
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

std::vector<Hamster>Hamster::HAMSTER_LIST;
const uint8_t Hamster::MAX_HAMSTER_COUNT{100U};
const uint8_t Hamster::NPC_HAMSTER_COUNT{5U};
const std::vector<std::string>Hamster::NPC_HAMSTER_IMAGES{
	"hamster.png",
};
const std::string Hamster::PLAYER_HAMSTER_IMAGE{"hamster.png"};
std::optional<Hamster*>Hamster::playerHamster;

Hamster::Hamster(const vf2d spawnPos,const std::string_view img,const PlayerControlled IsPlayerControlled)
:pos(spawnPos),IsPlayerControlled(IsPlayerControlled){
	animations=HamsterGame::GetAnimations(img);
	animations.ChangeState(internalAnimState,HamsterGame::DEFAULT);
}

void Hamster::UpdateHamsters(const float fElapsedTime){
	for(Hamster&h:HAMSTER_LIST){
		h.lastSafeLocationTimer=std::max(0.f,h.lastSafeLocationTimer-fElapsedTime);
		h.animations.UpdateState(h.internalAnimState,fElapsedTime);
		h.frictionEnabled=true;
		h.bumpTimer-=fElapsedTime;
		h.HandleCollision();
		switch(h.state){
			case NORMAL:{
				if(h.IsPlayerControlled){
					h.HandlePlayerControls();
				}else{
					//TODO: NPC controls.
				}
			}break;
			case BUMPED:{
				if(h.bumpTimer<=0.f){
					h.state=NORMAL;
				}
			}break;
			case DROWNING:{
				if(h.imgScale>0.f){
					h.shrinkEffectColor=BLACK;
					h.imgScale=std::max(0.f,h.imgScale-0.5f*fElapsedTime);
				}
				else{
					h.waitTimer=4.f;
					h.state=WAIT;
				}
			}break;
			case WAIT:{
				h.waitTimer=std::max(0.f,h.waitTimer-fElapsedTime);
				if(h.waitTimer<=0.f){
					h.imgScale=1.f;
					h.drownTimer=0.f;
					if(!h.lastSafeLocation.has_value()){
						#pragma region Safe Terrain Outline Search Algorithm
						{
							using TilePos=vi2d;
							using TileDistance=int;

							const vi2d playerTile{h.GetPos()/16};
							geom2d::rect<int>searchRect{{-1,-1},{3,3}};
							std::optional<std::pair<TilePos,TileDistance>>closestTile;

							const auto DetermineAndUpdateClosestTile=[&h,&playerTile,&closestTile](const vi2d&tile){
								if(!h.IsLethalTerrain(tile*16)){
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
							h.lastSafeLocation=closestTile.value().first*16+8;
						}
						#pragma endregion
						h.SetPos(h.lastSafeLocation.value());
					}
					h.state=NORMAL;
					h.RemoveAllPowerups();
				}
			}break;
			case BURNING:{
				if(h.imgScale>0.f){
					h.shrinkEffectColor=RED;
					h.imgScale=std::max(0.f,h.imgScale-0.5f*fElapsedTime);
				}else{
					h.waitTimer=4.f;
					h.state=WAIT;
				}
			}break;
		}
		if(h.hamsterJet.has_value())h.hamsterJet.value().Update(fElapsedTime);
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
				h.state=DROWNING;
			}
			if(h.burnTimer>=h.DEFAULT_BURN_TIME&&h.state!=BURNING&&h.state!=WAIT){
				h.state=BURNING;
			}
		}
		h.TurnTowardsTargetDirection();
		h.MoveHamster();
	}
}

void Hamster::LoadHamsters(const vf2d startingLoc){
	HAMSTER_LIST.clear();
	playerHamster.reset();
	HAMSTER_LIST.reserve(MAX_HAMSTER_COUNT);
	if(NPC_HAMSTER_COUNT+1>MAX_HAMSTER_COUNT)throw std::runtime_error{std::format("WARNING! Max hamster count is too high! Please expand the MAX_HAMSTER_COUNT if you want more hamsters. Requested {} hamsters.",MAX_HAMSTER_COUNT)};
	playerHamster=&HAMSTER_LIST.emplace_back(startingLoc,PLAYER_HAMSTER_IMAGE,PLAYER_CONTROLLED);
	for(int i:std::ranges::iota_view(0U,NPC_HAMSTER_COUNT)){
		HAMSTER_LIST.emplace_back(startingLoc,NPC_HAMSTER_IMAGES.at(util::random()%NPC_HAMSTER_IMAGES.size()),NPC);
	}
}

void Hamster::DrawHamsters(TransformedView&tv){
	for(Hamster&h:HAMSTER_LIST){
		const Animate2D::FrameSequence&anim{h.animations.GetFrames(h.internalAnimState)};
		const Animate2D::FrameSequence&wheelTopAnim{h.animations.GetFrames(HamsterGame::WHEEL_TOP)};
		const Animate2D::FrameSequence&wheelBottomAnim{h.animations.GetFrames(HamsterGame::WHEEL_BOTTOM)};
		const Animate2D::Frame&img{h.animations.GetState(h.internalAnimState)==HamsterGame::DEFAULT?anim.GetFrame(h.distanceTravelled/100.f):h.GetCurrentAnimation()};
		const Animate2D::Frame&wheelTopImg{wheelTopAnim.GetFrame(h.distanceTravelled/80.f)};
		const Animate2D::Frame&wheelBottomImg{wheelBottomAnim.GetFrame(h.distanceTravelled/80.f)};
		if(h.hamsterJet.has_value())h.hamsterJet.value().Draw();
		HamsterGame::Game().SetZ(h.z);
		if(h.HasPowerup(Powerup::WHEEL))tv.DrawPartialRotatedDecal(h.pos+vf2d{0.f,h.drawingOffsetY},wheelBottomImg.GetSourceImage()->Decal(),h.rot,wheelBottomImg.GetSourceRect().size/2,wheelBottomImg.GetSourceRect().pos,wheelBottomImg.GetSourceRect().size,vf2d{1.f,1.f}*h.imgScale,PixelLerp(h.shrinkEffectColor,WHITE,h.imgScale));
		HamsterGame::Game().SetZ(h.z+0.005f);
		tv.DrawPartialRotatedDecal(h.pos+vf2d{0.f,h.drawingOffsetY},img.GetSourceImage()->Decal(),h.rot,img.GetSourceRect().size/2,img.GetSourceRect().pos,img.GetSourceRect().size,vf2d{1.f,1.f}*h.imgScale,PixelLerp(h.shrinkEffectColor,WHITE,h.imgScale));
		HamsterGame::Game().SetZ(h.z+0.01f);
		if(h.HasPowerup(Powerup::WHEEL))tv.DrawPartialRotatedDecal(h.pos+vf2d{0.f,h.drawingOffsetY},wheelTopImg.GetSourceImage()->Decal(),h.rot,wheelTopImg.GetSourceRect().size/2,wheelTopImg.GetSourceRect().pos,wheelTopImg.GetSourceRect().size,vf2d{1.f,1.f}*h.imgScale,PixelLerp(h.shrinkEffectColor,{255,255,255,192},h.imgScale));
		HamsterGame::Game().SetZ(0.f);
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
		vel+=vf2d{currentVel.polar().x+(GetMaxSpeed()*HamsterGame::Game().GetElapsedTime())/GetTimeToMaxSpeed(),rot}.cart();
		vel=vf2d{std::min(GetMaxSpeed(),vel.polar().x),vel.polar().y}.cart();
		frictionEnabled=false;
	}
	if(HamsterGame::Game().GetKey(SPACE).bPressed){
		if(lastTappedSpace<=0.6f&&HasPowerup(Powerup::JET)){
			state=FLYING;
			lastSafeLocation.reset();
			hamsterJet.emplace(*this);
		}
		lastTappedSpace=0.f;
	}
	if(HamsterGame::Game().GetKey(P).bPressed)ObtainPowerup(Powerup::JET);
}

void Hamster::TurnTowardsTargetDirection(){
	util::turn_towards_direction(rot,targetRot,GetTurnSpeed()*HamsterGame::Game().GetElapsedTime());
}

void Hamster::MoveHamster(){
	SetPos(GetPos()+vel*HamsterGame::Game().GetElapsedTime());
	
	distanceTravelled+=vel.mag()*HamsterGame::Game().GetElapsedTime();

	#pragma region Handle Friction
		if(frictionEnabled){
			const vf2d currentVel{vel};
			vel=vf2d{std::max(0.f,currentVel.polar().x-GetFriction()*HamsterGame::Game().GetElapsedTime()),currentVel.polar().y}.cart();
		}
	#pragma endregion
}

void Hamster::HandleCollision(){
	for(Hamster&h:HAMSTER_LIST){
		if(this==&h)continue;
		if(abs(z-h.z)<=0.1f&&geom2d::overlaps(geom2d::circle<float>(GetPos(),GetRadius()),geom2d::circle<float>(h.GetPos(),h.GetRadius()))){
			if(geom2d::line<float>(GetPos(),h.GetPos()).length()==0.f){ //Push these two in random directions, they are on top of each other!
				float randDir{util::random(2*geom2d::pi)};
				vf2d collisionResolve1{GetPos()+vf2d{GetRadius(),randDir}.cart()};
				vf2d collisionResolve2{h.GetPos()+vf2d{h.GetRadius(),float(randDir+geom2d::pi)}.cart()};
				SetPos(collisionResolve1);
				h.SetPos(collisionResolve2);
				vel=vf2d{GetBumpAmount(),randDir}.cart();
				h.vel=vf2d{GetBumpAmount(),float(randDir+geom2d::pi)}.cart();
			}else{
				geom2d::line<float>collisionLine{geom2d::line<float>(GetPos(),h.GetPos())};
				float distance{collisionLine.length()};
				float totalRadii{GetRadius()+h.GetRadius()};
				float bumpDistance{totalRadii-distance};
				vf2d collisionResolve1{GetPos()+vf2d{bumpDistance/2.f,float(collisionLine.vector().polar().y+geom2d::pi)}.cart()};
				vf2d collisionResolve2{h.GetPos()+vf2d{bumpDistance/2.f,collisionLine.vector().polar().y}.cart()};
				SetPos(collisionResolve1);
				h.SetPos(collisionResolve2);
				vel=vf2d{GetBumpAmount(),float(collisionLine.vector().polar().y+geom2d::pi)}.cart();
				h.vel=vf2d{GetBumpAmount(),collisionLine.vector().polar().y}.cart();
			}
			state=h.state=BUMPED;
			bumpTimer=h.bumpTimer=0.12f;
		}
	}
	for(Powerup&powerup:Powerup::GetPowerups()){
		if(!HasPowerup(powerup.GetType())&&geom2d::overlaps(geom2d::circle<float>(GetPos(),collisionRadius),geom2d::circle<float>(powerup.GetPos(),20.f))){
			ObtainPowerup(powerup.GetType());
			powerup.OnPowerupObtain();
		}
	}
}

const float Hamster::GetRadius()const{
	return collisionRadius;
}

const Terrain::TerrainType Hamster::GetTerrainStandingOn()const{
	if(state==FLYING)return Terrain::ROCK;
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
		if(hamsterJet.value().GetState()==HamsterJet::LANDING)finalTimeToMaxSpd*=5.f;
		else if(state==FLYING)finalTimeToMaxSpd*=30.f;
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
	}
	if(HasPowerup(Powerup::WHEEL))finalMaxSpd*=1.5f;
	if(hamsterJet.has_value()){
		if(hamsterJet.value().GetState()==HamsterJet::LANDING)finalMaxSpd*=1.5f;
		else if(state==FLYING)finalMaxSpd*=8.f;
	}
	return finalMaxSpd;
}
const float Hamster::GetFriction()const{
	float finalFriction{DEFAULT_FRICTION};
	if(!HasPowerup(Powerup::ICE)&&GetTerrainStandingOn()==Terrain::ICE)finalFriction*=0.1f;
	else if(!HasPowerup(Powerup::SWAMP)&&GetTerrainStandingOn()==Terrain::SWAMP)finalFriction*=0.6f;
	if(hamsterJet.has_value()){
		if(hamsterJet.value().GetState()==HamsterJet::LANDING)finalFriction*=1.5f;
		else if(state==FLYING)finalFriction*=8.f;
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
	if(state==FLYING&&HamsterGame::Game().IsInBounds(vf2d{this->pos.x,pos.y})||!HamsterGame::Game().IsTerrainSolid(vf2d{this->pos.x,pos.y})){
		this->pos=vf2d{this->pos.x,pos.y};
		movedY=true;
	}
	if(state==FLYING&&HamsterGame::Game().IsInBounds(vf2d{pos.x,this->pos.y})||!HamsterGame::Game().IsTerrainSolid(vf2d{pos.x,this->pos.y}))this->pos=vf2d{pos.x,this->pos.y};
	if (!movedY&&(state==FLYING&&HamsterGame::Game().IsInBounds(vf2d{this->pos.x,pos.y})||!HamsterGame::Game().IsTerrainSolid(vf2d{this->pos.x,pos.y})))this->pos=vf2d{this->pos.x,pos.y};
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
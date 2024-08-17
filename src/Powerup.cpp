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
#include "Powerup.h"

std::vector<Powerup>Powerup::powerupList;
std::unordered_map<int,std::pair<Powerup::PowerupType,Powerup::TileType>>Powerup::powerupIds;
const vf2d Powerup::POWERUP_TILESET_STARTING_POS{144.f,816.f};

Powerup::Powerup(const vf2d pos,const PowerupType type)
:pos(pos),type(type){}
void Powerup::Initialize(const std::vector<Powerup>&powerupList){
	Powerup::powerupList.clear();
	Powerup::powerupList=powerupList;
}
const vf2d&Powerup::GetPos()const{
	return pos;	
}
const Powerup::PowerupType&Powerup::GetType()const{
	return type;
}
std::vector<Powerup>&Powerup::GetPowerups(){
	return powerupList;
}

void Powerup::AddOrUpdatePowerupIdList(const int powerupId,const PowerupType powerupType){
	if(powerupIds.count(powerupId)){
		powerupIds[powerupId].first=powerupType;
	}else{
		powerupIds[powerupId]={powerupType,TileType::DEFAULT};
	}
}

void Powerup::AddOrUpdatePowerupIdList(const int powerupId,const TileType powerupTileType){
	if(powerupIds.count(powerupId)){
		powerupIds[powerupId].second=powerupTileType;
	}else{
		powerupIds[powerupId]={PowerupType::WHEEL,powerupTileType};
	}
}

const bool Powerup::TileIDIsPowerupTile(const int tileId){
	return powerupIds.count(tileId);
}


const bool Powerup::TileIDIsUpperLeftPowerupTile(const int tileId){
	return powerupIds.count(tileId)&&powerupIds[tileId].second==TileType::IS_UPPERLEFT_TILE;
}

const Powerup::PowerupType Powerup::TileIDPowerupType(const int tileId){
	return powerupIds.at(tileId).first;
}

void Powerup::UpdatePowerups(const float fElapsedTime){
	for(Powerup&powerup:powerupList){
		powerup.z=abs(pow(sin(HamsterGame::Game().GetRuntime()/1.5f*geom2d::pi),4.f)*4)+5;
		if(powerup.spinSpd>0.f){
			powerup.spinSpd+=fElapsedTime;
			if(powerup.spinSpd>=3.f)powerup.spinSpd=0.f;
		}
	}
}

void Powerup::DrawPowerups(TransformedView&tv){
	for(const Powerup&powerup:powerupList){
		geom2d::rect<float>spriteRect{GetPowerupSubimageRect(powerup.GetType())};
		tv.DrawRotatedDecal(powerup.GetPos(),HamsterGame::GetGFX("shadow.png").Decal(),0.f,{16,16});
		float scaleX{1.f};
		if(powerup.spinSpd>0.f)scaleX=sin(geom2d::pi*HamsterGame::Game().GetRuntime()/powerup.spinSpd);
		tv.DrawPartialRotatedDecal(powerup.GetPos()-vf2d{0,powerup.z},HamsterGame::GetGFX("gametiles.png").Decal(),0.f,{16,16},spriteRect.pos,spriteRect.size,{scaleX,1.f});
	}
}

const geom2d::rect<float>Powerup::GetPowerupSubimageRect(const PowerupType powerupType){
	return {POWERUP_TILESET_STARTING_POS+vf2d{int(powerupType)*32.f,0.f},{32,32}};
}

void Powerup::OnPowerupObtain(){
	spinSpd=0.3f;
}
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
#include <vector>
#include <unordered_map>
#include "olcUTIL_Geometry2D.h"
#include "olcPGEX_TransformedView.h"

class Hamster;

class Powerup{
public:
	enum PowerupType{
		WHEEL,
		GRASS,
		SAND,
		SWAMP,
		LAVA,
		FOREST,
		ICE,
		JET,
	};
	enum TileType{
		IS_UPPERLEFT_TILE=true,
		DEFAULT=false,
	};
private:
	static std::vector<Powerup>powerupList;
	static std::unordered_map<int,std::pair<PowerupType,TileType>>powerupIds;
	static const vf2d POWERUP_TILESET_STARTING_POS;
	static std::unordered_map<PowerupType,std::string>powerupNames;
	vf2d pos;
	float z{5.f};
	float spinSpd{};
	PowerupType type;
public:
	Powerup(const vf2d pos,const PowerupType type);
	static void Initialize(const std::vector<Powerup>&powerupList);
	const vf2d&GetPos()const;
	const PowerupType&GetType()const;
	const std::string&GetName()const;
	static std::vector<Powerup>&GetPowerups();
	static void AddOrUpdatePowerupIdList(const int powerupId,const PowerupType powerupType);
	static void AddOrUpdatePowerupIdList(const int powerupId,const TileType powerupTileType);
	static const bool TileIDIsPowerupTile(const int tileId);
	static const bool TileIDIsUpperLeftPowerupTile(const int tileId);
	static const PowerupType TileIDPowerupType(const int tileId);
	static void UpdatePowerups(const float fElapsedTime);
	static void DrawPowerups(TransformedView&tv);
	static const geom2d::rect<float>GetPowerupSubimageRect(const PowerupType powerupType);
	void OnPowerupObtain(Hamster&pickupHamster);
	static std::unordered_map<PowerupType,std::string>powerupInfo;
};
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
#include "Terrain.h"

const std::string Terrain::TerrainToString(const TerrainType type){
	switch(type){
		case ROCK:{
			return "Rock";
		}break;
		case GRASS:{
			return "Grass";
		}break;
		case SAND:{
			return "Sand";
		}break;
		case SWAMP:{
			return "Swamp";
		}break;
		case LAVA:{
			return "Lava";
		}break;
		case SHORE:{
			return "Shore";
		}break;
		case OCEAN:{
			return "Ocean";
		}break;
		case FOREST:{
			return "Forest";
		}break;
		case TUNNEL:{
			return "Tunnel";
		}break;
		case ICE:{
			return "Ice";
		}break;
		default:{
			return "Void";
		}
	}
}
const std::pair<Terrain::FuelDamage,Terrain::KnockoutOccurs>Terrain::GetFuelDamageTakenAndKnockoutEffect(const TerrainType type,const CrashSpeed crashSpeed){

	const auto GetHardSurfaceCrashResult=[&crashSpeed](){
		switch(crashSpeed){
			case MAX:{
				return std::pair<FuelDamage,KnockoutOccurs>{1.f,true};
			}break;
			case MEDIUM:{
				return std::pair<FuelDamage,KnockoutOccurs>{0.8f,false};
			}break;
			case LIGHT:{
				return std::pair<FuelDamage,KnockoutOccurs>{0.5f,false};
			}break;
		}
	};
	const auto GetMediumSurfaceCrashResult=[&crashSpeed](){
		switch(crashSpeed){
			case MAX:{
				return std::pair<FuelDamage,KnockoutOccurs>{0.8f,true};
			}break;
			case MEDIUM:{
				return std::pair<FuelDamage,KnockoutOccurs>{0.5f,false};
			}break;
			case LIGHT:{
				return std::pair<FuelDamage,KnockoutOccurs>{0.25f,false};
			}break;
		}
	};
	const auto GetSoftSurfaceCrashResult=[&crashSpeed](){
		switch(crashSpeed){
			case MAX:{
				return std::pair<FuelDamage,KnockoutOccurs>{0.5f,false};
			}break;
			case MEDIUM:{
				return std::pair<FuelDamage,KnockoutOccurs>{0.25f,false};
			}break;
			case LIGHT:{
				return std::pair<FuelDamage,KnockoutOccurs>{0.25f,false};
			}break;
		}
	};

	switch(type){
		case ROCK:{
			return GetHardSurfaceCrashResult();
		}break;
		case GRASS:{
			return GetMediumSurfaceCrashResult();
		}break;
		case SAND:{
			return GetSoftSurfaceCrashResult();
		}break;
		case SWAMP:{
			return GetSoftSurfaceCrashResult();
		}break;
		case LAVA:{
			return GetSoftSurfaceCrashResult();
		}break;
		case SHORE:{
			return GetMediumSurfaceCrashResult();
		}break;
		case OCEAN:{
			return GetMediumSurfaceCrashResult();
		}break;
		case FOREST:{
			return GetMediumSurfaceCrashResult();
		}break;
		case TUNNEL:{
			return GetHardSurfaceCrashResult();
		}break;
		case ICE:{
			return GetHardSurfaceCrashResult();
		}break;
		default:{
			return GetMediumSurfaceCrashResult();
		}
	}
}
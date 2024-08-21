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
#pragma once
#include <string>

class Terrain{
public:
    enum class SolidType{
        SOLID=true,
        WALKABLE=false,
    };
    #undef VOID
    enum TerrainType{
        VOID,
        ROCK,
        GRASS,
        SAND,
        SWAMP,
        LAVA,
        SHORE,
        OCEAN,
        FOREST,
        TUNNEL,
        ICE,
    };
    enum Direction{
        NORTH,
        EAST,
        SOUTH,
        WEST,
    };
    enum CrashSpeed{
        MAX,
        MEDIUM,
        LIGHT,
    };
    using FuelDamage=float;
    using KnockoutOccurs=bool;
    static const std::string TerrainToString(const TerrainType type);
    static const std::pair<FuelDamage,KnockoutOccurs>GetFuelDamageTakenAndKnockoutEffect(const TerrainType type,const CrashSpeed crashSpeed);

public:
    Terrain();
    Terrain(SolidType solid,TerrainType type,Direction facingDirection);
    SolidType solid;
    TerrainType type;
    Direction facing;
};
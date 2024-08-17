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
		h.animations.UpdateState(h.internalAnimState,fElapsedTime);
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
		const Animate2D::Frame&img{h.GetCurrentAnimation()};
		tv.DrawPartialRotatedDecal(h.pos,img.GetSourceImage()->Decal(),h.rot,img.GetSourceRect().size/2,img.GetSourceRect().pos,img.GetSourceRect().size);
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
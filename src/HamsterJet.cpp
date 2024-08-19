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
	switch(state){
		case SWOOP_DOWN:{
			HamsterGame::Game().SetZoom(1.5f);
			z=util::lerp(0.f,3.f,std::pow(timer/3.f,2));
			vf2d originalPos{hamster.GetPos().x-128.f,hamster.GetPos().y+32.f};
			pos=hamster.GetPos().lerp(originalPos,std::pow(timer/3.f,4));
			if(timer<=0.4f){
				hamster.SetPos(hamsterOriginalPos-vf2d{0.f,sin(float(geom2d::pi)*timer/0.4f)*8.f});
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
			pos=targetPos.lerp(originalPos,std::sqrt(timer/3.f));
			z=util::lerp(targetZ,0.f,timer/3.f);
			hamster.SetPos(pos);
			hamster.SetZ(z+0.01f);
			if(timer<=0.f){
				state=PLAYER_CONTROL;
				HamsterGame::Game().SetZoom(1.f);
			}
		}break;
	}
}
void HamsterJet::Draw(){
	HamsterGame::Game().SetZ(z);
	HamsterGame::Game().tv.DrawPartialRotatedDecal(pos,jet.Decal(),0.f,{24,24},{},{48,48});
	const Animate2D::FrameSequence&lightAnim{HamsterGame::Game().GetAnimation("hamster_jet.png",HamsterGame::AnimationState::JET_LIGHTS)};
	const Animate2D::Frame&lightFrame{lightAnim.GetFrame(HamsterGame::Game().GetRuntime())};
	HamsterGame::Game().tv.DrawPartialRotatedDecal(pos,lights.Decal(),0.f,lightFrame.GetSourceRect().size/2.f,lightFrame.GetSourceRect().pos,lightFrame.GetSourceRect().size);
	HamsterGame::Game().SetZ(0.f);
}
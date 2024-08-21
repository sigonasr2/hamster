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

#include "Checkpoint.h"
#include "HamsterGame.h"
#include "Hamster.h"
#include "util.h"
std::vector<Checkpoint>Checkpoint::checkpoints;
Checkpoint::Checkpoint(const vf2d pos)
:pos(pos){
	animation.AddState(AnimationState::DEFAULT,HamsterGame::GetAnimation("checkpoint.png",AnimationState::DEFAULT));
	animation.AddState(AnimationState::CHECKPOINT_COLLECTED,HamsterGame::GetAnimation("checkpoint.png",AnimationState::CHECKPOINT_COLLECTED));
	animation.AddState(AnimationState::CHECKPOINT_CYCLING,HamsterGame::GetAnimation("checkpoint.png",AnimationState::CHECKPOINT_CYCLING));
}
void Checkpoint::Initialize(const std::vector<vf2d>&checkpointPositions){
	checkpoints.clear();
	for(const vf2d&checkpointPos:checkpointPositions){
		checkpoints.emplace_back(checkpointPos);
	}
}
void Checkpoint::UpdateCheckpoints(const float fElapsedTime){
	for(Checkpoint&checkpoint:checkpoints){
		checkpoint.animation.UpdateState(checkpoint.internal_animState,fElapsedTime);
		if(checkpoint.collectedByPlayerTimer>0.f){
			checkpoint.collectedByPlayerTimer-=fElapsedTime;
			if(checkpoint.collectedByPlayerTimer<=0.f){
				checkpoint.animation.ChangeState(checkpoint.internal_animState,AnimationState::CHECKPOINT_COLLECTED);
			}
		}
	}
	std::sort(checkpoints.begin(),checkpoints.end(),[](const Checkpoint&c1,const Checkpoint&c2){
		return geom2d::line<float>(Hamster::GetPlayer().GetPos(),c1.pos).length()>geom2d::line<float>(Hamster::GetPlayer().GetPos(),c2.pos).length();
	});
}
void Checkpoint::DrawCheckpoints(TransformedView&tv){
	for(Checkpoint&checkpoint:checkpoints){
		const Animate2D::Frame&frame{checkpoint.animation.GetFrame(checkpoint.internal_animState)};
		if(checkpoint.animation.GetState(checkpoint.internal_animState)!=AnimationState::DEFAULT){
			HamsterGame::Game().SetDecalMode(DecalMode::ADDITIVE);
			HamsterGame::Game().SetZ(0.007f);
			tv.DrawPartialRotatedDecal(checkpoint.pos,frame.GetSourceImage()->Decal(),0.f,frame.GetSourceRect().size/2,frame.GetSourceRect().pos,frame.GetSourceRect().size,{1.1f,1.1f});
			HamsterGame::Game().SetZ(0.009f);
			HamsterGame::Game().SetDecalMode(DecalMode::NORMAL);
		}
		HamsterGame::Game().SetZ(0.009f);
		tv.DrawPartialRotatedDecal(checkpoint.pos,frame.GetSourceImage()->Decal(),0.f,frame.GetSourceRect().size/2,frame.GetSourceRect().pos,frame.GetSourceRect().size);
		geom2d::line<float>playerToCheckpointLine{geom2d::line<float>(Hamster::GetPlayer().GetPos(),checkpoint.pos)};
		
		if(!Hamster::GetPlayer().HasCollectedCheckpoint(checkpoint)){
			const float screenDistance{playerToCheckpointLine.length()*(1.325f/(HamsterGame::Game().GetCameraZ()))};
			if(screenDistance>226){
				const vf2d dirVec{playerToCheckpointLine.vector().norm()};
				const float dir{dirVec.polar().y};
				std::optional<vf2d>projCircle{geom2d::project(geom2d::circle<float>({},16),HamsterGame::SCREEN_FRAME,geom2d::ray<float>(HamsterGame::SCREEN_FRAME.middle(),dirVec))};
				if(projCircle.has_value()){
					Pixel arrowCol{PixelLerp(GREEN,BLACK,std::clamp((screenDistance-226)/1000.f,0.f,1.f))};
					uint8_t iconAlpha{uint8_t(util::lerp(255.f,0.f,std::clamp((screenDistance-226)/1000.f,0.f,1.f)))};
					HamsterGame::Game().DrawPartialRotatedDecal(projCircle.value(),HamsterGame::GetGFX("checkpoint.png").Decal(),0.f,{64,64},{},{128,128},{0.125f,0.125f},{255,255,255,iconAlpha});
					HamsterGame::Game().DrawRotatedDecal(projCircle.value(),HamsterGame::GetGFX("checkpoint_arrow.png").Decal(),dir,HamsterGame::GetGFX("checkpoint_arrow.png").Sprite()->Size()/2,{1.f,1.f},arrowCol);
				}
			}
		}
	}
}
std::vector<Checkpoint>&Checkpoint::GetCheckpoints(){
	return checkpoints;
}
const vf2d&Checkpoint::GetPos()const{
	return pos;
}
void Checkpoint::OnCheckpointCollect(){
	collectedByPlayerTimer=2.f;
	animation.ChangeState(internal_animState,AnimationState::CHECKPOINT_CYCLING);
}
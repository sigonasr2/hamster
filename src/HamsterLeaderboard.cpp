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
#include "HamsterLeaderboard.h"
#include "Checkpoint.h"
#include "Hamster.h"
#include <ranges>

void HamsterLeaderboard::OnRaceStart(){
	hamsterRanking.clear();
	for(Hamster&hamster:Hamster::GetHamsters())hamsterRanking.emplace_back(hamster);
}

HamsterLeaderboard::HamsterRanking::HamsterRanking(Hamster&hamsterRef)
:hamster(hamsterRef){}

void HamsterLeaderboard::Update(){
	for(HamsterRanking&hamsterRank:hamsterRanking){
		Hamster&hamster{hamsterRank.hamster.get()};
		float finalRanking{float(hamster.GetCheckpointsCollectedCount())};
		std::optional<std::reference_wrapper<Checkpoint>>closestCheckpoint;
		for(Checkpoint&cp:Checkpoint::GetCheckpoints()){
			float distToHamster{geom2d::line<float>(hamster.GetPos(),cp.GetPos()).length()};
			if(hamster.HasCollectedCheckpoint(cp))continue;
			if(!closestCheckpoint.has_value())closestCheckpoint=cp;
			else if(distToHamster<geom2d::line<float>(hamster.GetPos(),closestCheckpoint.value().get().GetPos()).length())closestCheckpoint=cp;
		}
		if(closestCheckpoint.has_value()){
			vf2d lastCollectedPos{};
			if(hamster.GetLastCollectedCheckpoint().has_value())lastCollectedPos=hamster.GetLastCollectedCheckpoint().value();
			else lastCollectedPos=HamsterGame::Game().GetMapSpawnRect().middle();
			vf2d closestCheckpointPos{closestCheckpoint.value().get().GetPos()};
			float totalDist{geom2d::line<float>(lastCollectedPos,closestCheckpointPos).length()};
			float distFromHamsterToClosestCheckpoint{geom2d::line<float>(hamster.GetPos(),closestCheckpointPos).length()};
			float additionalRanking{std::clamp((totalDist-distFromHamsterToClosestCheckpoint)/totalDist,0.0001f,0.9999f)};
			finalRanking+=additionalRanking;
		}
		hamsterRank.ranking=std::min(finalRanking,float(Checkpoint::GetCheckpoints().size()));
	}
	std::sort(hamsterRanking.begin(),hamsterRanking.end(),[](const HamsterRanking&rank1,const HamsterRanking&rank2){return rank1.ranking<rank2.ranking;});
}

void HamsterLeaderboard::Draw(HamsterGame&game){
	game.DrawRotatedDecal(HamsterGame::SCREEN_FRAME.pos+vf2d{8.f,HamsterGame::SCREEN_FRAME.size.y/2.f},game.GetGFX("raceprogress.png").Decal(),0.f,game.GetGFX("raceprogress.png").Sprite()->Size()/2,{1.f,1.f},WHITE);
	vf2d progressBarBottomPos{HamsterGame::SCREEN_FRAME.pos+vf2d{8.f,HamsterGame::SCREEN_FRAME.size.y/2.f+game.GetGFX("raceprogress.png").Sprite()->height/2.f}};
	for(int i:std::ranges::iota_view(0U,Checkpoint::GetCheckpoints().size()-1)){
		game.FillRectDecal(progressBarBottomPos+vf2d{-5.f,-float((game.GetGFX("raceprogress.png").Sprite()->height/(Checkpoint::GetCheckpoints().size()))*(i+1))},{10.f,1.f},WHITE);
	}
	int playerPlacement{};
	std::optional<std::reference_wrapper<HamsterRanking>>playerHamsterRanking{};
	for(int placement{};HamsterRanking&ranking:hamsterRanking){
		if(ranking.hamster.get().IsPlayerControlled){
			playerHamsterRanking=ranking;
			playerPlacement=hamsterRanking.size()-placement;
			game.DrawPartialRotatedDecal(progressBarBottomPos+vf2d{0.f,-(float(ranking.ranking)/Checkpoint::GetCheckpoints().size())*game.GetGFX("raceprogress.png").Sprite()->height},ranking.hamster.get().GetCurrentAnimation().GetSourceImage()->Decal(),0.f,{8.f,6.f},{64.f,64.f},{16.f,12.f});
		}else game.DrawPartialRotatedDecal(progressBarBottomPos+vf2d{0.f,-(float(ranking.ranking)/Checkpoint::GetCheckpoints().size())*game.GetGFX("raceprogress.png").Sprite()->height},ranking.hamster.get().GetCurrentAnimation().GetSourceImage()->Decal(),0.f,{8.f,6.f},{64.f,64.f},{16.f,12.f});
		placement++;
	}
	
	if(playerHamsterRanking.has_value()){
		std::string addonStr{"th"};
		if(playerPlacement==1)addonStr="st";
		else if(playerPlacement==2)addonStr="nd";
		std::string placementStr{std::format("{}{}",playerPlacement,addonStr)};
		vi2d placementStrSize{game.GetTextSizeProp(placementStr)};
		Pixel blinkCol{DARK_RED};
		if(playerPlacement==1)blinkCol=CYAN;
		else if(playerPlacement<=3)blinkCol=DARK_GREEN;
		for(int y:std::ranges::iota_view(-1,2)){
			for(int x:std::ranges::iota_view(-1,2)){
				game.DrawRotatedStringPropDecal(progressBarBottomPos+vf2d{-4.f,8.f}+vi2d{x,y},placementStr,0.f,{},BLACK,{3.f,3.f});
			}
		}
		game.DrawRotatedStringPropDecal(progressBarBottomPos+vf2d{-4.f,8.f},placementStr,0.f,{},blinkCol,{3.f,3.f});
	}
}

const float HamsterLeaderboard::HamsterRanking::GetRanking()const{
	return ranking;
}
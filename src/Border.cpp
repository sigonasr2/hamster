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
#include "Border.h"
#include "HamsterGame.h"
#include <ranges>

const std::unordered_map<Border::BorderTemplate,std::tuple<std::string,Border::CycleTimer,Border::CycleTimer,Border::CycleTimer>>Border::BORDER_TEMPLATES{
	{Border::DEFAULT,{"border.png",{{{170,97,124},{114,109,163}},0.6f},{{{252,119,144}},1.2f},{{{231,129,97}},3.6f}}},
};

Border::CycleTimer::CycleTimer(const std::vector<Pixel>cycle,const float repeatTime)
:cycle(cycle),repeatTime(repeatTime),currentTime(repeatTime),borderParent(nullptr){}

void Border::ChangeBorder(const Border::BorderTemplate borderTemplate){
	const std::tuple<std::string,Border::CycleTimer,Border::CycleTimer,Border::CycleTimer>&borderData{BORDER_TEMPLATES.at(borderTemplate)};
	decorative=std::get<1>(borderData);
	highlight=std::get<2>(borderData);
	background=std::get<3>(borderData);
	decorative.borderParent=this;
	highlight.borderParent=this;
	background.borderParent=this;
	img.Create(HamsterGame::GetGFX(std::get<0>(borderData)).Sprite()->width,HamsterGame::GetGFX(std::get<0>(borderData)).Sprite()->height);
	for(int y:std::ranges::iota_view(0,img.Sprite()->height)){
		for(int x:std::ranges::iota_view(0,img.Sprite()->width)){
			img.Sprite()->SetPixel(x,y,HamsterGame::GetGFX(std::get<0>(borderData)).Sprite()->GetPixel(x,y));
		}
	}
	img.Decal()->Update();
}

void Border::CycleTimer::Update(const float fElapsedTime){
	currentTime-=fElapsedTime;
	if(currentTime<=0.f){
		const Pixel currentCol{cycle[currentInd]};
		currentInd=(currentInd+1)%cycle.size();
		const Pixel newCol{cycle[currentInd]};
		for(int y:std::ranges::iota_view(0,borderParent->img.Sprite()->height)){
			for(int x:std::ranges::iota_view(0,borderParent->img.Sprite()->width)){
				if(borderParent->img.Sprite()->GetPixel(x,y)==currentCol){
					borderParent->img.Sprite()->SetPixel(x,y,newCol);
				}
			}
		}
		borderParent->img.Decal()->Update();
		currentTime+=repeatTime;
	}
}

void Border::Update(const float fElapsedTime){
	decorative.Update(fElapsedTime);
	highlight.Update(fElapsedTime);
	background.Update(fElapsedTime);
}
void Border::Draw(){
	HamsterGame::Game().DrawDecal({},img.Decal());
}

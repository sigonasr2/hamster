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
#include "FloatingText.h"
#include "HamsterGame.h"
#include "util.h"
#include <ranges>

std::vector<FloatingText>FloatingText::floatingText;

FloatingText::FloatingText(const vf2d&pos,const std::string&text,const std::vector<Pixel>&colorCycle,const vf2d&scale)
:pos(pos),text(text),colorCycle(colorCycle),lifetime(4.f),scale(scale){}
void FloatingText::UpdateFloatingText(const float fElapsedTime){
	for(FloatingText&floatingText:FloatingText::floatingText){
		floatingText.Update(fElapsedTime);
	}
}
void FloatingText::Update(const float fElapsedTime){
	lifetime-=fElapsedTime;
	lastColorChange-=fElapsedTime;
	if(lastColorChange<=0.f){
		lastColorChange=0.5f;
		colorCycleInd++;
	}
	if(lifetime>1.f){
		pos.y-=16.f*fElapsedTime;
	}
}
void FloatingText::DrawFloatingText(TransformedView&tv){
	for(FloatingText&floatingText:FloatingText::floatingText){
		floatingText.Draw(tv);
	}
	std::erase_if(floatingText,[](const FloatingText&floatingText){return floatingText.lifetime<=0.f;});
}
void FloatingText::Draw(TransformedView&tv){
	const Pixel&currentCol{colorCycle[colorCycleInd%colorCycle.size()]};

	const vf2d strSize{HamsterGame::Game().GetTextSize(text)};

	uint8_t alpha{uint8_t(util::lerp(0U,255U,lifetime/4.f))};

	for(int y:std::ranges::iota_view(-1,2)){
		for(int x:std::ranges::iota_view(-1,2)){
			tv.DrawRotatedStringDecal(pos+vi2d{x,y},text,0.f,strSize/2,{0,0,0,alpha},scale);
		}
	}
	HamsterGame::Game().SetZ(0.01f);
	tv.DrawRotatedStringDecal(pos,text,0.f,strSize/2,{currentCol.r,currentCol.g,currentCol.b,uint8_t(currentCol.a*(alpha/255.f))},scale);
	HamsterGame::Game().SetZ(0.f);
}
void FloatingText::CreateFloatingText(const vf2d&pos,const std::string&text,const std::vector<Pixel>&colorCycle,const vf2d&scale){
	floatingText.emplace_back(pos,text,colorCycle,scale);
}
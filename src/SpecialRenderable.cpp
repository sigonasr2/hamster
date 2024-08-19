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

#include "SpecialRenderable.h"
#include "HamsterGame.h"
#include <ranges>

SpecialRenderable::SpecialRenderable(){};

void SpecialRenderable::Initialize(std::string_view imgName,const Pixel overrideCol,const Pixel matrixCol){
	IsInitialized=true;
	originalImgName=imgName;
	this->overrideCol=overrideCol;
	this->matrixCol=matrixCol;
	const Renderable&refImg{HamsterGame::GetGFX(imgName)};
	modifiedImg.Create(refImg.Sprite()->width,refImg.Sprite()->height);
	HamsterGame::Game().SetPixelMode(Pixel::ALPHA);
	for(int y:std::ranges::iota_view(0,refImg.Sprite()->height)){
		for(int x:std::ranges::iota_view(0,refImg.Sprite()->width)){
			modifiedImg.Sprite()->SetPixel(x,y,refImg.Sprite()->GetPixel(x,y));
		}
	}
	modifiedImg.Decal()->Update();
	Update(0.1f);
	HamsterGame::Game().SetPixelMode(Pixel::MASK);
}

void SpecialRenderable::Update(const float fElapsedTime){
	if(!IsInitialized)throw std::runtime_error{std::format("SpecialRenderable for {} is not properly initialized!",originalImgName)};
	lastPixelsUpdateTimer-=fElapsedTime;
	HamsterGame::Game().SetDrawTarget(modifiedImg.Sprite());
	HamsterGame::Game().SetPixelMode(Pixel::ALPHA);
	HamsterGame::Game().SetPixelBlend(0.5f);
	if(lastPixelsUpdateTimer<=0.f){
		lastPixelsUpdateTimer+=0.1f;
		for(int y:std::ranges::iota_view(0,modifiedImg.Sprite()->height)){
			for(int x:std::ranges::iota_view(0,modifiedImg.Sprite()->width)){
				if(HamsterGame::GetGFX(originalImgName).Sprite()->GetPixel(x,y)==overrideCol){
					modifiedImg.Sprite()->SetPixel(x,y,HamsterGame::GetGFX("MATRIX_TEXTURE").Sprite()->GetPixel(x,y));
					HamsterGame::Game().Draw({x,y},matrixCol);
				}
			}
		}
	}
	HamsterGame::Game().SetDrawTarget(nullptr);
	HamsterGame::Game().SetPixelMode(Pixel::MASK);
	HamsterGame::Game().SetPixelBlend(1.f);
	modifiedImg.Decal()->Update();
}
const Renderable&SpecialRenderable::Get()const{
	if(!IsInitialized)throw std::runtime_error{std::format("SpecialRenderable for {} is not properly initialized!",originalImgName)};
	return modifiedImg;	
}
void SpecialRenderable::ChangeMatrixColor(const Pixel newMatrixCol){
	matrixCol=newMatrixCol;
}
Decal*SpecialRenderable::Decal()const{
	return modifiedImg.Decal();
}
Sprite*SpecialRenderable::Sprite()const{
	return modifiedImg.Sprite();
}
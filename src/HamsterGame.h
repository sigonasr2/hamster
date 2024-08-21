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
#include <unordered_map>
#include "olcUTIL_Geometry2D.h"
#include "olcUTIL_Animate2D.h"
#include "olcPGEX_TransformedView.h"
#include "olcUTIL_Camera2D.h"
#include "Border.h"
#include "TMXParser.h"
#include "TSXParser.h"
#include "Terrain.h"
#include "SpecialRenderable.h"
#include "olcPGEX_Graphics3D.h"
#include "AnimationState.h"

struct Letter{
	vf2d pos;
	float spd;
	char c;
};

class HamsterGame : public olc::PixelGameEngine
{
	const static std::string ASSETS_DIR;
public:
	HamsterGame()=delete;
	HamsterGame(const std::string&appName);
	static geom2d::rect<float>SCREEN_FRAME;
	TransformedView tv{};
public:
	bool OnUserCreate()override final;
	bool OnUserUpdate(float fElapsedTime)override final;
	bool OnUserDestroy()override final;

	static const Renderable&GetGFX(const std::string&img);
	static const Animate2D::Animation<AnimationState::AnimationState>&GetAnimations(const std::string&img);
	static const Animate2D::FrameSequence&GetAnimation(const std::string&img,const AnimationState::AnimationState state);
	static HamsterGame&Game();
	static std::unordered_map<uint32_t,Animate2D::FrameSequence>ANIMATED_TILE_IDS;
	const double GetRuntime()const;
	const Terrain::TerrainType GetTerrainTypeAtPos(const vf2d pos)const;
	const bool IsTerrainSolid(const vf2d pos)const;
	void SetZoom(const float zoom);
	const float GetZoom()const;
	const bool IsInBounds(const vf2d pos)const;
private:
	void UpdateGame(const float fElapsedTime);
	void DrawGame();
	void LoadGraphics();
	void LoadAnimations();
	void LoadLevel(const std::string&mapName);
	void _LoadImage(const std::string&img);
	static std::unordered_map<std::string,Renderable>GFX;
	static std::unordered_map<std::string,Animate2D::Animation<AnimationState::AnimationState>>ANIMATIONS;
	static HamsterGame*self;
	Border border;
	void DrawLevelTiles();
	std::optional<TMXParser>currentMap;
	std::optional<TSXParser>currentTileset;
	double runTime{};
	Camera2D camera;
	Renderable mapImage;
	void UpdateMatrixTexture();
	float matrixTimer;
	std::vector<Letter>activeLetters;
	float updatePixelsTimer;
	Renderable animatedWaterTile;
	void UpdateWaterTexture();
	GFX3D::PipeLine renderer;
	virtual void Apply3DTransform(std::vector<DecalInstance>&decals)override final;
	float zoom{1.f}; //Increase to zoom out, decrease to zoom in (this is the overhead distance from the player).
	GFX3D::vec3d vUp{0,-1,0};
	GFX3D::vec3d vEye{0.f,0,1};
	GFX3D::vec3d vLookDir{0,0,-1};
	const float fLazyFollowRate{4.0f};
	vf2d cloudSpd{};
	vf2d cloudOffset{};
	float speedometerDisplayAmt{0.f};
};
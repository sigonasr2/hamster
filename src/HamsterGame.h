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
#include "olcPGEX_Graphics3D.h"
#include "AnimationState.h"
#include "olcPGEX_Viewport.h"
#include "Difficulty.h"
#include "olcPGEX_MiniAudio.h"
#include "HamsterNet.h"
#include "olcPGEX_SplashScreen.h"

struct Letter{
	vf2d pos;
	float spd;
	char c;
};

class HamsterGame : public olc::PixelGameEngine
{
public:
	enum class GameMode{
		GRAND_PRIX_1,
		GRAND_PRIX_2,
		GRAND_PRIX_3,
		SINGLE_RACE,
		MARATHON,
	};
	const static std::string ASSETS_DIR;
	const static int UNPLAYED;
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
	static std::unordered_map<std::string,int>mapPBs;
	static HamsterGame&Game();
	static std::unordered_map<uint32_t,Animate2D::FrameSequence>ANIMATED_TILE_IDS;
	const double GetRuntime()const;
	const Terrain::TerrainType GetTerrainTypeAtPos(const vf2d pos)const;
	const bool IsTerrainSolid(const vf2d pos)const;
	void SetZoom(const float zoom);
	const float GetZoom()const;
	const bool IsInBounds(const vf2d pos)const;
	const float GetCameraZ()const;
	const std::unordered_map<TunnelId,Tunnel>&GetTunnels()const;
	const Terrain::Direction&GetTileFacingDirection(const vf2d worldPos)const;
	const std::string&GetCurrentMapName()const;
	virtual void OnTextEntryComplete(const std::string& sText)override;
	const Difficulty&GetMapDifficulty()const;
	void OnPlayerFinishedRace();
	const GameMode GetGameMode();
	static void SavePB(const std::string&mapName,int ms);
	static void LoadPBs();
	const int GetRaceTime();
	const bool RaceCountdownCompleted();
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
	void SetupAndStartRace();
	std::optional<TMXParser>currentMap;
	std::optional<TSXParser>currentTileset;
	double runTime{};
	Camera2D camera;
	Renderable mapImage;
	std::vector<Letter>activeLetters;
	float updatePixelsTimer;
	GFX3D::PipeLine renderer;
	virtual void Apply3DTransform(std::vector<DecalInstance>&decals)override final;
	float zoom{1.f}; //Increase to zoom out, decrease to zoom in (this is the overhead distance from the player).
	GFX3D::vec3d vUp{0,-1,0};
	GFX3D::vec3d vEye{0.f,0,2.f};
	GFX3D::vec3d vLookDir{0,0,-1};
	const float fLazyFollowRate{4.0f};
	vf2d cloudSpd{};
	vf2d cloudOffset{};
	float speedometerDisplayAmt{0.f};
	ViewPort radar;
	void DrawRadar();
	float radarScale{36.f};
	std::vector<Renderable>waterTiles;
	std::string currentMapName;
	MiniAudio audio;
	std::unordered_map<std::string,int>bgm;
	GameMode mode{GameMode::SINGLE_RACE};
	HamsterNet net;
	float countdownTimer{};
	#ifndef __EMSCRIPTEN__
	#ifndef __DEBUG__
	SplashScreen splash;
	#endif
	#endif
	bool netInitialized{false};
	std::vector<std::string>mapNameList{
		"StageI.tmx",	
		"StageII.tmx",
		"StageIII.tmx",
		"StageIV.tmx",
		"StageV.tmx",
		"StageVI.tmx",
		"StageVII.tmx",
		"StageVIII.tmx",
		"StageIX.tmx",
		"StageX.tmx",
		"StageXI.tmx",
		"StageXII.tmx",
		"Grand Prix I",
		"Grand Prix II",
		"Grand Prix III",
	};
	std::string emscripten_temp_val{"123456"};
	std::vector<std::string>hamsterColorNames{
		"Yellow",
		"Pink",
		"Cyan",
		"Black",
		"Green",
		"Purple"
		"Red",
		"Blue",
	};
};
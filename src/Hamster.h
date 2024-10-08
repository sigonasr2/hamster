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

#include <vector>
#include "olcUTIL_Geometry2D.h"
#include "olcUTIL_Animate2D.h"
#include "Terrain.h"
#include <unordered_set>
#include "HamsterJet.h"
#include "HamsterGame.h"
#include "Checkpoint.h"
#include "HamsterAI.h"

using Timer=float;

class Hamster{
	friend class HamsterJet;
	enum PlayerControlled{
		PLAYER_CONTROLLED=true,
		NPC=false,
	};

	enum HamsterState{
		NORMAL,
		BUMPED,
		DROWNING,
		FLYING,
		WAIT,
		BURNING,
		KNOCKOUT,
		BURROWING,
		BURROW_WAIT,
		SURFACING,
	};

	static std::vector<Hamster>HAMSTER_LIST;

	static const uint8_t MAX_HAMSTER_COUNT;
	static const uint8_t NPC_HAMSTER_COUNT;

	static const std::vector<std::string>NPC_HAMSTER_IMAGES;
	static std::string PLAYER_HAMSTER_IMAGE;

	const float DEFAULT_MAX_SPD{128.f};
	const float DEFAULT_TIME_TO_MAX_SPD{0.3f};
	const float DEFAULT_FRICTION{400.f};
	const float DEFAULT_TURN_SPD{2.f*geom2d::pi};
	const float DEFAULT_BUMP_AMT{100.f};
	const float DEFAULT_DROWN_TIME{5.f};
	const float DEFAULT_BURN_TIME{0.3f};

	vf2d pos;
	vf2d vel;
	std::optional<vf2d>lastSafeLocation{};
	float z{};
	float lastSafeLocationTimer{};
	float rot{};
	float targetRot{};
	bool frictionEnabled{false};
	float collisionRadius{12.f};
	float bumpTimer{};
	float waitTimer{};
	double distanceTravelled{};
	float drownTimer{};
	float burnTimer{};
	float imgScale{1.f};
	Pixel shrinkEffectColor{BLACK};
	Animate2D::Animation<AnimationState::AnimationState>animations;
	Animate2D::AnimationState internalAnimState;
	static std::optional<Hamster*>playerHamster;
	HamsterState state{NORMAL};
	std::unordered_set<Powerup::PowerupType>powerups;
	std::optional<HamsterJet>hamsterJet;
	float lastTappedSpace{0.f};
	float drawingOffsetY{0.f};
	float readyFlashTimer{};
	float jetFuel{0.f};
	float jetFuelDisplayAmt{0.f};
	float knockoutTimer{0.f};
	std::set<vf2d>checkpointsCollected;
	float raceFinishAnimTimer{0.f};
	std::pair<Timer,vf2d>collectedCheckpointInfo{INFINITY,{}};
	float burrowTimer{};
	float burrowImgShrinkTimer{};
	uint16_t enteredTunnel{};
	HamsterAI ai;
	float aiNodeTime{0.f};
	std::optional<vf2d>temporaryNode;
	int randomId{};
	uint8_t boostCounter{};
	float boostTimer{};
	float canCollectWheelPowerupTimer{};
	float SEARCH_RANGE{1.f};
	std::string colorFilename;
	int points{};
	std::optional<int>finishedRaceTime;
	std::optional<vf2d>lastObtainedCheckpointPos;
	HamsterAI::AIType aiLevel{HamsterAI::AIType::NORMAL};
	float lastFootstep{};
	float lastCollisionSound{};
public:
	Hamster(const vf2d spawnPos,const std::string&img,const PlayerControlled IsPlayerControlled=NPC);
	static const Hamster&GetPlayer();
	static void UpdateHamsters(const float fElapsedTime);
	static void CreateHamsters(const HamsterGame::GameMode mode);
	static void MoveHamstersToSpawn(const geom2d::rect<int>startingLoc);
	static void DrawHamsters(TransformedView&tv);
	static void DrawOverlay();
	const Animate2D::Frame&GetCurrentAnimation()const;
	const vf2d&GetPos()const;
	const float&GetZ()const;
	void HandlePlayerControls();
	void HandleAIControls();
	void TurnTowardsTargetDirection();
	void MoveHamster();
	void HandleCollision();
	const float GetRadius()const;
	const Terrain::TerrainType GetTerrainStandingOn()const;
	const bool IsTerrainStandingOnSolid()const;
	const float GetTimeToMaxSpeed()const;
	const float GetMaxSpeed()const;
	const float GetFriction()const;
	const float GetTurnSpeed()const;
	const float GetBumpAmount()const;
	void ObtainPowerup(const Powerup::PowerupType powerup);
	const bool HasPowerup(const Powerup::PowerupType powerup)const;
	void RemoveAllPowerups();
	const bool IsLethalTerrain(const vf2d pos)const;
	const bool IsSolidTerrain(const vf2d pos)const;
	const bool IsDrowning()const;
	const bool IsBurning()const;
	const float GetDrownRatio()const;
	const float GetBurnRatio()const;
	void SetPos(const vf2d pos);
	void SetZ(const float z);
	static void OnUserDestroy();
	void SetDrawingOffsetY(const float offsetY);
	const vf2d GetNearestSafeLocation()const;
	void SetJetFuel(const float amt);
	void Knockout();
	const float GetSpeed()const;
	const Terrain::TerrainType GetTerrainHoveringOver()const;
	void SetState(const HamsterState state);
	const bool CollectedAllCheckpoints()const;
	const bool HasCollectedCheckpoint(const Checkpoint&cp)const;
	static std::vector<Hamster>&GetHamsters();
	const HamsterState&GetState()const;
	const bool BurnedOrDrowned()const;
	const bool CanMove()const;
	const bool FlyingInTheAir()const;
	const float GetAILandingSpeed()const;
	const float GetAIAdjustNodeTime()const;
	const float GetAINodeDistanceVariance()const;
	const vf2d GetAINodePositionVariance()const;
	const bool IsBurrowed()const;
	const size_t GetCheckpointsCollectedCount()const;
	const std::optional<vf2d>GetLastCollectedCheckpoint()const;
	PlayerControlled IsPlayerControlled;
	const int GetFinishedTime()const;
	const std::string&GetHamsterImage()const;
	static void ClearHamsters();
};
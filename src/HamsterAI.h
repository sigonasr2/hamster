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
#include <string>
#include "olcUTIL_Geometry2D.h"

class HamsterAI{
public:
	class Action{
	public:
		enum ActionType{
			MOVE,
			COLLECT_POWERUP,
			LAUNCH_JET,
			LANDING,
			ENTER_TUNNEL,
			CHECKPOINT_COLLECTED,
			BOOST,
		};
		vi2d pos;
		ActionType type;
	};
	enum AIType{
		SMART,
		NORMAL,
		DUMB,
		END, //NOTE: Not an AI type, just used for iteration detection.
	};
	using ActionOptRef=std::optional<std::reference_wrapper<HamsterAI::Action>>;
	const ActionOptRef GetCurrentAction();
	const ActionOptRef AdvanceToNextAction();
	const ActionOptRef GetPreviousAction();
	const ActionOptRef RevertToPreviousAction();
	const ActionOptRef PeekNextAction();
	const AIType GetAIType()const;
	void LoadAI(const std::string&mapName,int numb);

	static void OnMove(const vi2d pos);
	static void OnPowerupCollection(const vi2d pos);
	static void OnJetLaunch(const vi2d pos);
	static void OnJetBeginLanding(const vi2d pos);
	static void OnTunnelEnter(const vi2d pos);
	static void OnCheckpointCollected(const vi2d pos);
	static void OnBoost(const vi2d pos);

	static void OnTextEntryComplete(const std::string&enteredText);

	static void Update(const float fElapsedTime);
	static void DrawOverlay();
	static const bool IsRecording();
private:
	static bool recordingMode;
	static std::vector<Action>recordedActions;
	std::vector<Action>actionsToPerform;
	size_t actionInd{};
	static std::optional<vi2d>lastTileWalkedOn; //In World Coords
	AIType type{END};
};
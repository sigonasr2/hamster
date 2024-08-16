#include "HamsterGame.h"

geom2d::rect<float>HamsterGame::SCREEN_FRAME{{96,0},{320,288}};

HamsterGame::HamsterGame()
{
	sAppName = "Project Hamster";
}

bool HamsterGame::OnUserCreate(){
	LoadAnimations();
	return true;
}

bool HamsterGame::OnUserUpdate(float fElapsedTime){
	DrawDecal({},
	gameWindow.FillRectDecal({},{150.f,150.f},WHITE);
	return true;
}

bool HamsterGame::OnUserDestroy(){
	
	return true;
}


int main()
{
	HamsterGame game;
	if(game.Construct(512, 288, 2, 2))
		game.Start();

	return 0;
}


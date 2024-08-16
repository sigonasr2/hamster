#include "HamsterGame.h"
#include <stdexcept>

geom2d::rect<float>HamsterGame::SCREEN_FRAME{{96,0},{320,288}};
std::unordered_map<std::string,Animate2D::Animation<int>>HamsterGame::ANIMATIONS;
std::unordered_map<std::string,Renderable>HamsterGame::GFX;
const std::string HamsterGame::ASSETS_DIR{"assets/"};

HamsterGame::HamsterGame()
{
	sAppName = "Project Hamster";
}

bool HamsterGame::OnUserCreate(){
	LoadGraphics();
	LoadAnimations();
	return true;
}

void HamsterGame::LoadGraphics(){

	#undef LoadImage
	auto LoadImage=[this](std::string_view img){
		GFX.insert({ASSETS_DIR+std::string(img),Renderable{}});
		rcode result{GFX[ASSETS_DIR+std::string(img)].Load(ASSETS_DIR+std::string(img))};
		if(result!=OK)throw std::runtime_error{std::format("Failed to Load Image {}. OLC Rcode: {}",img,int(result))};
	};

	LoadImage("border.png");
}

void HamsterGame::LoadAnimations(){
	
}

bool HamsterGame::OnUserUpdate(float fElapsedTime){
	DrawDecal({},GetGFX("border.png").Decal());
	gameWindow.FillRectDecal({},{500.f,150.f},WHITE);
	return true;
}

const Renderable&HamsterGame::GetGFX(const std::string_view img){
	if(!GFX.count(ASSETS_DIR+std::string(img)))throw std::runtime_error{std::format("Image {} does not exist!",img)};
	return GFX[ASSETS_DIR+std::string(img)];
}
const Animate2D::Animation<int>&HamsterGame::GetAnimations(const std::string_view img){
	if(!ANIMATIONS.count(ASSETS_DIR+std::string(img)))throw std::runtime_error{std::format("Animations for {} does not exist!",img)};
	return ANIMATIONS[ASSETS_DIR+std::string(img)];
}

bool HamsterGame::OnUserDestroy(){
	ANIMATIONS.clear();
	GFX.clear();
	return true;
}

int main()
{
	HamsterGame game;
	if(game.Construct(512, 288, 2, 2))
		game.Start();

	return 0;
}


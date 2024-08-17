#include "HamsterGame.h"
#include "Hamster.h"
#include <stdexcept>

geom2d::rect<float>HamsterGame::SCREEN_FRAME{{96,0},{320,288}};
std::unordered_map<std::string,Animate2D::Animation<HamsterGame::AnimationState>>HamsterGame::ANIMATIONS;
std::unordered_map<std::string,Renderable>HamsterGame::GFX;
const std::string HamsterGame::ASSETS_DIR{"assets/"};

HamsterGame::HamsterGame()
{
	sAppName = "Project Hamster";
}

bool HamsterGame::OnUserCreate(){
	camera=Camera2D{SCREEN_FRAME.size};
	camera.SetMode(Camera2D::Mode::LazyFollow);
	tv.Initialise(SCREEN_FRAME.size,{1,1});
	tv.SetWorldOffset(-SCREEN_FRAME.pos);
	LoadGraphics();
	LoadAnimations();
	LoadLevel(); //THIS IS TEMPORARY.
	return true;
}

void HamsterGame::_LoadImage(const std::string_view img){
	GFX.insert({ASSETS_DIR+std::string(img),Renderable{}});
	rcode result{GFX[ASSETS_DIR+std::string(img)].Load(ASSETS_DIR+std::string(img))};
	if(result!=OK)throw std::runtime_error{std::format("Failed to Load Image {}. OLC Rcode: {}",img,int(result))};
}

void HamsterGame::LoadGraphics(){
	_LoadImage("border.png");
}

void HamsterGame::LoadAnimations(){
	auto LoadImageIfRequired=[this](const std::string_view img){if(!GFX.count(ASSETS_DIR+std::string(img)))_LoadImage(img);};
	auto LoadStillAnimation=[this,&LoadImageIfRequired](const AnimationState state,const std::string_view img){
		Animate2D::FrameSequence stillAnimation{0.f,Animate2D::Style::OneShot};
		LoadImageIfRequired(img);
		stillAnimation.AddFrame(Animate2D::Frame{&GetGFX(img),{{},GetGFX(img).Sprite()->Size()}});
		ANIMATIONS[ASSETS_DIR+std::string(img)].AddState(state,stillAnimation);
	};
	auto LoadAnimation=[this,&LoadImageIfRequired](const AnimationState state,const std::string_view img,const std::vector<vf2d>frames,const float frameDuration=0.1f,const Animate2D::Style style=Animate2D::Style::Repeat,vf2d frameSize={32,32}){
		Animate2D::FrameSequence newAnimation{frameDuration,style};
		LoadImageIfRequired(img);
		for(const vf2d&framePos:frames){
			newAnimation.AddFrame(Animate2D::Frame{&GetGFX(img),{framePos,frameSize}});
		}
		ANIMATIONS[ASSETS_DIR+std::string(img)].AddState(state,newAnimation);
	};

	LoadAnimation(DEFAULT,"hamster.png",{{0,32},{32,32}},0.3f);
}

void HamsterGame::LoadLevel(){
	const vf2d levelSpawnLoc{50,50}; //TEMPORARY
	Hamster::LoadHamsters(levelSpawnLoc);
	camera.SetTarget(Hamster::GetPlayer().GetPos());
}

void HamsterGame::UpdateGame(const float fElapsedTime){
	camera.Update(fElapsedTime);
	tv.SetWorldOffset(-SCREEN_FRAME.pos+camera.GetViewPosition());
	Hamster::UpdateHamsters(fElapsedTime);
}

void HamsterGame::DrawGame(){
	tv.FillRectDecal({10,10},{500.f,150.f},WHITE);
	Hamster::DrawHamsters(tv);
	DrawDecal({},GetGFX("border.png").Decal());
}

bool HamsterGame::OnUserUpdate(float fElapsedTime){
	UpdateGame(fElapsedTime);
	DrawGame();
	return true;
}

const Renderable&HamsterGame::GetGFX(const std::string_view img){
	if(!GFX.count(ASSETS_DIR+std::string(img)))throw std::runtime_error{std::format("Image {} does not exist!",img)};
	return GFX[ASSETS_DIR+std::string(img)];
}
const Animate2D::Animation<HamsterGame::AnimationState>&HamsterGame::GetAnimations(const std::string_view img){
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
	if(game.Construct(512, 288, 3, 3))
		game.Start();

	return 0;
}


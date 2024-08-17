#include "HamsterGame.h"
#include "Hamster.h"
#include <stdexcept>
#include <ranges>

geom2d::rect<float>HamsterGame::SCREEN_FRAME{{96,0},{320,288}};
std::unordered_map<std::string,Animate2D::Animation<HamsterGame::AnimationState>>HamsterGame::ANIMATIONS;
std::unordered_map<std::string,Renderable>HamsterGame::GFX;
const std::string HamsterGame::ASSETS_DIR{"assets/"};
HamsterGame*HamsterGame::self{nullptr};
std::unordered_map<uint32_t,Animate2D::FrameSequence>HamsterGame::ANIMATED_TILE_IDS;

HamsterGame::HamsterGame(){
	sAppName = "Project Hamster";
	HamsterGame::self=this;
}

bool HamsterGame::OnUserCreate(){
	camera=Camera2D{SCREEN_FRAME.size};
	camera.SetMode(Camera2D::Mode::LazyFollow);
	tv.Initialise(SCREEN_FRAME.size,{1,1});
	tv.SetWorldOffset(-SCREEN_FRAME.pos);
	LoadGraphics();
	LoadAnimations();
	LoadLevel("TestLevel.tmx"); //THIS IS TEMPORARY.

	border.ChangeBorder(Border::DEFAULT);
	return true;
}

void HamsterGame::_LoadImage(const std::string_view img){
	GFX.insert({ASSETS_DIR+std::string(img),Renderable{}});
	rcode result{GFX[ASSETS_DIR+std::string(img)].Load(ASSETS_DIR+std::string(img))};
	if(result!=OK)throw std::runtime_error{std::format("Failed to Load Image {}. OLC Rcode: {}",img,int(result))};
}

void HamsterGame::LoadGraphics(){
	_LoadImage("border.png");
	_LoadImage("gametiles.png");
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
	Animate2D::FrameSequence&waterAnimFrames{(*ANIMATED_TILE_IDS.insert({1384,Animate2D::FrameSequence{0.2f}}).first).second};
	for(vf2d&sourcePos:std::vector<vf2d>{{192+16*0,784},{192+16*1,784},{192+16*2,784},{192+16*3,784},{192+16*4,784},{192+16*5,784},{192+16*6,784},{192+16*7,784}}){
		waterAnimFrames.AddFrame(Animate2D::Frame{&GetGFX("gametiles.png"),{sourcePos,{16,16}}});
	}
	Animate2D::FrameSequence&lavaAnimFrames{(*ANIMATED_TILE_IDS.insert({1412,Animate2D::FrameSequence{0.2f}}).first).second};
	for(vf2d&sourcePos:std::vector<vf2d>{{192+16*0,800},{192+16*1,800},{192+16*2,800},{192+16*3,800},{192+16*4,800},{192+16*5,800},{192+16*6,800},{192+16*7,800},{192+16*8,800}}){
		lavaAnimFrames.AddFrame(Animate2D::Frame{&GetGFX("gametiles.png"),{sourcePos,{16,16}}});
	}
}

void HamsterGame::LoadLevel(const std::string_view mapName){
	const vf2d levelSpawnLoc{50,50}; //TEMPORARY

	currentMap=TMXParser{ASSETS_DIR+std::string(mapName)};
	currentTileset=TSXParser{ASSETS_DIR+std::string("Terrain.tsx")};

	Hamster::LoadHamsters(levelSpawnLoc);
	camera.SetTarget(Hamster::GetPlayer().GetPos());
}

void HamsterGame::UpdateGame(const float fElapsedTime){
	camera.Update(fElapsedTime);
	tv.SetWorldOffset(-SCREEN_FRAME.pos+camera.GetViewPosition());
	Hamster::UpdateHamsters(fElapsedTime);
	border.Update(fElapsedTime);
}

void HamsterGame::DrawGame(){
	DrawLevelTiles();
	Hamster::DrawHamsters(tv);
	border.Draw();
	DrawStringDecal(SCREEN_FRAME.pos+vf2d{1,1},"Terrain Type: "+Terrain::TerrainToString(Hamster::GetPlayer().GetTerrainStandingOn()),BLACK);
	DrawStringDecal(SCREEN_FRAME.pos,"Terrain Type: "+Terrain::TerrainToString(Hamster::GetPlayer().GetTerrainStandingOn()));
}

const Terrain::TerrainType HamsterGame::GetTerrainTypeAtPos(const vf2d pos)const{
	Terrain::TerrainType tileType{Terrain::VOID};
	if(pos.x<=0.f||pos.y<=0.f||pos.x>=currentMap.value().GetData().GetMapData().width*16||pos.y>=currentMap.value().GetData().GetMapData().height*16)return tileType;
	for(const LayerTag&layer:currentMap.value().GetData().GetLayers()){
		int tileX{int(floor(pos.x)/16)};
		int tileY{int(floor(pos.y)/16)};
		int tileID{layer.tiles[tileY][tileX]-1};
		if(tileID==-1)continue;
		if(currentTileset.value().GetData().GetTerrainData().count(tileID))tileType=currentTileset.value().GetData().GetTerrainData().at(tileID).second;
	}
	return tileType;
}

void HamsterGame::DrawLevelTiles(){
	for(const LayerTag&layer:currentMap.value().GetData().GetLayers()){
		for(float y=tv.GetWorldTL().y-16;y<=tv.GetWorldBR().y+16;y+=16){
			for(float x=tv.GetWorldTL().x-1+SCREEN_FRAME.pos.x;x<=tv.GetWorldBR().x+16+SCREEN_FRAME.pos.x;x+=16){
				if(x<=0.f||y<=0.f||x>=currentMap.value().GetData().GetMapData().width*16||y>=currentMap.value().GetData().GetMapData().height*16)continue;
				const int numTilesWide{GetGFX("gametiles.png").Sprite()->width/16};
				const int numTilesTall{GetGFX("gametiles.png").Sprite()->height/16};

				int tileX{int(floor(x)/16)};
				int tileY{int(floor(y)/16)};
				int tileID{layer.tiles[tileY][tileX]-1};
				if(tileID==-1)continue;

				int imgTileX{tileID%numTilesWide};
				int imgTileY{tileID/numTilesWide};
				if(ANIMATED_TILE_IDS.count(tileID)){
					Animate2D::FrameSequence&animatedTile{ANIMATED_TILE_IDS[tileID]};
					const Animate2D::Frame&currentFrame{animatedTile.GetFrame(runTime)};
					tv.DrawPartialDecal(vf2d{float(tileX),float(tileY)}*16,currentFrame.GetSourceImage()->Decal(),currentFrame.GetSourceRect().pos,currentFrame.GetSourceRect().size);
				}else{
					tv.DrawPartialDecal(vf2d{float(tileX),float(tileY)}*16,GetGFX("gametiles.png").Decal(),vf2d{float(imgTileX),float(imgTileY)}*16,{16,16});
				}
			}
		}
	}
}

bool HamsterGame::OnUserUpdate(float fElapsedTime){
	runTime+=fElapsedTime;
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

HamsterGame&HamsterGame::Game(){
	return *self;
}

const double HamsterGame::GetRuntime()const{
	return runTime;
}

int main()
{
	HamsterGame game;
	if(game.Construct(512, 288, 3, 3))
		game.Start();

	return 0;
}


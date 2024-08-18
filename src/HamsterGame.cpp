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
	LoadGraphics();
	LoadAnimations();
	currentTileset=TSXParser{ASSETS_DIR+std::string("Terrain.tsx")};
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
	_LoadImage("shadow.png");
	_LoadImage("drownmeter.png");
	_LoadImage("burnmeter.png");
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
	LoadAnimation(WHEEL_TOP,"hamster.png",{{0,96},{32,96}},0.1f);
	LoadAnimation(WHEEL_BOTTOM,"hamster.png",{{64,96},{96,96}},0.1f);
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

	Hamster::LoadHamsters(levelSpawnLoc);
	camera.SetTarget(Hamster::GetPlayer().GetPos());
	
	
	mapImage.Create(currentMap.value().GetData().GetMapData().width*16,currentMap.value().GetData().GetMapData().height*16);
	SetDrawTarget(mapImage.Sprite());
	Clear(BLANK);
	SetPixelMode(Pixel::MASK);

	#pragma region Detect powerup tiles
		std::vector<Powerup>mapPowerups;
		for(const LayerTag&layer:currentMap.value().GetData().GetLayers()){
			for(size_t y:std::ranges::iota_view(0U,layer.tiles.size())){
				for(size_t x:std::ranges::iota_view(0U,layer.tiles[y].size())){
					const int tileID{layer.tiles[y][x]-1};
					if(Powerup::TileIDIsUpperLeftPowerupTile(tileID))mapPowerups.emplace_back(vf2d{float(x),float(y)}*16+vf2d{16,16},Powerup::TileIDPowerupType(tileID));
					
					const int numTilesWide{GetGFX("gametiles.png").Sprite()->width/16};
					const int numTilesTall{GetGFX("gametiles.png").Sprite()->height/16};
					
					int imgTileX{tileID%numTilesWide};
					int imgTileY{tileID/numTilesWide};
					if(tileID==-1||Powerup::TileIDIsPowerupTile(tileID))continue;
					DrawPartialSprite(vf2d{float(x),float(y)}*16,GetGFX("gametiles.png").Sprite(),vf2d{float(imgTileX),float(imgTileY)}*16.f,vf2d{16.f,16.f});
				}
			}
		}
		Powerup::Initialize(mapPowerups);
	#pragma endregion

	mapImage.Decal()->Update();
	SetPixelMode(Pixel::NORMAL);
	SetDrawTarget(nullptr);
}

void HamsterGame::UpdateGame(const float fElapsedTime){
	camera.SetViewSize(tv.GetWorldVisibleArea());
	camera.Update(fElapsedTime);
	tv.SetWorldOffset(tv.ScaleToWorld(-SCREEN_FRAME.pos)+camera.GetViewPosition());
	Hamster::UpdateHamsters(fElapsedTime);
	Powerup::UpdatePowerups(fElapsedTime);
	border.Update(fElapsedTime);
}

void HamsterGame::DrawGame(){
	DrawLevelTiles();
	Powerup::DrawPowerups(tv);
	Hamster::DrawHamsters(tv);
	border.Draw();

	#pragma region Powerup Display
		for(int y:std::ranges::iota_view(0,4)){
			for(int x:std::ranges::iota_view(0,2)){
				const int powerupInd{y*2+x};
				const float drawX{x*32.f+12.f};
				const float drawY{y*32.f+12.f+96.f};
				const Powerup::PowerupType powerupType{Powerup::PowerupType(powerupInd)};
				const geom2d::rect<float>powerupSubimageRect{Powerup::GetPowerupSubimageRect(powerupType)};
				if(Hamster::GetPlayer().HasPowerup(powerupType)){
					SetDecalMode(DecalMode::ADDITIVE);
					DrawPartialRotatedDecal(vf2d{drawX,drawY}+16,GetGFX("gametiles.png").Decal(),0.f,{16.f,16.f},powerupSubimageRect.pos,powerupSubimageRect.size,{1.1f,1.1f});
					SetDecalMode(DecalMode::NORMAL);
					DrawPartialDecal({drawX,drawY},GetGFX("gametiles.png").Decal(),powerupSubimageRect.pos,powerupSubimageRect.size);
				}else{
					DrawPartialDecal({drawX,drawY},GetGFX("gametiles.png").Decal(),powerupSubimageRect.pos,powerupSubimageRect.size,{1.f,1.f},VERY_DARK_GREY);
				}
			}
		}
	#pragma endregion
	#pragma region Drown/Burn Bar.
		if(Hamster::GetPlayer().IsDrowning()){
			DrawDecal({12.f,240.f},GetGFX("drownmeter.png").Decal());
			GradientFillRectDecal(vf2d{12.f,240.f}+vf2d{12.f,5.f},vf2d{Hamster::GetPlayer().GetDrownRatio()*57.f,4.f},{145,199,255},{226,228,255},{226,228,255},{145,199,255});
		}
		else if(Hamster::GetPlayer().IsBurning()){
			DrawDecal({12.f,240.f},GetGFX("burnmeter.png").Decal());
			GradientFillRectDecal(vf2d{12.f,240.f}+vf2d{12.f,5.f},vf2d{Hamster::GetPlayer().GetBurnRatio()*57.f,4.f},{250,177,163},{226,228,255},{226,228,255},{250,177,163});
		}
	#pragma endregion

		tv.FillRectDecal(GetMousePos(),{2,2},GREEN);
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
	float extendedBounds{SCREEN_FRAME.pos.x};
	extendedBounds*=1/tv.GetWorldScale().x;
	tv.DrawDecal({},mapImage.Decal());
	for(const LayerTag&layer:currentMap.value().GetData().GetLayers()){
		for(float y=tv.GetWorldTL().y-17;y<=tv.GetWorldBR().y+16;y+=16){
			for(float x=tv.GetWorldTL().x-17+extendedBounds;x<=tv.GetWorldBR().x+16+extendedBounds;x+=16){
				if(x<=0.f||y<=0.f||x>=currentMap.value().GetData().GetMapData().width*16||y>=currentMap.value().GetData().GetMapData().height*16)continue;
				int tileX{int(floor(x)/16)};
				int tileY{int(floor(y)/16)};
				int tileID{layer.tiles[tileY][tileX]-1};
				if(ANIMATED_TILE_IDS.count(tileID)){
					Animate2D::FrameSequence&animatedTile{ANIMATED_TILE_IDS[tileID]};
					const Animate2D::Frame&currentFrame{animatedTile.GetFrame(runTime)};
					tv.DrawPartialDecal(vf2d{float(tileX),float(tileY)}*16,currentFrame.GetSourceImage()->Decal(),currentFrame.GetSourceRect().pos,currentFrame.GetSourceRect().size);
				}else continue;
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


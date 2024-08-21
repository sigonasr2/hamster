#include "HamsterGame.h"
#include "Hamster.h"
#include <stdexcept>
#include <ranges>
#include "util.h"
#include "Checkpoint.h"
#include "FloatingText.h"

geom2d::rect<float>HamsterGame::SCREEN_FRAME{{96,0},{320,288}};
std::unordered_map<std::string,Animate2D::Animation<AnimationState::AnimationState>>HamsterGame::ANIMATIONS;
std::unordered_map<std::string,Renderable>HamsterGame::GFX;
const std::string HamsterGame::ASSETS_DIR{"assets/"};
HamsterGame*HamsterGame::self{nullptr};
std::unordered_map<uint32_t,Animate2D::FrameSequence>HamsterGame::ANIMATED_TILE_IDS;

HamsterGame::HamsterGame(const std::string&appName){
	sAppName=appName;
	HamsterGame::self=this;
}

bool HamsterGame::OnUserCreate(){
	olc::GFX3D::ConfigureDisplay();
	camera=Camera2D{SCREEN_FRAME.size};
	camera.SetMode(Camera2D::Mode::LazyFollow);
	tv.Initialise(SCREEN_FRAME.size,{1,1});
	LoadGraphics();
	LoadAnimations();
	currentTileset=TSXParser{ASSETS_DIR+std::string("Terrain.tsx")};
	LoadLevel("TestLevel.tmx"); //THIS IS TEMPORARY.

	border.ChangeBorder(Border::DEFAULT);

	renderer.SetProjection(90.0f, (float)SCREEN_FRAME.size.x/(float)SCREEN_FRAME.size.y, 0.1f, 1000.0f, 0, SCREEN_FRAME.pos.y, 512, SCREEN_FRAME.size.y);
	std::vector<vf2d>radarCircle;
	for(int i=360;i>=0;i-=20){
		float angle=util::degToRad(float(i))-geom2d::pi/2;
		if(i==360){radarCircle.push_back(vf2d{cos(angle),sin(angle)}*43+vf2d{43,44});}
		radarCircle.push_back(vf2d{cos(angle),sin(angle)}*43+vf2d{43,44});
	}
	radar=ViewPort{radarCircle,{5.f,8.f}};

	for(int i:std::ranges::iota_view(0,8)){
		waterTiles.emplace_back();
		Renderable&waterTile{waterTiles.back()};
		waterTile.Create(16,16,false,false);
		SetDrawTarget(waterTile.Sprite());
		DrawPartialSprite({},GetGFX("gametiles.png").Sprite(),{192+i*16,784},{16,16});
		SetDrawTarget(nullptr);
		waterTile.Decal()->Update();
	}
	return true;
}

void HamsterGame::_LoadImage(const std::string&img){
	GFX.insert({img,Renderable{}});
	rcode result{GFX[img].Load(ASSETS_DIR+img,nullptr,false,false)};
	if(result!=OK)throw std::runtime_error{std::format("Failed to Load Image {}. OLC Rcode: {}",img,int(result))};
}

void HamsterGame::LoadGraphics(){
	_LoadImage("border.png");
	_LoadImage("gametiles.png");
	_LoadImage("shadow.png");
	_LoadImage("drownmeter.png");
	_LoadImage("burnmeter.png");
	_LoadImage("dot.png");
	_LoadImage("clouds.png");
	_LoadImage("aimingTarget.png");
	_LoadImage("fallometer.png");
	_LoadImage("fallometer_outline.png");
	_LoadImage("fuelmeter.png");
	_LoadImage("fuelbar.png");
	_LoadImage("fuelbar_outline.png");
	_LoadImage("speedometer.png");
	_LoadImage("speedometer_overlay.png");
	_LoadImage("radar.png");
	_LoadImage("checkpoint_arrow.png");
	_LoadImage("radaricons.png");
}

void HamsterGame::LoadAnimations(){
	auto LoadImageIfRequired=[this](const std::string&img){if(!GFX.count(std::string(img)))_LoadImage(img);};
	auto LoadStillAnimation=[this,&LoadImageIfRequired](const AnimationState::AnimationState state,const std::string&img){
		Animate2D::FrameSequence stillAnimation{0.f,Animate2D::Style::OneShot};
		LoadImageIfRequired(img);
		stillAnimation.AddFrame(Animate2D::Frame{&GetGFX(img),{{},GetGFX(img).Sprite()->Size()}});
		ANIMATIONS[std::string(img)].AddState(state,stillAnimation);
	};
	auto LoadAnimation=[this,&LoadImageIfRequired](const AnimationState::AnimationState state,const std::string&img,const std::vector<vf2d>frames,const float frameDuration=0.1f,const Animate2D::Style style=Animate2D::Style::Repeat,vf2d frameSize={32,32}){
		Animate2D::FrameSequence newAnimation{frameDuration,style};
		LoadImageIfRequired(img);
		for(const vf2d&framePos:frames){
			newAnimation.AddFrame(Animate2D::Frame{&GetGFX(img),{framePos,frameSize}});
		}
		ANIMATIONS[std::string(img)].AddState(state,newAnimation);
	};

	LoadAnimation(AnimationState::DEFAULT,"hamster.png",{{0,32},{32,32}},0.3f);
	LoadAnimation(AnimationState::WHEEL_TOP,"hamster.png",{{0,96},{32,96}},0.1f);
	LoadAnimation(AnimationState::WHEEL_BOTTOM,"hamster.png",{{64,96},{96,96}},0.1f);
	LoadAnimation(AnimationState::KNOCKOUT,"hamster.png",{{64,32},{96,32}},0.2f);
	LoadAnimation(AnimationState::SIDE_VIEW,"hamster.png",{{0,0},{32,0}},0.3f);
	Animate2D::FrameSequence&waterAnimFrames{(*ANIMATED_TILE_IDS.insert({1384,Animate2D::FrameSequence{0.2f}}).first).second};
	for(vf2d&sourcePos:std::vector<vf2d>{{192+16*0,784},{192+16*1,784},{192+16*2,784},{192+16*3,784},{192+16*4,784},{192+16*5,784},{192+16*6,784},{192+16*7,784}}){
		waterAnimFrames.AddFrame(Animate2D::Frame{&GetGFX("gametiles.png"),{sourcePos,{16,16}}});
	}
	Animate2D::FrameSequence&lavaAnimFrames{(*ANIMATED_TILE_IDS.insert({1412,Animate2D::FrameSequence{0.2f}}).first).second};
	for(vf2d&sourcePos:std::vector<vf2d>{{192+16*0,800},{192+16*1,800},{192+16*2,800},{192+16*3,800},{192+16*4,800},{192+16*5,800},{192+16*6,800},{192+16*7,800},{192+16*8,800}}){
		lavaAnimFrames.AddFrame(Animate2D::Frame{&GetGFX("gametiles.png"),{sourcePos,{16,16}}});
	}
	LoadAnimation(AnimationState::JET_LIGHTS,"hamster_jet.png",{{0,48},{48,48}},0.3f,Animate2D::Style::Repeat,{48,48});
	LoadAnimation(AnimationState::JET,"hamster_jet.png",{{},{0,96},{48,96},{96,96},{0,144},{48,144},{96,144},{0,192},{48,192},{96,192}},0.2f,Animate2D::Style::Repeat,{48,48});
	LoadAnimation(AnimationState::JET_FLAMES,"hamster_jet.png",{{48,0},{96,0}},0.15f,Animate2D::Style::Repeat,{48,48});
	LoadAnimation(AnimationState::DEFAULT,"checkpoint.png",{{}},0.f,Animate2D::Style::OneShot,{128,128});
	LoadAnimation(AnimationState::CHECKPOINT_CYCLING,"checkpoint.png",{{},{128,0}},0.4f,Animate2D::Style::Repeat,{128,128});
	LoadAnimation(AnimationState::CHECKPOINT_COLLECTED,"checkpoint.png",{{128,0}},0.f,Animate2D::Style::OneShot,{128,128});
}

void HamsterGame::LoadLevel(const std::string&mapName){
	const vf2d levelSpawnLoc{50,50}; //TEMPORARY

	currentMap=TMXParser{ASSETS_DIR+mapName};
	cloudSpd.x=util::random_range(-12.f,12.f);
	cloudSpd.y=util::random_range(-0.3f,0.3f);
	cloudOffset.x=util::random();
	cloudOffset.y=util::random();

	Hamster::LoadHamsters(levelSpawnLoc);
	camera.SetTarget(Hamster::GetPlayer().GetPos());
	
	
	mapImage.Create(currentMap.value().GetData().GetMapData().width*16,currentMap.value().GetData().GetMapData().height*16);
	SetDrawTarget(mapImage.Sprite());
	Clear(BLANK);
	SetPixelMode(Pixel::MASK);

	#pragma region Detect special tiles
	{
		std::vector<Powerup>mapPowerups;
		std::vector<vf2d>checkpoints;
		for(const LayerTag&layer:currentMap.value().GetData().GetLayers()){
			for(size_t y:std::ranges::iota_view(0U,layer.tiles.size())){
				for(size_t x:std::ranges::iota_view(0U,layer.tiles[y].size())){
					const int tileID{layer.tiles[y][x]-1};
					if(Powerup::TileIDIsUpperLeftPowerupTile(tileID))mapPowerups.emplace_back(vf2d{float(x),float(y)}*16+vf2d{16,16},Powerup::TileIDPowerupType(tileID));
					
					if(tileID==1484)checkpoints.emplace_back(vf2d{float(x),float(y)}*16+vf2d{64,64});
					
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
		Checkpoint::Initialize(checkpoints);
	}
	#pragma endregion

	mapImage.Decal()->Update();
	SetPixelMode(Pixel::NORMAL);
	SetDrawTarget(nullptr);
}

void HamsterGame::UpdateGame(const float fElapsedTime){
	vEye.z+=(Hamster::GetPlayer().GetZ()+zoom-vEye.z)*fLazyFollowRate*fElapsedTime;
	speedometerDisplayAmt+=(Hamster::GetPlayer().GetSpeed()-speedometerDisplayAmt)*fLazyFollowRate*fElapsedTime;

	if(GetMouseWheel()>0){
		radarScale=std::clamp(radarScale/2.f,6.f,96.f);
	}else if(GetMouseWheel()<0){
		radarScale=std::clamp(radarScale*2.f,6.f,96.f);
	}
	cloudOffset+=cloudSpd*fElapsedTime;
	camera.SetViewSize(tv.GetWorldVisibleArea());
	camera.Update(fElapsedTime);
	camera.SetLazyFollowRate(4.f*Hamster::GetPlayer().GetMaxSpeed()/128.f);
	tv.SetWorldOffset(tv.ScaleToWorld(-SCREEN_FRAME.pos)+camera.GetViewPosition());
	Hamster::UpdateHamsters(fElapsedTime);
	Powerup::UpdatePowerups(fElapsedTime);
	Checkpoint::UpdateCheckpoints(fElapsedTime);
	FloatingText::UpdateFloatingText(fElapsedTime);
	border.Update(fElapsedTime);
}

void HamsterGame::DrawGame(){
	SetZ(-0.01f);
	const size_t waterTileInd{size_t(GetRuntime()/0.2f)};
	tv.DrawPartialDecal({-3200,-3200},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400},waterTiles[waterTileInd%waterTiles.size()].Decal(),{0,0},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400});
	SetZ(-0.0005f);
	DrawLevelTiles();
	Checkpoint::DrawCheckpoints(tv);
	SetZ(0.01f);
	Powerup::DrawPowerups(tv);
	Hamster::DrawHamsters(tv);
	SetZ(3.f);
	tv.DrawPartialDecal({-3200,-3200},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400},GetGFX("dot.png").Decal(),{0,0},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400},{226,228,255,32});
	SetZ(7.f);
	tv.DrawPartialDecal({-3200,-3200},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400},GetGFX("dot.png").Decal(),{0,0},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400},{178,242,255,64});
	SetZ(2.f);
	tv.DrawPartialDecal({-3200,-3200},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400},GetGFX("clouds.png").Decal(),cloudOffset,currentMap.value().GetData().GetMapData().MapSize*16/2.f,{255,255,255,64});
	SetZ(6.f);
	tv.DrawPartialDecal({-3200,-3200},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400},GetGFX("clouds.png").Decal(),cloudOffset*2,currentMap.value().GetData().GetMapData().MapSize*16/2.f,{255,255,255,72});
	SetZ(0.f);
	FloatingText::DrawFloatingText(tv);
	border.Draw();
	Hamster::DrawOverlay();
	#pragma region Powerup Display
		for(int y:std::ranges::iota_view(0,4)){
			for(int x:std::ranges::iota_view(0,2)){
				const int powerupInd{y*2+x};
				const float drawX{x*32.f+16.f};
				const float drawY{y*32.f+12.f+96.f};
				const Powerup::PowerupType powerupType{Powerup::PowerupType(powerupInd)};
				const geom2d::rect<float>powerupSubimageRect{Powerup::GetPowerupSubimageRect(powerupType)};
				const Powerup tempPowerup{{},powerupType};
				const std::string powerupName{tempPowerup.GetName()};
				const vf2d powerupTextSize{GetTextSize(powerupName)};
				if(Hamster::GetPlayer().HasPowerup(powerupType)){
					SetDecalMode(DecalMode::ADDITIVE);
					DrawPartialRotatedDecal(vf2d{drawX,drawY}+16,GetGFX("gametiles.png").Decal(),0.f,{16.f,16.f},powerupSubimageRect.pos,powerupSubimageRect.size,{1.1f,1.1f});
					SetDecalMode(DecalMode::NORMAL);
					DrawPartialDecal({drawX,drawY},GetGFX("gametiles.png").Decal(),powerupSubimageRect.pos,powerupSubimageRect.size);
					SetDecalMode(DecalMode::ADDITIVE);
					for(int y:std::ranges::iota_view(-1,2)){
						for(int x:std::ranges::iota_view(-1,2)){
							DrawRotatedStringDecal(vf2d{drawX+16.f,drawY+32.f}+vi2d{x,y},powerupName,0.f,powerupTextSize/2,CYAN,{0.5f,1.f});
						}
					}
					SetDecalMode(DecalMode::NORMAL);
					DrawRotatedStringDecal({drawX+16.f,drawY+32.f},powerupName,0.f,powerupTextSize/2,VERY_DARK_BLUE,{0.5f,1.f});
				}else{
					DrawPartialDecal({drawX,drawY},GetGFX("gametiles.png").Decal(),powerupSubimageRect.pos,powerupSubimageRect.size,{1.f,1.f},VERY_DARK_GREY);
					DrawRotatedStringDecal({drawX+16.f,drawY+32.f},powerupName,0.f,powerupTextSize/2,DARK_GREY,{0.5f,1.f});
				}
			}
		}
	#pragma endregion
	#pragma region Drown/Burn Bar.
		{
			if(Hamster::GetPlayer().IsDrowning()){
				DrawDecal({12.f,240.f},GetGFX("drownmeter.png").Decal());
				GradientFillRectDecal(vf2d{12.f,240.f}+vf2d{12.f,5.f},vf2d{Hamster::GetPlayer().GetDrownRatio()*57.f,4.f},{145,199,255},{226,228,255},{226,228,255},{145,199,255});
			}
			else if(Hamster::GetPlayer().IsBurning()){
				DrawDecal({12.f,240.f},GetGFX("burnmeter.png").Decal());
				GradientFillRectDecal(vf2d{12.f,240.f}+vf2d{12.f,5.f},vf2d{Hamster::GetPlayer().GetBurnRatio()*57.f,4.f},{250,177,163},{226,228,255},{226,228,255},{250,177,163});
			}
		}
	#pragma endregion
	float speedometerWidth{float(GetGFX("speedometer.png").Sprite()->width)};
	const float speedometerSpd{std::min(speedometerWidth,speedometerDisplayAmt/180.f*speedometerWidth)};
	DrawPartialRotatedDecal(SCREEN_FRAME.pos+SCREEN_FRAME.size+vf2d{48.f,-4.f-GetGFX("speedometer_overlay.png").Sprite()->height},GetGFX("speedometer_overlay.png").Decal(),0.f,GetGFX("speedometer_overlay.png").Sprite()->Size()/2,{},vf2d{speedometerSpd,float(GetGFX("speedometer_overlay.png").Sprite()->height)});
	DrawRotatedDecal(SCREEN_FRAME.pos+SCREEN_FRAME.size+vf2d{48.f,-4.f-GetGFX("speedometer.png").Sprite()->height},GetGFX("speedometer.png").Decal(),0.f,GetGFX("speedometer.png").Sprite()->Size()/2);
	const std::string speedometerStr{std::format("{:.0f}km/h",speedometerDisplayAmt)};
	const vf2d speedometerStrSize{GetTextSize(speedometerStr)};
	Pixel speedometerCol{CYAN};
	if(speedometerDisplayAmt>=180)speedometerCol=RED;
	else if(speedometerDisplayAmt>=120)speedometerCol=YELLOW;
	else if(speedometerDisplayAmt>=80)speedometerCol=GREEN;
	for(int y:std::ranges::iota_view(-1,2)){
		for(int x:std::ranges::iota_view(-1,2)){
			if(x==0&&y==0)continue;
			DrawStringDecal(SCREEN_FRAME.pos+SCREEN_FRAME.size-speedometerStrSize-vf2d{4.f,4.f}+vi2d{x,y},speedometerStr,BLACK);
		}
	}
	DrawStringDecal(SCREEN_FRAME.pos+SCREEN_FRAME.size-speedometerStrSize-vf2d{4.f,4.f},speedometerStr,speedometerCol);
	DrawDecal({2.f,4.f},GetGFX("radar.png").Decal());
	DrawRadar();
	DrawStringDecal({0,8.f},std::to_string(GetFPS()));
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

const bool HamsterGame::IsTerrainSolid(const vf2d pos)const{
	if(!IsInBounds(pos))return true;
	for(const LayerTag&layer:currentMap.value().GetData().GetLayers()){
		int tileX{int(floor(pos.x)/16)};
		int tileY{int(floor(pos.y)/16)};
		if(tileX<=0||tileX>=currentMap.value().GetData().GetMapData().MapSize.x||tileY<=0||tileY>=currentMap.value().GetData().GetMapData().MapSize.y)break;
		int tileID{layer.tiles[tileY][tileX]-1};
		if(tileID==-1)continue;
		if(currentTileset.value().GetData().GetTerrainData().count(tileID)&&currentTileset.value().GetData().GetTerrainData().at(tileID).first==Terrain::SolidType::SOLID)return true;
	}
	return false;
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

const Renderable&HamsterGame::GetGFX(const std::string&img){
	return GFX[img];
}
const Animate2D::Animation<AnimationState::AnimationState>&HamsterGame::GetAnimations(const std::string&img){
	return ANIMATIONS[img];
}

bool HamsterGame::OnUserDestroy(){
	Hamster::OnUserDestroy();
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

void HamsterGame::Apply3DTransform(std::vector<DecalInstance>&decals){
	std::vector<DecalInstance>oldDecals;
	std::vector<DecalInstance>foregroundDecals;
	oldDecals.reserve(decals.size());
	std::copy(decals.begin(),decals.end(),std::back_inserter(oldDecals));
	decals.clear();
	GFX3D::vec3d vLookTarget = GFX3D::Math::Vec_Add(vEye, vLookDir);
	
	GFX3D::ClearDepth();

	renderer.SetCamera(vEye, vLookTarget, vUp);
	
	GFX3D::mat4x4 matRotateX=GFX3D::Math::Mat_MakeRotationX(0.f);
	GFX3D::mat4x4 matRotateZ=GFX3D::Math::Mat_MakeRotationZ(0.f);

	GFX3D::mat4x4 matWorld=GFX3D::Math::Mat_MultiplyMatrix(matRotateX,matRotateZ);
	
	renderer.SetTransform(matWorld);

	float zIncrementer{0.f};

	for(DecalInstance&decal:oldDecals){
		SetDecalMode(decal.mode);
		if(decal.transform==GFX3DTransform::NO_TRANSFORM){
			foregroundDecals.emplace_back(decal);
		}
		else
		if(decal.points==3){
			GFX3D::triangle tri{{{decal.pos[0].x,decal.pos[0].y,decal.z[0],1.f},{decal.pos[1].x,decal.pos[1].y,decal.z[1],1.f},{decal.pos[2].x,decal.pos[2].y,decal.z[2],1.f}},{{decal.uv[0].x,decal.uv[0].y,0.f},{decal.uv[1].x,decal.uv[1].y,0.f},{decal.uv[2].x,decal.uv[2].y,0.f}},{decal.tint[0],decal.tint[1],decal.tint[2]}};
			tri.p[0].z+=zIncrementer;
			tri.p[1].z+=zIncrementer;
			tri.p[2].z+=zIncrementer;
			renderer.Render({tri},decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			if(decal.z[0]>0.1f||decal.z[1]>0.1f||decal.z[2]>0.1f){
				tri.col[0]=tri.col[1]=tri.col[2]={0,0,0,uint8_t(util::lerp(0,160,(1/std::pow(decal.z[0]/10.f+1,4))))};
				tri.p[0].z=tri.p[1].z=tri.p[2].z=0.1f+zIncrementer;
				renderer.Render({tri},decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			}
		}else if(decal.points==4){
			GFX3D::triangle tri{{{decal.pos[0].x,decal.pos[0].y,decal.z[0],1.f},{decal.pos[1].x,decal.pos[1].y,decal.z[1],1.f},{decal.pos[2].x,decal.pos[2].y,decal.z[2],1.f}},{{decal.uv[0].x,decal.uv[0].y,0.f},{decal.uv[1].x,decal.uv[1].y,0.f},{decal.uv[2].x,decal.uv[2].y,0.f}},{decal.tint[0],decal.tint[1],decal.tint[2]}};
			GFX3D::triangle tri2{{{decal.pos[0].x,decal.pos[0].y,decal.z[0],1.f},{decal.pos[2].x,decal.pos[2].y,decal.z[2],1.f},{decal.pos[3].x,decal.pos[3].y,decal.z[3],1.f}},{{decal.uv[0].x,decal.uv[0].y,0.f},{decal.uv[2].x,decal.uv[2].y,0.f},{decal.uv[3].x,decal.uv[3].y,0.f}},{decal.tint[0],decal.tint[2],decal.tint[3]}};
			tri.p[0].z+=zIncrementer;
			tri.p[1].z+=zIncrementer;
			tri.p[2].z+=zIncrementer;
			tri2.p[0].z+=zIncrementer;
			tri2.p[1].z+=zIncrementer;
			tri2.p[2].z+=zIncrementer;
			renderer.Render({tri,tri2},decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			if(decal.decal!=GetGFX("dot.png").Decal()&&(decal.z[0]>0.1f||decal.z[1]>0.1f||decal.z[2]>0.1f||decal.z[3]>0.1f)){
				tri.col[0]=tri.col[1]=tri.col[2]=tri2.col[0]=tri2.col[1]=tri2.col[2]={0,0,0,uint8_t(util::lerp(0,160,(1/std::pow(decal.z[0]/10.f+1,4))))};
				tri.p[0].z=tri.p[1].z=tri.p[2].z=tri2.p[0].z=tri2.p[1].z=tri2.p[2].z=0.1f+zIncrementer;
				renderer.Render({tri,tri2},decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			}
		}else{
			std::vector<GFX3D::triangle>tris;
			std::vector<GFX3D::triangle>shadowTris;
			tris.reserve(decal.points/3);
			if(decal.structure!=DecalStructure::LIST)throw std::runtime_error{std::format("WARNING! Using triangle structure type {} is unsupported! Please only use DecalStructure::LIST!!",int(decal.structure))};
			if(decal.points%3!=0)throw std::runtime_error{std::format("WARNING! Number of decal structure points is not a multiple of 3! Points provided: {}. THIS SHOULD NOT BE HAPPENING!",decal.points)};
			for(int i{0};i<decal.points;i+=3){
				GFX3D::triangle tri{{{decal.pos[i+0].x,decal.pos[i+0].y,decal.z[i+0],1.f},{decal.pos[i+1].x,decal.pos[i+1].y,decal.z[i+1],1.f},{decal.pos[i+2].x,decal.pos[i+2].y,decal.z[i+2],1.f}},{{decal.uv[i+0].x,decal.uv[i+0].y,0.f},{decal.uv[i+1].x,decal.uv[i+1].y,0.f},{decal.uv[i+2].x,decal.uv[i+2].y,0.f}},{decal.tint[i+0],decal.tint[i+1],decal.tint[i+2]}};
				tri.p[0].z+=zIncrementer;
				tri.p[1].z+=zIncrementer;
				tri.p[2].z+=zIncrementer;
				tris.emplace_back(tri);
				if(decal.z[i+0]>0.1f||decal.z[i+1]>0.1f||decal.z[i+2]>0.1f){
					tri.col[0]=tri.col[1]=tri.col[2]={0,0,0,uint8_t(util::lerp(0,160,(1/std::pow(decal.z[0]/10.f+1,4))))};
					tri.p[0].z=tri.p[1].z=tri.p[2].z=0.1f+zIncrementer;
					shadowTris.emplace_back(tri);
				}
			}
			renderer.Render(tris,decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			if(shadowTris.size()>0){
				renderer.Render(shadowTris,decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			}
		}
		SetDecalMode(DecalMode::NORMAL);
		zIncrementer+=0.000001f;
	}

	std::sort(decals.begin(),decals.end(),[](const DecalInstance&d1,const DecalInstance&d2){return d1.z[0]>d2.z[0];});
	std::copy(foregroundDecals.begin(),foregroundDecals.end(),std::back_inserter(decals));
}

const Animate2D::FrameSequence&HamsterGame::GetAnimation(const std::string&img,const AnimationState::AnimationState state){
	return GetAnimations(img).GetFrames(state);
}

void HamsterGame::SetZoom(const float zoom){
	this->zoom=zoom;
}
const float HamsterGame::GetZoom()const{
	return zoom;
}

const bool HamsterGame::IsInBounds(const vf2d pos)const{
	return !(pos.x<=-160.f||pos.y<=-160.f||pos.x>=currentMap.value().GetData().GetMapData().width*16+160.f||pos.y>=currentMap.value().GetData().GetMapData().height*16+160.f);
}

const float HamsterGame::GetCameraZ()const{
	return vEye.z;
}

void HamsterGame::DrawRadar(){
	const vf2d radarOffset{43.f,44.f};
	const auto WorldToRadar=[this,&radarOffset](const vf2d&pos){
		vf2d relativeWorldPos{pos-Hamster::GetPlayer().GetPos()};
		return relativeWorldPos/radarScale+radarOffset+vf2d{5.f,8.f}/radarScale;
	};

	enum IconType{
		CHECKPOINT,
		WHEEL,
		GRASS,
		SAND,
		SWAMP,
		LAVA,
		FOREST,
		ICE,
		JET,
		HAMSTER,
	};

	const std::unordered_map<IconType,geom2d::rect<float>>icon{
		{CHECKPOINT,{{16.f*0,0.f},{16.f,16.f}}},
		{WHEEL,{{16.f*1,0.f},{16.f,16.f}}},
		{GRASS,{{16.f*2,0.f},{16.f,16.f}}},
		{SAND,{{16.f*3,0.f},{16.f,16.f}}},
		{SWAMP,{{16.f*4,0.f},{16.f,16.f}}},
		{LAVA,{{16.f*5,0.f},{16.f,16.f}}},
		{FOREST,{{16.f*6,0.f},{16.f,16.f}}},
		{ICE,{{16.f*7,0.f},{16.f,16.f}}},
		{JET,{{16.f*8,0.f},{16.f,16.f}}},
		{HAMSTER,{{16.f*9,0.f},{16.f,16.f}}},
	};

	const auto DeferRenderingBasedOnPosition=[this,&icon](const vf2d&pos,const IconType powerupIcon,const uint8_t iconAlpha){
		if(geom2d::intersects(geom2d::circle<float>{{43.f+5.f,44.f+8.f},43},geom2d::rect<float>{pos-vf2d{16.f,16.f},{32.f,32.f}}).size()>0)radar.DrawPartialRotatedDecal(pos,GetGFX("radaricons.png").Decal(),0.f,{8.f,8.f},icon.at(powerupIcon).pos,icon.at(powerupIcon).size,{1.f,1.f},{255,255,255,iconAlpha});
		else if(geom2d::contains(geom2d::circle<float>{{43.f+5.f,44.f+8.f},43},geom2d::rect<float>{pos-vf2d{8.f,8.f},{16.f,16.f}}))DrawPartialRotatedDecal(pos+vf2d{5.f,8.f},GetGFX("radaricons.png").Decal(),0.f,{8.f,8.f},icon.at(powerupIcon).pos,icon.at(powerupIcon).size,{1.f,1.f},{255,255,255,iconAlpha});
	};

	for(const Powerup&powerup:Powerup::GetPowerups()){
		IconType powerupIcon{IconType(int(powerup.GetType())+1)};
		uint8_t iconAlpha{255U};
		if(Hamster::GetPlayer().HasPowerup(powerup.GetType()))iconAlpha=64U;
		DeferRenderingBasedOnPosition(WorldToRadar(powerup.GetPos()),powerupIcon,iconAlpha);
	}
	for(const Checkpoint&cp:Checkpoint::GetCheckpoints()){
		uint8_t iconAlpha{255U};
		if(Hamster::GetPlayer().HasCollectedCheckpoint(cp))iconAlpha=64U;
		DeferRenderingBasedOnPosition(WorldToRadar(cp.GetPos()),CHECKPOINT,iconAlpha);
	}
	for(const Hamster&h:Hamster::GetHamsters()){
		if(&h==&Hamster::GetPlayer())continue;
		uint8_t iconAlpha{255U};
		if(h.BurnedOrDrowned())iconAlpha=64U;
		DeferRenderingBasedOnPosition(WorldToRadar(h.GetPos()),HAMSTER,iconAlpha);
	}
}

int main()
{
	HamsterGame game("Project Hamster");
	if(game.Construct(512, 288, 3, 3))
		game.Start();

	return 0;
}
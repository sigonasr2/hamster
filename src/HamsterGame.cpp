#include "HamsterGame.h"
#include "Hamster.h"
#include <stdexcept>
#include <ranges>
#include "util.h"

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
	return true;
}

void HamsterGame::_LoadImage(const std::string_view img){
	GFX.insert({ASSETS_DIR+std::string(img),Renderable{}});
	rcode result{GFX[ASSETS_DIR+std::string(img)].Load(ASSETS_DIR+std::string(img),nullptr,false,false)};
	if(result!=OK)throw std::runtime_error{std::format("Failed to Load Image {}. OLC Rcode: {}",img,int(result))};
}

void HamsterGame::LoadGraphics(){
	_LoadImage("border.png");
	_LoadImage("gametiles.png");
	_LoadImage("shadow.png");
	_LoadImage("drownmeter.png");
	_LoadImage("burnmeter.png");
	_LoadImage("hamster_jet.png");
	_LoadImage("dot.png");
	_LoadImage("clouds.png");
	_LoadImage("aimingTarget.png");
	_LoadImage("fallometer.png");
	_LoadImage("fallometer_outline.png");
	_LoadImage("fuelmeter.png");
	_LoadImage("fuelbar.png");
	_LoadImage("fuelbar_outline.png");
	UpdateMatrixTexture();
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
	LoadAnimation(KNOCKOUT,"hamster.png",{{64,32},{96,32}},0.2f);
	Animate2D::FrameSequence&waterAnimFrames{(*ANIMATED_TILE_IDS.insert({1384,Animate2D::FrameSequence{0.2f}}).first).second};
	for(vf2d&sourcePos:std::vector<vf2d>{{192+16*0,784},{192+16*1,784},{192+16*2,784},{192+16*3,784},{192+16*4,784},{192+16*5,784},{192+16*6,784},{192+16*7,784}}){
		waterAnimFrames.AddFrame(Animate2D::Frame{&GetGFX("gametiles.png"),{sourcePos,{16,16}}});
	}
	Animate2D::FrameSequence&lavaAnimFrames{(*ANIMATED_TILE_IDS.insert({1412,Animate2D::FrameSequence{0.2f}}).first).second};
	for(vf2d&sourcePos:std::vector<vf2d>{{192+16*0,800},{192+16*1,800},{192+16*2,800},{192+16*3,800},{192+16*4,800},{192+16*5,800},{192+16*6,800},{192+16*7,800},{192+16*8,800}}){
		lavaAnimFrames.AddFrame(Animate2D::Frame{&GetGFX("gametiles.png"),{sourcePos,{16,16}}});
	}
	LoadAnimation(JET_LIGHTS,"hamster_jet.png",{{0,48},{48,48}},0.3f,Animate2D::Style::Repeat,{48,48});
	LoadAnimation(JET_FLAMES,"hamster_jet.png",{{48,0},{96,0}},0.15f,Animate2D::Style::Repeat,{48,48});

	animatedWaterTile.Create(16,16,false,false);
	UpdateWaterTexture();
}

void HamsterGame::LoadLevel(const std::string_view mapName){
	const vf2d levelSpawnLoc{50,50}; //TEMPORARY

	currentMap=TMXParser{ASSETS_DIR+std::string(mapName)};
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
	UpdateMatrixTexture();
	UpdateWaterTexture();
	cloudOffset+=cloudSpd*fElapsedTime;
	camera.SetViewSize(tv.GetWorldVisibleArea());
	camera.Update(fElapsedTime);
	camera.SetLazyFollowRate(4.f*Hamster::GetPlayer().GetMaxSpeed()/128.f);
	tv.SetWorldOffset(tv.ScaleToWorld(-SCREEN_FRAME.pos)+camera.GetViewPosition());
	Hamster::UpdateHamsters(fElapsedTime);
	Powerup::UpdatePowerups(fElapsedTime);
	border.Update(fElapsedTime);
}

void HamsterGame::DrawGame(){
	tv.DrawPartialDecal({-3200,-3200},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400},animatedWaterTile.Decal(),{0,0},currentMap.value().GetData().GetMapData().MapSize*16+vf2d{6400,6400});
	DrawLevelTiles();
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
	border.Draw();
	Hamster::DrawOverlay();
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
	bool tileIsBlank{true};
	for(const LayerTag&layer:currentMap.value().GetData().GetLayers()){
		int tileX{int(floor(pos.x)/16)};
		int tileY{int(floor(pos.y)/16)};
		if(tileX<=0||tileX>=currentMap.value().GetData().GetMapData().MapSize.x||tileY<=0||tileY>=currentMap.value().GetData().GetMapData().MapSize.y)break;
		int tileID{layer.tiles[tileY][tileX]-1};
		if(tileID==-1)continue;
		tileIsBlank=false;
		if(currentTileset.value().GetData().GetTerrainData().count(tileID)&&currentTileset.value().GetData().GetTerrainData().at(tileID).first==Terrain::SolidType::SOLID)return true;
	}
	return tileIsBlank;
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

	vEye.z+=(Hamster::GetPlayer().GetZ()+zoom-vEye.z)*fLazyFollowRate*fElapsedTime;

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

void HamsterGame::UpdateMatrixTexture(){
	const auto result{GFX.insert({ASSETS_DIR+"MATRIX_TEXTURE",Renderable{}})};
	Renderable&texture{(*result.first).second};
	if(result.second){
		texture.Create(64,64,false,false);
		texture.Sprite()->SetSampleMode(Sprite::PERIODIC);
	}

	const std::array<char,10>matrixLetters{'0','1','2','3','4','5','6','7','8','9'};
	
	if(matrixTimer==0){
		activeLetters.emplace_back(vf2d{float(rand()%64),float(64)},util::random(-40)-20,matrixLetters[rand()%matrixLetters.size()]);
		matrixTimer=util::random(0.125);
	}
	if(updatePixelsTimer==0){
		SetDrawTarget(texture.Sprite());
		Sprite*img=texture.Sprite();
		for(int y=63;y>=0;y--){
			for(int x=63;x>=0;x--){
				Pixel col=img->GetPixel(x,y);
				if(col.r>0){
					if(x>0){
						Pixel leftCol=img->GetPixel(x-1,y);
						if(leftCol.r<col.r){
							leftCol=PixelLerp(col,leftCol,0.125);
						}
						Draw(x-1,y,leftCol);
					}
					if(x<img->width-1){
						Pixel rightCol=img->GetPixel(x+1,y);
						if(rightCol.r<col.r){
							rightCol=PixelLerp(col,rightCol,0.125);
						}
						Draw(x+1,y,rightCol);
					}
					col/=8;
					Draw(x,y,col);
				}
			}
		}
		for(int y=0;y<64;y++){
			Draw({0,y},img->GetPixel(1,y));
		}
		SetDrawTarget(nullptr);
		updatePixelsTimer=0.1;
	}
	if(activeLetters.size()>0){
		SetDrawTarget(texture.Sprite());
		for(Letter&letter:activeLetters){
			letter.pos.y+=letter.spd*GetElapsedTime();
			DrawString(letter.pos,std::string(1,letter.c));
		}
		SetDrawTarget(nullptr);
		texture.Decal()->Update();
	}
	matrixTimer=std::max(0.f,matrixTimer-GetElapsedTime());
	updatePixelsTimer=std::max(0.f,updatePixelsTimer-GetElapsedTime());
	std::erase_if(activeLetters,[](Letter&letter){return letter.pos.y<-32;});
}

void HamsterGame::UpdateWaterTexture(){
	const Animate2D::FrameSequence&waterAnimSequence{ANIMATED_TILE_IDS[1384]};
	const Animate2D::Frame&frame{waterAnimSequence.GetFrame(GetRuntime())};
	SetDrawTarget(animatedWaterTile.Sprite());
	DrawPartialSprite({},frame.GetSourceImage()->Sprite(),frame.GetSourceRect().pos,frame.GetSourceRect().size);
	SetDrawTarget(nullptr);
	animatedWaterTile.Decal()->Update();
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

	for(DecalInstance&decal:oldDecals){
		if(decal.transform==GFX3DTransform::NO_TRANSFORM)foregroundDecals.emplace_back(decal);
		else
		if(decal.points==3){
			GFX3D::triangle tri{{{decal.pos[0].x,decal.pos[0].y,decal.z[0],1.f},{decal.pos[1].x,decal.pos[1].y,decal.z[1],1.f},{decal.pos[2].x,decal.pos[2].y,decal.z[2],1.f}},{{decal.uv[0].x,decal.uv[0].y,0.f},{decal.uv[1].x,decal.uv[1].y,0.f},{decal.uv[2].x,decal.uv[2].y,0.f}},{decal.tint[0],decal.tint[1],decal.tint[2]}};
			renderer.Render({tri},decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			if(decal.z[0]>0.1f||decal.z[1]>0.1f||decal.z[2]>0.1f){
				tri.col[0]=tri.col[1]=tri.col[2]={0,0,0,uint8_t(util::lerp(0,160,(1/std::pow(decal.z[0]/10.f+1,4))))};
				tri.p[0].z=tri.p[1].z=tri.p[2].z=0.1f;
				renderer.Render({tri},decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			}
		}else if(decal.points==4){
			GFX3D::triangle tri{{{decal.pos[0].x,decal.pos[0].y,decal.z[0],1.f},{decal.pos[1].x,decal.pos[1].y,decal.z[1],1.f},{decal.pos[2].x,decal.pos[2].y,decal.z[2],1.f}},{{decal.uv[0].x,decal.uv[0].y,0.f},{decal.uv[1].x,decal.uv[1].y,0.f},{decal.uv[2].x,decal.uv[2].y,0.f}},{decal.tint[0],decal.tint[1],decal.tint[2]}};
			GFX3D::triangle tri2{{{decal.pos[0].x,decal.pos[0].y,decal.z[0],1.f},{decal.pos[2].x,decal.pos[2].y,decal.z[2],1.f},{decal.pos[3].x,decal.pos[3].y,decal.z[3],1.f}},{{decal.uv[0].x,decal.uv[0].y,0.f},{decal.uv[2].x,decal.uv[2].y,0.f},{decal.uv[3].x,decal.uv[3].y,0.f}},{decal.tint[0],decal.tint[2],decal.tint[3]}};
			renderer.Render({tri,tri2},decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			if(decal.decal!=GetGFX("dot.png").Decal()&&(decal.z[0]>0.1f||decal.z[1]>0.1f||decal.z[2]>0.1f||decal.z[3]>0.1f)){
				tri.col[0]=tri.col[1]=tri.col[2]=tri2.col[0]=tri2.col[1]=tri2.col[2]={0,0,0,uint8_t(util::lerp(0,160,(1/std::pow(decal.z[0]/10.f+1,4))))};
				tri.p[0].z=tri.p[1].z=tri.p[2].z=tri2.p[0].z=tri2.p[1].z=tri2.p[2].z=0.1f;
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
				tris.emplace_back(tri);
				if(decal.z[i+0]>0||decal.z[i+1]>0||decal.z[i+2]>0){
					tri.col[0]=tri.col[1]=tri.col[2]={0,0,0,uint8_t(util::lerp(0,160,(1/std::pow(decal.z[0]/10.f+1,4))))};
					tri.p[0].z=tri.p[1].z=tri.p[2].z=0.1f;
					shadowTris.emplace_back(tri);
				}
			}
			renderer.Render(tris,decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			if(shadowTris.size()>0){
				renderer.Render(shadowTris,decal.decal,GFX3D::RENDER_TEXTURED|GFX3D::RENDER_DEPTH);
			}
		}
	}

	std::sort(decals.begin(),decals.end(),[](const DecalInstance&d1,const DecalInstance&d2){return d1.z[0]>d2.z[0];});
	std::copy(foregroundDecals.begin(),foregroundDecals.end(),std::back_inserter(decals));
}

const Animate2D::FrameSequence&HamsterGame::GetAnimation(const std::string_view img,const AnimationState state){
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

int main()
{
	HamsterGame game;
	if(game.Construct(512, 288, 3, 3))
		game.Start();

	return 0;
}
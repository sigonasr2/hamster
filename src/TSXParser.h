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
#include <sstream>
#include "TMXParser.h"
#include "Terrain.h"
#include "Powerup.h"

using namespace olc;


struct TileCollisionData{
	geom2d::rect<float>collision;
};

struct Tileset{
	XMLTag ImageData;
    int tilewidth=0,tileheight=0;
    int imagewidth=0,imageheight=0;
    bool isTerrain=false;
    std::unordered_map<int,TileCollisionData>CollisionData;
    std::unordered_map<int,std::vector<int>>AnimationData;
    std::unordered_map<int,std::pair<Terrain::SolidType,Terrain::TerrainType>>TerrainData;
    friend std::ostream& operator << (std::ostream& os, Tileset& rhs);
public:
    const std::unordered_map<int,std::pair<Terrain::SolidType,Terrain::TerrainType>>&GetTerrainData()const;
};

class TSXParser{
	public:
	const Tileset&GetData()const;
	private:
	Tileset parsedTilesetInfo;
	void ParseTag(std::string tag);
    std::vector<std::string> previousTag;
    std::vector<int> previousTagID;
	public:
	TSXParser(std::string file);
};

//#define TSX_PARSER_SETUP

#ifdef TSX_PARSER_SETUP
#undef TSX_PARSER_SETUP
    extern bool _DEBUG_MAP_LOAD_INFO;
	const Tileset&TSXParser::GetData()const{
		return parsedTilesetInfo;
	}
    std::ostream&operator<<(std::ostream& os, Tileset& rhs){
        os<<rhs.ImageData.FormatTagData(rhs.ImageData.data)<<"\n";
        return os;
    }
    const std::unordered_map<int,std::pair<Terrain::SolidType,Terrain::TerrainType>>&Tileset::GetTerrainData()const{
        return TerrainData;
    }
    void TSXParser::ParseTag(std::string tag) {
        XMLTag newTag;
        //First character is a '<' so we discard it.
        tag.erase(0,1); tag.erase(tag.length()-1,1); //Erase the first and last characters in the tag. Now parse by spaces.
        std::stringstream s(tag); //Turn it into a string stream to now parse into individual whitespaces.
        std::string data;
        while (s.good()) {
            int quotationMarkCount=0;
            bool pastEquals=false;
            data="";
            bool valid=false;
            while(s.good()){
                int character=s.get();
                if(character=='"'){
                    quotationMarkCount++;
                }
                if(character==' '&&quotationMarkCount%2==0){
                    valid=true;
                    break;
                }
                data+=character;
                if(pastEquals&&quotationMarkCount%2==0){
                    valid=true;
                    break;
                }
                if(character=='='&&quotationMarkCount%2==0){
                    pastEquals=true;
                }
            }
            if(valid&&data.length()>0){
                if (newTag.tag.length()==0) { //Tag's empty, so first line is the tag.
                    newTag.tag=data;
                } else {
                    std::string key = data.substr(0,data.find("="));
                    std::string value = data.substr(data.find("=")+1,std::string::npos);

                    //Strip Quotation marks.
                    value = value.substr(1,std::string::npos);
                    value = value.substr(0,value.length()-1);

                    newTag.data[key]=value;
                }
            }
        }
        
        if (newTag.tag=="tileset") {
            parsedTilesetInfo.tilewidth=stoi(newTag.data["tilewidth"]);
            parsedTilesetInfo.tileheight=stoi(newTag.data["tileheight"]);
            if(newTag.data.find("class")!=newTag.data.end()){
                parsedTilesetInfo.isTerrain=newTag.GetString("class")=="Terrain";
            }
        } else
        if (newTag.tag=="image") {
            parsedTilesetInfo.ImageData=newTag;
            parsedTilesetInfo.imagewidth=newTag.GetInteger("width");
            parsedTilesetInfo.imageheight=newTag.GetInteger("height");
        } else
        if (newTag.tag=="tile"){
            previousTag.emplace_back(newTag.tag);
            previousTagID.clear();
            previousTagID.emplace_back(newTag.GetInteger("id"));
        } else
        if(newTag.tag=="frame"){
            //The way animation data is stored is every "animation_tile_precision" ms indicating which frame we should be on.
            for(int&tagID:previousTagID){
                for(int i=0;i<newTag.GetInteger("duration")/100;i++){
                    parsedTilesetInfo.AnimationData[tagID].push_back(newTag.GetInteger("tileid"));
                }
            }
        } else
        if(newTag.tag=="property"&&newTag.data["propertytype"]=="TerrainType"){
            //The way animation data is stored is every "animation_tile_precision" ms indicating which frame we should be on.
            for(int&tagID:previousTagID){
                std::pair<Terrain::SolidType,Terrain::TerrainType>&tileData{parsedTilesetInfo.TerrainData[tagID]};
                tileData.second=Terrain::TerrainType(newTag.GetInteger("value"));
            }
        } else
        if(newTag.tag=="property"&&newTag.data["name"]=="Solid"){
            //The way animation data is stored is every "animation_tile_precision" ms indicating which frame we should be on.
            for(int&tagID:previousTagID){
                std::pair<Terrain::SolidType,Terrain::TerrainType>&tileData{parsedTilesetInfo.TerrainData[tagID]};
                tileData.first=Terrain::SolidType(newTag.GetBool("value"));
            }
        } else
        if(newTag.tag=="property"&&newTag.data["propertytype"]=="PowerupType"){
            //The way animation data is stored is every "animation_tile_precision" ms indicating which frame we should be on.
            for(int&tagID:previousTagID){
                Powerup::AddOrUpdatePowerupIdList(tagID,Powerup::PowerupType(newTag.GetInteger("value")));
            }
        } else
        if(newTag.tag=="property"&&newTag.data["name"]=="Upper-Left"){
            //The way animation data is stored is every "animation_tile_precision" ms indicating which frame we should be on.
            for(int&tagID:previousTagID){
                Powerup::AddOrUpdatePowerupIdList(tagID,Powerup::TileType(newTag.GetBool("value")));
            }
        } else
        if (newTag.tag=="object"&&previousTag.size()>0&&previousTag[0]=="tile"){
            for(int&tagID:previousTagID){
                TileCollisionData data;
                data.collision=geom2d::rect<float>{{newTag.GetFloat("x"),newTag.GetFloat("y")},{newTag.GetFloat("width"),newTag.GetFloat("height")}};
                if(!parsedTilesetInfo.CollisionData.count(tagID)){
                    parsedTilesetInfo.CollisionData[tagID]=data;
                }
            }
        }
    }
    TSXParser::TSXParser(std::string file){
        std::ifstream f(file,std::ios::in);

        std::string accumulator="";

        while (f.good()) {
            std::string data;
            f>>data;
            if (data.empty()) continue;

            if (accumulator.length()>0) {
                accumulator+=" "+data;
                //Check if it ends with '>'
                if (data[data.length()-1]=='>') {
                    ParseTag(accumulator);
                    accumulator="";
                }
            } else
                if (data[0]=='<') {
                    //Beginning of XML tag.
                    accumulator=data;
                    if(accumulator.length()>1&&accumulator.at(1)=='/'){
                        if(accumulator.starts_with("</tile>")){
                            previousTag.clear();
                            previousTagID.clear();
                        }
                        accumulator=""; //Restart because this is an end tag.
                    }
                    if(accumulator.length()>1&&accumulator.find('>')!=std::string::npos){
                        accumulator=""; //Restart because this tag has nothing in it!
                    }
                }
        }
    }
#endif
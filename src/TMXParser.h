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
#include "olcUTIL_Geometry2D.h"
#include "olcPixelGameEngine.h"
#include <sstream>
#include <set>
#include <queue>

using MapName=std::string;
using namespace olc;
using namespace std::literals;

struct XMLTag{
    std::string tag;
    std::map<std::string,std::string> data;
    const std::string FormatTagData(std::map<std::string,std::string>tiles);
    friend std::ostream& operator << (std::ostream& os, XMLTag& rhs);
    std::string str();
    int GetInteger(std::string dataTag);
    float GetFloat(std::string dataTag);
    double GetDouble(std::string dataTag);
    bool GetBool(std::string dataTag);
    std::string GetString(std::string dataTag)const;
};

struct ForegroundTileTag:public XMLTag{
    //Whether or not to hide this foreground tile
    bool hide=true;
};

struct MapTag{
    int width=0,height=0;
    int tilewidth=0,tileheight=0;
    bool optimized=false; //An optimized map will require us to flatten it out and use it as a single tile.
    bool provideOptimization=false; //An optimized map will require us to flatten it out and use it as a single tile.
    vi2d playerSpawnLocation;
    vi2d MapSize; //The number of tiles in width and height of this map.
    vi2d TileSize; //How large in pixels the map's tiles are.
    MapTag();
    MapTag(int width,int height,int tilewidth,int tileheight);
    friend std::ostream& operator << (std::ostream& os, MapTag& rhs);
};

struct LayerTag{
    XMLTag tag;
    std::vector<std::vector<int>> tiles;
    std::string unlockCondition="";
    std::string str();
};

struct ZoneData{
    geom2d::rect<int>zone;
    bool isUpper=false;
    std::vector<XMLTag>properties;
};

struct Tunnel{
    vf2d worldPos;
    uint16_t linkedTo;
};

using TunnelId=uint16_t;

struct Map{
    friend class AiL;
    friend class TMXParser;
private:
    MapTag MapData;
    std::string name;
    Renderable*optimizedTile=nullptr;
    std::vector<XMLTag> TilesetData;
    std::vector<LayerTag> LayerData;
    std::string mapType="";
    std::set<std::string>spawns;
    std::map<std::string,std::vector<::ZoneData>>ZoneData;
    std::unordered_map<TunnelId,Tunnel>TunnelData;
    geom2d::rect<int>SpawnZone;
public:
    Map();
    void _SetMapData(MapTag data);
    bool skipLoadoutScreen=false;
    const MapTag&GetMapData()const;
    const std::string_view GetMapType()const;
    const std::vector<LayerTag>&GetLayers()const;
    const std::unordered_map<uint16_t,Tunnel>&GetTunnels()const;
    const MapName&GetMapName()const;
    const std::string_view GetMapDisplayName()const;
    const Renderable*const GetOptimizedMap()const;
    const std::map<std::string,std::vector<::ZoneData>>&GetZones()const;
    const geom2d::rect<int>&GetSpawnZone()const;
    std::string FormatLayerData(std::ostream& os, std::vector<LayerTag>tiles);
    friend std::ostream& operator << (std::ostream& os, Map& rhs);
    friend std::ostream& operator << (std::ostream& os, std::vector<XMLTag>& rhs);
};

struct Property{
    std::string name;
    std::string value;
    int GetInteger();
    float GetFloat();
    double GetDouble();
    bool GetBool();
};

class TMXParser{
    public:
    const Map&GetData()const;
    private:
    Map parsedMapInfo;
    std::string fileName;
    ZoneData*prevZoneData=nullptr;
    void ParseTag(std::string tag);
    int monsterPropertyTagCount=-1;
    bool infiniteMap=false;
    LayerTag*currentLayerTag=nullptr;
    std::vector<std::pair<TunnelId,TunnelId>>TunnelLinks;
    uint16_t previousTunnelId=0U;
    public:
    TMXParser(std::string file);
};

//#define TMX_PARSER_SETUP //Toggle for code-writing.

#ifdef TMX_PARSER_SETUP
#undef TMX_PARSER_SETUP
    const std::string XMLTag::FormatTagData(std::map<std::string,std::string>tiles){
        std::string displayStr="";
        for (std::map<std::string,std::string>::iterator it=data.begin();it!=data.end();it++) {
            displayStr+="  "+it->first+": "+it->second+"\n";
        }
        return displayStr;
    }
    std::ostream& operator << (std::ostream& os, XMLTag& rhs){ 
        os << 
            rhs.tag <<"\n"<< 
            rhs.FormatTagData(rhs.data) <<"\n";
        return os;
    }
    std::ostream& operator << (std::ostream& os, MapTag& rhs){ 
        os << 
            "(width:"<<rhs.width<<", height:"<<rhs.height<<", Tile width:"<<rhs.tilewidth<<", Tile height:"<<rhs.tileheight<<",playerSpawnLocation:"<<rhs.playerSpawnLocation<<")\n";
        return os;
    }
    int XMLTag::GetInteger(std::string dataTag) {
        return std::stoi(data[dataTag]);
    }
    float XMLTag::GetFloat(std::string dataTag) {
        return std::stof(data[dataTag]);
    }
    double XMLTag::GetDouble(std::string dataTag) {
        return std::stod(data[dataTag]);
    }
    std::string XMLTag::GetString(std::string dataTag)const{
        return data.at(dataTag);
    }
    bool XMLTag::GetBool(std::string dataTag) {
        if (data[dataTag]=="0"||data[dataTag]=="false") {
            return false;
        }else{
            return true;
        }
    }
    int Property::GetInteger() {
        return std::stoi(value);
    }
    float Property::GetFloat() {
        return std::stof(value);
    }
    double Property::GetDouble() {
        return std::stod(value);
    }
    bool Property::GetBool() {
        if (value=="0") {
            return false;
        }else{
            return true;
        }
    }
    const std::unordered_map<uint16_t,Tunnel>&Map::GetTunnels()const{
        return TunnelData;
    }
    Map::Map(){
        ZoneData["UpperZone"];
        ZoneData["LowerZone"];
    }
    const geom2d::rect<int>&Map::GetSpawnZone()const{
        return SpawnZone;
    }
    MapTag::MapTag(){}
    MapTag::MapTag(int width,int height,int tilewidth,int tileheight)
        :width(width),height(height),tilewidth(tilewidth),tileheight(tileheight),MapSize({width,height}),TileSize({tilewidth,tileheight}){}
    std::string XMLTag::str() {
        return tag+"\n";
    }
    std::string LayerTag::str() {
        std::string displayStr=tag.tag+"\n"+tag.FormatTagData(tag.data);
        displayStr+="  DATA ("+std::to_string(tiles[0].size())+"x"+std::to_string(tiles.size())+"  )\n";
        return displayStr;
    }
    std::string Map::FormatLayerData(std::ostream& os, std::vector<LayerTag>tiles) {
        std::string displayStr;
        for (int i=0;i<LayerData.size();i++) {
            displayStr+=LayerData[i].str();
        }
        return displayStr;
    }
    const std::string_view Map::GetMapType()const{
        return mapType;
    }
    const MapName&Map::GetMapName()const{
        return name;
    }
    void Map::_SetMapData(MapTag data){
        MapData=data;
    }
    const MapTag&Map::GetMapData()const{
        return MapData;
    }
    const std::vector<LayerTag>&Map::GetLayers()const{
        return LayerData;
    }
    const std::string_view Map::GetMapDisplayName()const{
        return name;
    }
    const Renderable*const Map::GetOptimizedMap()const{
        return optimizedTile;
    }
    std::ostream& operator <<(std::ostream& os, std::vector<XMLTag>& rhs) { 
        for(XMLTag&tag:rhs){
            os << 
            tag<<"\n";
        }
        return os;
    }
    std::ostream& operator <<(std::ostream& os, Map& rhs) { 
        os << 
            rhs.MapData <<"\n"<< 
            rhs.TilesetData <<"\n"<< 
            rhs.FormatLayerData(os,rhs.LayerData) <<"\n";
        return os;
    }
    const Map&TMXParser::GetData()const{
        return parsedMapInfo;
    }
    void TMXParser::ParseTag(std::string tag) {
        auto ReadNextTag=[&](){
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
                    }else{
                        std::string key = data.substr(0,data.find("="));
                        std::string value = data.substr(data.find("=")+1,std::string::npos);

                        //Strip Quotation marks.
                        value = value.substr(1,std::string::npos);
                        value = value.substr(0,value.length()-1);

                        newTag.data[key]=value;
                    }
                }
            }
            return newTag;
        };

        XMLTag newTag=ReadNextTag();

        auto ParseMonsterTemplateName=[](const XMLTag&monsterTag){
            std::string monsterName=monsterTag.GetString("template").substr("../maps/Monsters/"s.length());
            monsterName=monsterName.substr(0,monsterName.find(".tx"));
            return monsterName;
        };

        if (newTag.tag=="map"){
            if(stoi(newTag.data["infinite"])==1){
                infiniteMap=true;
                throw std::runtime_error{std::format("WARNING! Infinite maps are not supported! Invalid map: {}",fileName)};
                return;
            }
            parsedMapInfo.MapData={stoi(newTag.data["width"]),stoi(newTag.data["height"]),stoi(newTag.data["tilewidth"]),stoi(newTag.data["tileheight"])};
        }else
        if (newTag.tag=="tileset"){
            parsedMapInfo.TilesetData.push_back(newTag);
        }else
        if (newTag.tag=="layer"){
            LayerTag l = {newTag};
            parsedMapInfo.LayerData.push_back(l);
            currentLayerTag=&parsedMapInfo.LayerData.back();
        }else 
        if(newTag.tag=="property"&&prevZoneData!=nullptr){
            //This is a property for a zone that doesn't fit into the other categories, we add it to the previous zone data encountered.
            prevZoneData->properties.push_back(newTag);
        }else
        if(newTag.tag=="property"&&newTag.GetString("name")=="Link"){
            TunnelLinks.emplace_back(previousTunnelId,newTag.GetInteger("value"));
        }else
        if (newTag.tag=="object"&&newTag.data.find("type")!=newTag.data.end()){
            if (newTag.GetString("type")=="Tunnel"){
                previousTunnelId=newTag.GetInteger("id");
                parsedMapInfo.TunnelData.insert({previousTunnelId,Tunnel{vi2d{newTag.GetInteger("x"),newTag.GetInteger("y")}/16*16}});
            }else
               if (newTag.GetString("type")=="SpawnZone"){
                parsedMapInfo.SpawnZone={vi2d{newTag.GetInteger("x"),newTag.GetInteger("y")},vi2d{newTag.GetInteger("width"),newTag.GetInteger("height")}};
            }else{
                //This is an object with a type that doesn't fit into other categories, we can add it to ZoneData.
                std::vector<ZoneData>&zones=parsedMapInfo.ZoneData[newTag.data["type"]];
                float width=1.f;
                float height=1.f;
                if(newTag.data.count("width")>0)width=newTag.GetFloat("width");
                if(newTag.data.count("height")>0)height=newTag.GetFloat("height");
                zones.emplace_back(geom2d::rect<int>{{newTag.GetInteger("x"),newTag.GetInteger("y")},{int(width),int(height)}});
                prevZoneData=&zones.back();
            }
        }
    }
    TMXParser::TMXParser(std::string file){
        fileName=file;
        std::ifstream f(file,std::ios::in);

        std::string accumulator="";

        while (f.good()&&!infiniteMap) {
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
                        accumulator=""; //Restart because this is an end tag.
                    }
                    if(accumulator.length()>1&&accumulator.find('>')!=std::string::npos){
                        accumulator=""; //Restart because this tag has nothing in it!
                    }
                }else{
                    //Start reading in data for this layer.
                    std::vector<int>rowData;
                    while (data.find(",")!=std::string::npos) {
                        std::string datapiece = data.substr(0,data.find(","));
                        data = data.substr(data.find(",")+1,std::string::npos);
                        rowData.push_back(stoul(datapiece));
                    }
                    if (data.length()) {
                        rowData.push_back(stoul(data));
                    }
                    parsedMapInfo.LayerData[parsedMapInfo.LayerData.size()-1].tiles.push_back(rowData);
                }
        }
        std::sort(parsedMapInfo.TilesetData.begin(),parsedMapInfo.TilesetData.end(),[](XMLTag&t1,XMLTag&t2){return t1.GetInteger("firstgid")<t2.GetInteger("firstgid");});
        for(const std::pair<TunnelId,TunnelId>&link:TunnelLinks){
            parsedMapInfo.TunnelData.at(link.first).linkedTo=link.second;
            parsedMapInfo.TunnelData.at(link.second).linkedTo=link.first;
        }
    }
        

#endif
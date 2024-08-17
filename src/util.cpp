#include "util.h"

std::random_device rd;
std::mt19937 rng(rd());

float olc::util::random(float range){
	static std::uniform_real_distribution<float>distrib(0,1);
	return distrib(rng)*range;
}

int olc::util::random(){
	static std::uniform_int_distribution<int>distrib(0,32767);
	return distrib(rng);
}

const float olc::util::random_range(const float min,const float max){
	return random(max-min)+min;
}
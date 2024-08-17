#include "util.h"
#include "olcUTIL_Geometry2D.h"

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

float olc::util::angle_difference(float angle_1, float angle_2)
{
    angle_1 = fmod(angle_1, 2 * geom2d::pi);
    angle_2 = fmod(angle_2, 2 * geom2d::pi);
    float angle_diff = angle_1 - angle_2;

    if (angle_diff > geom2d::pi)
        angle_diff -= 2 * geom2d::pi;
    else if (angle_diff < -geom2d::pi)
        angle_diff += 2 * geom2d::pi;

    return -angle_diff;
}


void olc::util::turn_towards_direction(float&angle,float target,float rate)
{
    const auto median_3=[](float a,float b,float c){
        return std::max(std::min(a, b), std::min(std::max(a, b), c));
    };

    float diff = angle_difference(angle,target);
    angle += median_3(-rate, rate, diff);

    float newAngleDiff = angle_difference(angle,target);

    if(diff>0&&newAngleDiff<0||
        diff<0&&newAngleDiff>0)angle=fmod(target,2*geom2d::pi); //We have crossed the angle difference threshold and can safely say we reached it.
}

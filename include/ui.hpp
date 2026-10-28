#pragma once
#include "game.hpp"
struct GLFWwindow;
namespace lvh {
void initUI(GLFWwindow*); void stopUI(); void beginUI();
void drawUI(Game&,float fps,int width,int height,const std::string& gpu,bool showHelp);
void finishUI();
}


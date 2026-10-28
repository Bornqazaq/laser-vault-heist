#include "ui.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <cmath>
namespace lvh {
void initUI(GLFWwindow*w){
 IMGUI_CHECKVERSION();ImGui::CreateContext();auto&io=ImGui::GetIO();io.IniFilename=nullptr;
 io.FontGlobalScale=1.30f;ImGui::StyleColorsDark();auto&style=ImGui::GetStyle();style.WindowRounding=9;style.FrameRounding=5;style.WindowPadding={20,16};
 style.Colors[ImGuiCol_WindowBg]={.035f,.032f,.026f,.94f};style.Colors[ImGuiCol_Text]={.94f,.9f,.81f,1};style.Colors[ImGuiCol_Button]={.34f,.24f,.11f,1};
 ImGui_ImplGlfw_InitForOpenGL(w,true);ImGui_ImplOpenGL3_Init("#version 330");
}
void stopUI(){ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();}
void beginUI(){ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();}
void finishUI(){ImGui::Render();ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());}
void drawUI(Game&g,float fps,int width,int height,const std::string&gpu,bool help){
 auto*dl=ImGui::GetForegroundDrawList();ImU32 ivory=IM_COL32(239,227,202,245),gold=IM_COL32(207,166,92,255);
 dl->AddRectFilled({24,24},{350,101},IM_COL32(10,12,12,195),5);dl->AddRectFilled({24,24},{27,101},gold);
 dl->AddText({43,37},gold,"LASER VAULT HEIST");dl->AddText({43,66},ivory,"Reach the vault. Leave no alarm.");
 char counter[64];snprintf(counter,64,"%.0f FPS  |  OPENGL",fps);
 dl->AddRectFilled({float(width-240),24},{float(width-24),62},IM_COL32(10,12,12,195),5);dl->AddText({float(width-224),34},ivory,counter);
 if(g.state==State::Playing){
  float x=width*.5f,y=height*.5f;dl->AddCircleFilled({x,y},2,ivory);
  std::string posture=g.crouching?"CROUCHED":"STANDING";dl->AddText({30,float(height-48)},gold,posture.c_str());
  if(!g.hint.empty()){auto size=ImGui::CalcTextSize(g.hint.c_str());dl->AddRectFilled({x-size.x*.5f-18,float(height-112)},{x+size.x*.5f+18,float(height-72)},IM_COL32(8,10,11,215),5);dl->AddText({x-size.x*.5f,float(height-102)},ivory,g.hint.c_str());}
  dl->AddText({float(width-350),float(height-48)},ivory,"WASD  Move    CTRL  Crouch    F1  Help");
 }
 if(g.state==State::Alarm||g.state==State::Victory||g.state==State::Pause){
  dl->AddRectFilled({0,0},{float(width),float(height)},IM_COL32(3,5,6,130));
  ImGui::SetNextWindowPos({width*.5f,height*.5f},ImGuiCond_Always,{.5f,.5f});ImGui::SetNextWindowSize({460,220});
  ImGui::Begin("Outcome",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove);
  ImGui::TextColored(g.state==State::Alarm?ImVec4(1,.25f,.18f,1):ImVec4(.88f,.72f,.4f,1),"%s",g.state==State::Alarm?"ALARM TRIGGERED":g.state==State::Victory?"VAULT OPENED":"PAUSED");
  ImGui::Spacing();ImGui::TextWrapped("%s",g.state==State::Alarm?"A laser touched you. Watch its timing and use the low passages.":g.state==State::Victory?"Quiet steps. Clean execution. The vault is yours.":"The room will wait. Take a breath.");
  ImGui::Spacing();ImGui::Text("%s",g.state==State::Pause?"ESC   Return to the heist":"R   Start another attempt");ImGui::Text("ESC   Pause / release mouse");ImGui::End();
 }
 if(help){ImGui::SetNextWindowPos({24,125},ImGuiCond_Always);ImGui::SetNextWindowSize({420,330});ImGui::Begin("FIELD NOTES",nullptr,ImGuiWindowFlags_NoResize);
 ImGui::Text("WASD         Walk");ImGui::Text("Mouse        Look");ImGui::Text("CTRL / C     Crouch");ImGui::Text("E            Interact");ImGui::Text("R            Retry after alarm / victory");ImGui::Text("ESC          Pause");ImGui::Text("F1           Toggle this guide");ImGui::Text("F12          Save screenshot");ImGui::Spacing();
 ImGui::TextWrapped("Desk lamp, CCTV selector and vault wheel are interactive. All scene lights turn red when a laser is touched.");ImGui::TextWrapped("%s",gpu.c_str());ImGui::End();}
}
}


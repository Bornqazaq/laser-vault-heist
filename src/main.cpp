#include "scene.hpp"
#include "game.hpp"
#include "ui.hpp"
#include "animation.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <cmath>
namespace {
struct Mouse {lvh::Game* game;bool locked=true,first=true;double x=0,y=0;};
void cursor(GLFWwindow*w,double x,double y){auto*m=static_cast<Mouse*>(glfwGetWindowUserPointer(w));if(!m||!m->locked)return;if(m->first){m->x=x;m->y=y;m->first=false;}
 m->game->yaw+=float(x-m->x)*.105f;m->game->pitch=std::clamp(m->game->pitch-float(y-m->y)*.105f,-85.f,85.f);m->x=x;m->y=y;}
void error(int code,const char*msg){std::cerr<<"GLFW "<<code<<": "<<msg<<'\n';}
}
int main(int argc,char**argv){
 using namespace lvh;
 bool grey=false,hidden=false,help=false,preview=false,facePreview=false,restPose=false;std::string previewClip="Idle";int maxFrames=0,wantedWidth=1280,wantedHeight=720;fs::path screenshot,root=fs::absolute(argv[0]).parent_path();
 for(int i=1;i<argc;i++){std::string arg=argv[i];if(arg=="--greybox")grey=true;else if(arg=="--character-preview")preview=true;else if(arg=="--face-preview"){preview=facePreview=true;}else if(arg=="--rest-pose")restPose=true;else if(arg=="--clip"&&i+1<argc)previewClip=argv[++i];else if(arg=="--hidden")hidden=true;else if(arg=="--frames"&&i+1<argc)maxFrames=std::stoi(argv[++i]);else if(arg=="--screenshot"&&i+1<argc)screenshot=argv[++i];else if(arg=="--root"&&i+1<argc)root=argv[++i];else if(arg=="--1080p"){wantedWidth=1920;wantedHeight=1080;}else{std::cerr<<"Unknown argument: "<<arg<<'\n';return 2;}}
 if(preview){wantedWidth=1100;wantedHeight=1100;}
 glfwSetErrorCallback(error);if(!glfwInit())return 1;glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
 glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
 glfwWindowHint(GLFW_VISIBLE,hidden?GLFW_FALSE:GLFW_TRUE);glfwWindowHint(GLFW_SAMPLES,0);
 GLFWwindow*window=glfwCreateWindow(wantedWidth,wantedHeight,"Laser Vault Heist",nullptr,nullptr);if(!window){glfwTerminate();return 1;}
 glfwMakeContextCurrent(window);glfwSwapInterval(0);if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))){glfwDestroyWindow(window);glfwTerminate();return 1;}
 int result=0;
 try{
 std::string gpu=reinterpret_cast<const char*>(glGetString(GL_RENDERER));std::string version=reinterpret_cast<const char*>(glGetString(GL_VERSION));
 std::cout<<"OpenGL "<<version<<"\nGPU "<<gpu<<"\n";fs::create_directories(root/"evidence");std::ofstream(root/"evidence"/"runtime-info.txt")<<"OpenGL: "<<version<<"\nGPU: "<<gpu<<"\n";
 Game game;Mouse mouse{&game};glfwSetWindowUserPointer(window,&mouse);glfwSetCursorPosCallback(window,cursor);glfwSetInputMode(window,GLFW_CURSOR,hidden?GLFW_CURSOR_NORMAL:GLFW_CURSOR_DISABLED);
 fs::current_path(root);
 Scene scene;Animator animator;if(grey)scene.greybox();else {if(!preview)scene.load(root/"assets"/"vault.scene");scene.load(root/"assets"/"thief.scene");animator.load(root/"assets"/"thief.lvanim");}Renderer renderer(root/"shaders");renderer.characterPreview=preview;initUI(window);
 double last=glfwGetTime(),accum=0,fpsTime=last;int frame=0,fpsFrames=0;float fps=0;
 bool wasE=false,wasR=false,wasEsc=false,wasF1=false,wasF12=false;State beforePause=State::Playing;
 std::ofstream timings(root/"evidence"/"frame-times.csv");timings<<"frame,seconds,frame_ms,state\n";
 while(!glfwWindowShouldClose(window)){
 double now=glfwGetTime(),elapsed=now-last;last=now;glfwPollEvents();
 auto key=[&](int k){return glfwGetKey(window,k)==GLFW_PRESS;};
 bool esc=key(GLFW_KEY_ESCAPE);if(esc&&!wasEsc){if(game.state==State::Pause)game.state=beforePause;else{beforePause=game.state;game.state=State::Pause;}mouse.locked=game.state!=State::Pause;mouse.first=true;glfwSetInputMode(window,GLFW_CURSOR,mouse.locked?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);}wasEsc=esc;
 bool f1=key(GLFW_KEY_F1);if(f1&&!wasF1)help=!help;wasF1=f1;
 Input input;input.forward=float(key(GLFW_KEY_W))-float(key(GLFW_KEY_S));input.right=float(key(GLFW_KEY_D))-float(key(GLFW_KEY_A));input.crouch=key(GLFW_KEY_LEFT_CONTROL)||key(GLFW_KEY_C);
 bool e=key(GLFW_KEY_E),r=key(GLFW_KEY_R);input.interact=e&&!wasE;input.retry=r&&!wasR;wasE=e;wasR=r;
 // 240 Hz bounded fixed updates prevent a thin beam being skipped during a slow frame.
 // Stall time beyond one second is discarded rather than a single unbounded step.
 accum+=std::min(elapsed,1.0);while(accum>=1.0/240.0){game.update(1.f/240,input);input.interact=input.retry=false;accum-=1.0/240.0;}
 for(int i=0;i<6;i++){auto&l=game.lasers[i];scene.setLocal("Laser"+std::to_string(i),glm::scale(l.matrix,l.halfSize*2.f));for(int side:{-1,1})scene.setLocal("EmitterRoot"+std::to_string(i)+"_"+std::to_string(side),l.matrix*glm::translate(glm::mat4(1),glm::vec3(side*l.halfSize.x,0,0)));}
 auto animate=[&](const std::string&name,const glm::mat4&change){int i=scene.find(name);if(i>=0)scene.nodes[i].local=scene.nodes[i].base*change;};
 float wheel=std::min(game.safeTime,1.f)*glm::radians(150.f),door=glm::smoothstep(1.f,3.3f,game.safeTime)*glm::radians(100.f);
 animate("SafeWheel",glm::rotate(glm::mat4(1),wheel,glm::vec3(0,0,1)));animate("SafeDoorPivot",glm::rotate(glm::mat4(1),-door,glm::vec3(0,1,0)));
 float angle=.65f*std::sin(game.time*.5f);animate("SpotPivot0",glm::rotate(glm::mat4(1),angle,glm::vec3(0,1,0)));animate("SpotPivot1",glm::rotate(glm::mat4(1),-angle,glm::vec3(0,1,0)));
 scene.lampOn=game.lamp;
 for(int bolt=0;bolt<3;bolt++)animate("SafeBolt"+std::to_string(bolt),glm::translate(glm::mat4(1),glm::vec3(-.22f*glm::smoothstep(.5f,1.15f,game.safeTime),0,0)));
 bool alarm=game.state==State::Alarm||(game.state==State::Pause&&beforePause==State::Alarm);
 glm::vec3 warm=alarm?glm::vec3(1,.015f,.006f):glm::vec3(1,.60f,.25f),cool=alarm?glm::vec3(1,.01f,.005f):glm::vec3(.40f,.58f,1.f);
 std::vector<Light>lights={{{-3.5f,2.3f,-5},warm,{0,-1,0},2.2f},{{3.5f,2.3f,-4},warm,{0,-1,0},2.5f},{{-2,2.7f,-10.8f},warm,{0,-1,0},2.4f},{{2,2.7f,-10.8f},warm,{0,-1,0},2.4f},{{-3.5f,2.7f,-2},cool,{0,-1,0},1.7f},{{-2.8f,1.25f,-2.1f},warm,{0,-1,0},game.lamp?1.7f:0.f}};
 for(int i=0;i<2;i++){float a=i==0?angle:-angle;glm::vec3 p(i==0?-2.2f:2.2f,3.6f,-6.5f);glm::vec3 direction=glm::normalize(glm::vec3(std::sin(a)*.65f,-1,-.35f));lights.push_back({p,warm,direction,4.f,.88f,.75f});}
 for(auto& light:lights)light.power*=.42f;
 int width,height;glfwGetFramebufferSize(window,&width,&height);if(width<=0||height<=0){glfwWaitEventsTimeout(.05);last=glfwGetTime();continue;}renderer.width=width;renderer.height=height;
 if(!grey){
 std::string clip=preview?previewClip:game.interactionTime>0?"Interact":game.crouching?(input.forward||input.right?"CrouchWalk":"CrouchIdle"):(input.forward||input.right?"Walk":"Idle");
 float phase=clip=="Walk"?game.stepDistance/1.65f:clip=="CrouchWalk"?game.stepDistance/1.05f:clip=="Interact"?1.f-game.interactionTime/.7f:game.time/2.f;
 if(preview)phase=game.time/(clip=="Walk"?1.f:clip=="CrouchWalk"?1.3333f:2.f);
 animator.update(clip,phase,float(elapsed),preview&&restPose);scene.skinPalette=animator.palette;
 scene.characterModel=preview?glm::mat4(1):glm::translate(glm::mat4(1),game.position)*glm::rotate(glm::mat4(1),glm::radians(90.f-game.yaw),glm::vec3(0,1,0));
 }
 glm::vec3 eyePos=game.camera();glm::mat4 view=game.view();
 if(preview){eyePos=facePreview?glm::vec3(0,1.69f,.58f):glm::vec3(0,1.02f,2.05f);view=glm::lookAt(eyePos,facePreview?glm::vec3(0,1.69f,0):glm::vec3(0,.98f,0),glm::vec3(0,1,0));
 lights={{{-1.2f,2.3f,2},glm::vec3(1,.85f,.72f),{0,-1,0},1.2f},{{1.4f,1.9f,1.2f},glm::vec3(.65f,.78f,1),{0,-1,0},.9f},{{0,2,-1},glm::vec3(1,.7f,.4f),{0,-1,0},1.f}};}
 renderer.frame(scene,view,eyePos,lights,alarm,game.cctvView,game.time);
 fpsFrames++;if(now-fpsTime>.35){fps=float(fpsFrames/(now-fpsTime));fpsFrames=0;fpsTime=now;}
 beginUI();drawUI(game,fps,width,height,gpu,help);finishUI();
 bool f12=key(GLFW_KEY_F12);if(f12&&!wasF12)saveFramebuffer(root/"evidence"/("capture-"+std::to_string(frame)+".png"),width,height);wasF12=f12;
 frame++;timings<<frame<<','<<now<<','<<elapsed*1000.0<<','<<int(game.state)<<'\n';
 if(maxFrames&&frame>=maxFrames){if(!screenshot.empty())saveFramebuffer(screenshot,width,height);glfwSetWindowShouldClose(window,true);}
 glfwSwapBuffers(window);
 GLenum glError=glGetError();if(glError!=GL_NO_ERROR)throw std::runtime_error("OpenGL error "+std::to_string(glError));
 }
 stopUI();std::cout<<"Rendered "<<frame<<" frames without GL errors\n";
 }catch(const std::exception&e){std::cerr<<"FATAL: "<<e.what()<<'\n';result=1;}
 glfwDestroyWindow(window);glfwTerminate();return result;
}


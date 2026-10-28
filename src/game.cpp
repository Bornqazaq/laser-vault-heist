#include "game.hpp"
#include <algorithm>
#include <cmath>
namespace lvh {
bool pointInBox(glm::vec3 point,const glm::mat4&model,glm::vec3 halfSize){glm::vec3 p=glm::vec3(glm::inverse(model)*glm::vec4(point,1));return glm::all(glm::lessThanEqual(glm::abs(p),halfSize+glm::vec3(1e-6f)));}
bool sampledBodyHits(const Laser&l,glm::vec3 feet,float height){
 // Local-space point/box tests sample the player's upright body. The small
 // local margin is conservative for our beam roll limits (at most 14 degrees).
 for(float h=.12f;h<=height+.001f;h+=.08f)if(pointInBox(feet+glm::vec3(0,h,0),l.matrix,l.halfSize+glm::vec3(.16f,.055f,.16f)))return true;
 return pointInBox(feet+glm::vec3(0,height,0),l.matrix,l.halfSize+glm::vec3(.16f,.055f,.16f));
}
Game::Game(){updateLasers();}
void Game::reset(){int attempts=retries+1;*this=Game();retries=attempts;}
glm::vec3 Game::forward()const{float y=glm::radians(yaw),p=glm::radians(pitch);return glm::normalize(glm::vec3(cos(y)*cos(p),sin(p),sin(y)*cos(p)));}
glm::vec3 Game::camera()const{return position+glm::vec3(0,eye,0);}
glm::mat4 Game::view()const{return glm::lookAt(camera(),camera()+forward(),glm::vec3(0,1,0));}
void Game::updateLasers(){
 const float depths[]={-2.9f,-4.2f,-5.6f,-6.9f,-8.15f,-9.5f};
 for(int i=0;i<6;i++){
 auto&l=lasers[i];float height=1.35f;float angle=0;
 if(i==0)height=2.35f+.55f*std::sin(time*.65f);
 if(i==1)height=1.0f+.62f*std::sin(time*.8f+.8f);
 if(i==2){height=2.35f;angle=glm::radians(10.f)*std::sin(time*.7f);}
 if(i==3)height=1.1f+.60f*std::sin(time*.6f+1.3f);
 if(i==4){height=1.65f;angle=glm::radians(7.f)*std::sin(time*.75f);}
 if(i==5)height=1.23f;
 l.center={0,height,depths[i]};l.angle=angle;
 // A left-wall pivot makes the rotating beam's origin explicit.
 glm::vec3 pivot(-3.65f,height,depths[i]);
 l.halfSize.x=3.65f/std::cos(angle);
 l.matrix=glm::translate(glm::mat4(1),pivot)*glm::rotate(glm::mat4(1),angle,glm::vec3(0,0,1))*glm::translate(glm::mat4(1),glm::vec3(l.halfSize.x,0,0));
 }
}
bool Game::blocked(glm::vec3 p)const{
 if(p.x < -3.65f||p.x>3.65f||p.z>-.35f||p.z< -11.55f)return true;
 auto box=[&](float x0,float x1,float z0,float z1){return p.x>x0-.22f&&p.x<x1+.22f&&p.z>z0-.22f&&p.z<z1+.22f;};
 return box(-3.7f,-2.05f,-3.8f,-1.5f)||box(-3.85f,-3.05f,-9.6f,-4.4f)||box(-1.65f,1.65f,-11.9f,-10.7f);
}
bool Game::playerHit()const{for(auto&l:lasers)if(sampledBodyHits(l,position,crouching?1.12f:1.78f))return true;return false;}
int Game::interactionTarget()const{
 const glm::vec3 points[]={{-2.85f,1.24f,-1.97f},{-2.78f,1.27f,-3.0f},{0,1.65f,-10.9f}};
 int target=0;float best=-100;
 for(int i=0;i<3;i++){glm::vec3 delta=points[i]-camera();float distance=glm::length(delta);if(distance>2.15f)continue;float alignment=glm::dot(forward(),delta/std::max(.001f,distance));if(alignment<.40f)continue;float score=alignment*3-distance;if(score>best){best=score;target=i+1;}}
 return target;
}
void Game::interact(){
 if(state!=State::Playing)return;
 int target=interactionTarget();
 if(target==1){lamp=!lamp;interactionTime=.6f;}
 if(target==2){cctvView=1-cctvView;interactionTime=.6f;}
 if(target==3&&lastCrossed){safeOpened=true;interactionTime=.7f;}
}
void Game::update(float dt,const Input&i){
 if(i.retry&&(state==State::Alarm||state==State::Victory)){reset();return;}
 if(state!=State::Playing)return;
 time+=dt;updateLasers();interactionTime=std::max(0.f,interactionTime-dt);
 crouching=i.crouch;float target=crouching?1.04f:1.67f;eye+=(target-eye)*std::min(1.f,dt*13.f);
 glm::vec3 f=forward();f.y=0;f=glm::normalize(f);glm::vec3 right=glm::normalize(glm::cross(f,glm::vec3(0,1,0)));
 glm::vec3 motion=f*i.forward+right*i.right;if(glm::length(motion)>1)motion=glm::normalize(motion);
 motion*=dt*(crouching?1.25f:2.45f);glm::vec3 old=position,p=position;p.x+=motion.x;if(!blocked(p))position.x=p.x;p=position;p.z+=motion.z;if(!blocked(p))position.z=p.z;stepDistance+=glm::length(position-old);
 if(playerHit()){state=State::Alarm;return;}
 if(position.z< -9.74f)lastCrossed=true;
 if(i.interact)interact();
 if(safeOpened){safeTime+=dt;if(safeTime>3.4f)state=State::Victory;}
 hint="";
 switch(interactionTarget()){
 case 1:hint=lamp?"E   Switch desk lamp off":"E   Switch desk lamp on";break;
 case 2:hint="E   Switch security camera";break;
 case 3:hint=lastCrossed?"E   Open the vault":"Cross the final laser to unlock the vault";break;
 }

}
}


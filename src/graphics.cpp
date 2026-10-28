#include "scene.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <array>
namespace lvh {
std::string readText(const fs::path&p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Cannot read "+p.string());std::ostringstream s;s<<f.rdbuf();return s.str();}
static GLuint compile(GLenum type,const fs::path&p){
 std::string source=readText(p);const char*text=source.c_str();GLuint id=glCreateShader(type);
 glShaderSource(id,1,&text,nullptr);glCompileShader(id);GLint ok=0;glGetShaderiv(id,GL_COMPILE_STATUS,&ok);
 if(!ok){std::array<char,4096> log{};glGetShaderInfoLog(id,int(log.size()),nullptr,log.data());glDeleteShader(id);throw std::runtime_error(p.string()+": "+log.data());}return id;
}
Shader::Shader(const fs::path&v,const fs::path&f){
 GLuint vs=compile(GL_VERTEX_SHADER,v),ps=compile(GL_FRAGMENT_SHADER,f);id=glCreateProgram();
 glAttachShader(id,vs);glAttachShader(id,ps);glLinkProgram(id);glDeleteShader(vs);glDeleteShader(ps);
 GLint ok;glGetProgramiv(id,GL_LINK_STATUS,&ok);if(!ok){char log[4096]{};glGetProgramInfoLog(id,4096,nullptr,log);throw std::runtime_error(std::string("Shader link: ")+log);}
}
Shader::~Shader(){if(id)glDeleteProgram(id);}void Shader::use()const{glUseProgram(id);}
void Shader::set(const char*n,int v)const{glUniform1i(glGetUniformLocation(id,n),v);}
void Shader::set(const char*n,float v)const{glUniform1f(glGetUniformLocation(id,n),v);}
void Shader::set(const char*n,glm::vec3 v)const{glUniform3fv(glGetUniformLocation(id,n),1,glm::value_ptr(v));}
void Shader::set(const char*n,const glm::mat4&v)const{glUniformMatrix4fv(glGetUniformLocation(id,n),1,GL_FALSE,glm::value_ptr(v));}
Mesh::Mesh(const std::vector<Vertex>&v,const std::vector<unsigned>&i){
 count=GLsizei(i.size());glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glGenBuffers(1,&ebo);
 glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(v.size()*sizeof(Vertex)),v.data(),GL_STATIC_DRAW);
 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);glBufferData(GL_ELEMENT_ARRAY_BUFFER,GLsizeiptr(i.size()*sizeof(unsigned)),i.data(),GL_STATIC_DRAW);
 const int sizes[]={3,3,2,4,4};const size_t offsets[]={offsetof(Vertex,position),offsetof(Vertex,normal),offsetof(Vertex,uv),offsetof(Vertex,weights),offsetof(Vertex,joints)};
 for(unsigned a=0;a<5;a++){glEnableVertexAttribArray(a);glVertexAttribPointer(a,sizes[a],GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsets[a]));}
 glBindVertexArray(0);
}
Mesh::~Mesh(){if(instances)glDeleteBuffers(1,&instances);glDeleteBuffers(1,&vbo);glDeleteBuffers(1,&ebo);glDeleteVertexArrays(1,&vao);}
void Mesh::draw()const{glBindVertexArray(vao);glDrawElements(GL_TRIANGLES,count,GL_UNSIGNED_INT,nullptr);}
void Mesh::setInstances(const std::vector<glm::mat4>&m){
 if(!instances)glGenBuffers(1,&instances);instanceCount=int(m.size());glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,instances);
 glBufferData(GL_ARRAY_BUFFER,GLsizeiptr(m.size()*sizeof(glm::mat4)),m.data(),GL_STATIC_DRAW);
 for(int i=0;i<4;i++){glEnableVertexAttribArray(5+i);glVertexAttribPointer(5+i,4,GL_FLOAT,GL_FALSE,sizeof(glm::mat4),reinterpret_cast<void*>(size_t(i)*sizeof(glm::vec4)));glVertexAttribDivisor(5+i,1);}
}
void Mesh::drawInstances()const{glBindVertexArray(vao);glDrawElementsInstanced(GL_TRIANGLES,count,GL_UNSIGNED_INT,nullptr,instanceCount);}
std::shared_ptr<Mesh> Mesh::box(){
 std::vector<Vertex>v;std::vector<unsigned>ix;
 const glm::vec3 ns[]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
 const glm::vec3 us[]={{0,0,-1},{0,0,1},{1,0,0},{1,0,0},{1,0,0},{-1,0,0}};
 const glm::vec3 vs[]={{0,1,0},{0,1,0},{0,0,-1},{0,0,1},{0,1,0},{0,1,0}};
 for(int f=0;f<6;f++){unsigned k=unsigned(v.size());for(int c=0;c<4;c++){float x=(c==1||c==2)?1.f:-1.f,y=c>=2?1.f:-1.f;v.push_back({(ns[f]+us[f]*x+vs[f]*y)*.5f,ns[f],{(x+1)*.5f,(y+1)*.5f}});}for(unsigned q:{0u,1u,2u,0u,2u,3u})ix.push_back(k+q);}
 return std::make_shared<Mesh>(v,ix);
}
std::shared_ptr<Mesh> Mesh::load(const fs::path&p){
 std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Missing mesh: "+p.string());
 char magic[4];uint32_t version,nv,ni;f.read(magic,4);f.read(reinterpret_cast<char*>(&version),4);f.read(reinterpret_cast<char*>(&nv),4);f.read(reinterpret_cast<char*>(&ni),4);
 if(std::memcmp(magic,"LVHM",4)||version!=1||nv>2000000||ni>12000000)throw std::runtime_error("Unsupported mesh: "+p.string());
 static_assert(sizeof(Vertex)==64,"Runtime vertex layout");std::vector<Vertex>v(nv);std::vector<unsigned>idx(ni);
 f.read(reinterpret_cast<char*>(v.data()),std::streamsize(v.size()*sizeof(Vertex)));f.read(reinterpret_cast<char*>(idx.data()),std::streamsize(idx.size()*4));
 if(!f)throw std::runtime_error("Truncated mesh: "+p.string());for(auto i:idx)if(i>=nv)throw std::runtime_error("Invalid mesh index: "+p.string());
 return std::make_shared<Mesh>(v,idx);
}
GLuint loadTexture(const fs::path&p,bool srgb){
 int w,h,n;stbi_set_flip_vertically_on_load(true);unsigned char*data=stbi_load(p.string().c_str(),&w,&h,&n,4);
 if(!data)throw std::runtime_error("Cannot decode texture: "+p.string());
 GLuint id;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);glTexImage2D(GL_TEXTURE_2D,0,srgb?GL_SRGB8_ALPHA8:GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
 glGenerateMipmap(GL_TEXTURE_2D);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);stbi_image_free(data);return id;
}
void Target::release(){if(depth)glDeleteRenderbuffers(1,&depth);if(color)glDeleteTextures(1,&color);if(fbo)glDeleteFramebuffers(1,&fbo);depth=color=fbo=0;}
Target::~Target(){release();}
void Target::resize(int w,int h,bool hdr){
 if(w==width&&h==height&&fbo)return;release();width=w;height=h;glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
 glGenTextures(1,&color);glBindTexture(GL_TEXTURE_2D,color);glTexImage2D(GL_TEXTURE_2D,0,hdr?GL_RGBA16F:GL_RGBA8,w,h,0,GL_RGBA,GL_FLOAT,nullptr);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);
 glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,w,h);
 glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
 if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Incomplete framebuffer");glBindFramebuffer(GL_FRAMEBUFFER,0);
}
void saveFramebuffer(const fs::path&p,int w,int h){
 std::vector<unsigned char>pixels(size_t(w)*h*3);glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
 stbi_flip_vertically_on_write(true);if(!p.parent_path().empty())fs::create_directories(p.parent_path());if(!stbi_write_png(p.string().c_str(),w,h,3,pixels.data(),w*3))throw std::runtime_error("Cannot write screenshot");
}
Renderer::Renderer(const fs::path&root):surface(root/"surface.vert",root/"surface.frag"),post(root/"post.vert",root/"post.frag"),depthShader(root/"depth.vert",root/"depth.frag"){
 glGenVertexArrays(1,&quad);unsigned char pixel[]={255,255,255,255};glGenTextures(1,&white);glBindTexture(GL_TEXTURE_2D,white);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,pixel);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glGenFramebuffers(1,&shadowFbo);glGenTextures(1,&shadowMap);glBindTexture(GL_TEXTURE_2D,shadowMap);glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,2048,2048,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
 float border[]={1,1,1,1};glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);glBindFramebuffer(GL_FRAMEBUFFER,shadowFbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,shadowMap,0);
 glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Shadow framebuffer incomplete");
 glBindFramebuffer(GL_FRAMEBUFFER,0);cctv.resize(640,360);lightVP=glm::ortho(-9.f,9.f,-9.f,9.f,.1f,35.f)*glm::lookAt(glm::vec3(-8,15,-1),glm::vec3(0,0,-6),glm::vec3(0,1,0));
}
Renderer::~Renderer(){glDeleteVertexArrays(1,&quad);glDeleteTextures(1,&white);glDeleteTextures(1,&shadowMap);glDeleteFramebuffers(1,&shadowFbo);}
void Renderer::draw(Scene&s,const glm::mat4&view,const glm::mat4&proj,glm::vec3 eyePos,const std::vector<Light>&lights,bool alarm,bool security){
 surface.use();surface.set("uView",view);surface.set("uProjection",proj);surface.set("uLightVP",lightVP);surface.set("uEye",eyePos);surface.set("uAlarm",int(alarm));
 surface.set("uTexture",0);surface.set("uShadow",1);surface.set("uLightCount",int(lights.size()));surface.set("uSkinned",0);surface.set("uInstanced",0);
 glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,shadowMap);glActiveTexture(GL_TEXTURE0);
 for(size_t i=0;i<lights.size();i++){std::string prefix="uLights["+std::to_string(i)+"].";const auto&l=lights[i];surface.set((prefix+"position").c_str(),l.position);surface.set((prefix+"color").c_str(),l.color);surface.set((prefix+"direction").c_str(),l.direction);surface.set((prefix+"power").c_str(),l.power);surface.set((prefix+"cutoff").c_str(),l.cutoff);surface.set((prefix+"outer").c_str(),l.outer);}
 for(auto&n:s.nodes){if(n.mesh<0||(n.character&&!security)||(n.screen&&security))continue;
 const auto&m=s.materials.at(n.material);surface.set("uModel",n.world);surface.set("uColor",m.color);surface.set("uSpecular",m.specular);surface.set("uShine",m.shine);surface.set("uEmission",m.emission);surface.set("uScreen",int(n.screen));
 glBindTexture(GL_TEXTURE_2D,n.screen?cctv.color:(m.texture?m.texture:white));s.meshes.at(n.mesh)->draw();
 }
}
void Renderer::frame(Scene&s,const glm::mat4&view,glm::vec3 eyePos,const std::vector<Light>&lights,bool alarm,int cameraIndex,float time){
 (void)time;s.updateWorld();glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);
 glBindFramebuffer(GL_FRAMEBUFFER,shadowFbo);glViewport(0,0,2048,2048);glClear(GL_DEPTH_BUFFER_BIT);depthShader.use();depthShader.set("uLightVP",lightVP);depthShader.set("uInstanced",0);
 glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(2,4);
 for(auto&n:s.nodes){if(n.mesh<0||n.screen||n.character)continue;depthShader.set("uModel",n.world);s.meshes[n.mesh]->draw();}glDisable(GL_POLYGON_OFFSET_FILL);
 glm::vec3 securityEye=cameraIndex==0?glm::vec3(-3.5,3.35,-.5):glm::vec3(3.5,3.25,-11.4);
 glm::mat4 securityView=glm::lookAt(securityEye,glm::vec3(0,.8,-6),glm::vec3(0,1,0));
 glBindFramebuffer(GL_FRAMEBUFFER,cctv.fbo);glViewport(0,0,cctv.width,cctv.height);glClearColor(.025f,.035f,.05f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 draw(s,securityView,glm::perspective(glm::radians(72.f),16.f/9.f,.08f,35.f),securityEye,lights,alarm,true);
 sceneTarget.resize(width,height,true);glBindFramebuffer(GL_FRAMEBUFFER,sceneTarget.fbo);glViewport(0,0,width,height);glClearColor(.018f,.025f,.045f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 draw(s,view,glm::perspective(glm::radians(68.f),float(width)/height,.06f,50.f),eyePos,lights,alarm,false);
 glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,width,height);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);
 post.use();post.set("uScene",0);post.set("uTint",glm::vec3(1));glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,sceneTarget.color);glBindVertexArray(quad);glDrawArrays(GL_TRIANGLES,0,3);
}
}


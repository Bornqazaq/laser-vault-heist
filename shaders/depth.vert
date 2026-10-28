#version 330 core
layout(location=0)in vec3 aPosition;
layout(location=5)in mat4 aInstance;
uniform mat4 uModel,uLightVP;uniform bool uInstanced;
void main(){gl_Position=uLightVP*(uInstanced?aInstance:uModel)*vec4(aPosition,1);}


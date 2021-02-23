#version 450 core
layout (location = 0) in vec3 dir;
layout (location = 1) in vec2 pos;
layout (location = 2) in vec3[30] randomvector3;
out vec3 direction;
out vec3 origin;
out vec2 point;
out vec2 wnh;
out vec3[30] randomVector3;
uniform vec3 eye;
uniform vec2 screen;
void main()
{
	wnh=screen;
	randomVector3=randomvector3;
	direction=dir;
	origin=eye;
	point=pos;
    gl_Position = vec4(2*(pos[0]+0.5f)/screen[0]-1.0f,2*(pos[1]+0.5f)/screen[1]-1.0f,0.0f, 1.0);
}
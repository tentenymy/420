#version 150

in vec3 position;
in vec2 uv;

out vec2 tc;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;


void main()
{
	int mode = 1; // 0: Gradient; 1: texture
  	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  	tc = uv;
}


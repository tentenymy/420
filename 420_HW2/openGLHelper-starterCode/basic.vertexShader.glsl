#version 150

in vec3 position;
in vec4 color;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;



void main()
{
	int mode = 1; // 0: Gradient; 1: texture
  	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  	//col = color;
  	col = vec4(1.0, 0.0, 0.0, 1.0);
}


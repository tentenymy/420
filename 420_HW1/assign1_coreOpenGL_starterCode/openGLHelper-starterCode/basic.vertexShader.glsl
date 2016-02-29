#version 150

in vec3 position;
in vec4 color;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;



void main()
{
	int mode = 0; // 0: Gradient; 1: texture


  	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  	if (mode == 0) // Gradient
  		col = vec4(position[2] * 5.0f, position[2] * 5.0f, 0.0f, 1.0);
  	else  // Colorful image or texture image
  		col = color;
}


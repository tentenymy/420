#version 150

in vec2 tc;

out vec4 c;

uniform sampler2D myTextureSampler;


void main()
{
  c = texture(myTextureSampler, tc);
}


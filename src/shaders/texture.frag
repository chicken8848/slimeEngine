#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixture;

void main() {
  FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mixture);
}

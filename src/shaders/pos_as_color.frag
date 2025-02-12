#version 330 core
out vec4 FragColor;

in vec3 vertexColor;
in vec3 vector_position;

void main() {
  FragColor = vec4(vector_position, 1.0);
}

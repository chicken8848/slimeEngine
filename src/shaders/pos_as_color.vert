#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

out vec3 vertexColor;
out vec3 vector_position;

void main() {
  gl_Position = vec4(-1 * aPos, 1.0f);
  vertexColor = aCol;
  vector_position = aPos;
}

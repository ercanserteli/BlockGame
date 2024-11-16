// Vertex shader
#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 color;
uniform vec3 skyColor;
uniform float starVisibility;

void main() {
  vec4 viewPos = view * model * vec4(aPos, 1.0f);
  gl_Position = projection * viewPos;

  float culling_d = 3000.f;
  float gray = (culling_d + viewPos.z) / culling_d;
  vertexColor = mix(skyColor, mix(skyColor, color, starVisibility), max(0.0, min(gray, 1.0)));
}
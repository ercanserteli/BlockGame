// Vertex shader
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in float aAOFactor;

const int NUM_CASCADES = 3;

out vec3 vertexColor;
out vec3 normal;
out vec3 fragPos;
out float aoFactor;
out float mist;
out vec4 fragPosSunSpace[NUM_CASCADES];
out float clipSpacePosZ;

uniform float cullingDistance;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 sunSpaceMatrix[NUM_CASCADES];

const float e = 2.71828;

void main() {
  vec4 modelPos = model * vec4(aPos, 1.0f);
  vec4 viewPos = view * modelPos;

  gl_Position = projection * viewPos;

  float grayness = 1 - min(1.0f, max(0.000001f, (cullingDistance + viewPos.z) /
                                                    cullingDistance));
  float exp_grayness = pow(e, (1 - 1 / (grayness * grayness)));
  mist = exp_grayness;

  clipSpacePosZ = -viewPos.z;
  vertexColor = aColor;
  fragPos = vec3(modelPos);
  for (int i = 0; i < NUM_CASCADES; i++) {
    fragPosSunSpace[i] = sunSpaceMatrix[i] * modelPos;
  }
  normal = normalize((mat3(model) * aNormal));
  aoFactor = aAOFactor;
}

#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform int cascadeLevel;

void main() {
  float depthValue = texture(depthMap, TexCoords).r;

  // Visualize different cascades with different colors
  vec3 cascadeColors[3] =
      vec3[](vec3(1.0, 0.0, 0.0), // Red for near cascade
             vec3(0.0, 1.0, 0.0), // Green for middle cascade
             vec3(0.0, 0.0, 1.0)  // Blue for far cascade
      );

  // Enhance the visualization by using a non-linear mapping
  float enhancedDepth = pow(depthValue, 10.0);
  FragColor = vec4(cascadeColors[cascadeLevel] * enhancedDepth, 1.0);
}

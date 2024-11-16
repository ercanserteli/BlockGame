#pragma once

const char * frag_source =
"// Fragment shader\n"
"#version 420 core\n"
"out vec4 FragColor;\n"
"\n"
"const int NUM_CASCADES = 3;\n"
"\n"
"in vec3 vertexColor;\n"
"in vec3 normal;\n"
"in vec3 fragPos;\n"
"in float aoFactor;\n"
"in float mist;\n"
"in vec4 fragPosSunSpace[NUM_CASCADES];\n"
"in float clipSpacePosZ;\n"
"\n"
"uniform sampler2D shadowMap[NUM_CASCADES];\n"
"\n"
"uniform float ambientBase;\n"
"uniform vec3 sunColor;\n"
"uniform vec3 sunPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 objectColor;\n"
"uniform vec3 skyColor;\n"
"uniform float diffuseStrength;\n"
"uniform float specularStrength;\n"
"uniform float cascadeEnds[NUM_CASCADES];\n"
"\n"
"float ShadowCalculation(int CascadeIndex, vec4 fragPosSunSpace) {\n"
"  vec3 projCoords = fragPosSunSpace.xyz;\n"
"  projCoords = projCoords * 0.5 + 0.5; // converting -1,1 -> 0,1\n"
"\n"
"  float bias = max(0.001 * (1.0 - dot(normal, normalize(sunPos))), 0.0001);\n"
"  float closestDepth = texture(shadowMap[CascadeIndex], projCoords.xy).r;\n"
"  float currentDepth = projCoords.z;\n"
"  float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;\n"
"\n"
"  // Handle edge cases\n"
"  if (projCoords.z > 1.0) {\n"
"    shadow = 0.0;\n"
"  }\n"
"\n"
"  return shadow;\n"
"}\n"
"\n"
"void main() {\n"
"  float ambientStrength = ambientBase + 0.1f * aoFactor;\n"
"  vec3 ambient = ambientStrength * sunColor;\n"
"\n"
"  vec3 lightDir = normalize(sunPos);\n"
"  vec3 viewDir = normalize(viewPos - fragPos);\n"
"  vec3 halfwayDir = normalize(lightDir + viewDir);\n"
"\n"
"  float shadow = 0.0;\n"
"\n"
"  //  for debug\n"
"  const vec3 colors[] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};\n"
"  vec3 realColor = vertexColor;\n"
"  //\\ for debug\n"
"\n"
"  for (int i = 0; i < NUM_CASCADES; i++) {\n"
"    if (clipSpacePosZ <= cascadeEnds[i]) {\n"
"      shadow = ShadowCalculation(i, fragPosSunSpace[i]);\n"
"      // realColor = colors[i];  // uncomment for debug colors\n"
"      break;\n"
"    }\n"
"  }\n"
"\n"
"  float diff = max(dot(normal, lightDir), 0.0);\n"
"  vec3 diffuse = diffuseStrength * diff * sunColor * (1.0 - shadow);\n"
"\n"
"  float shininess = 128;\n"
"  float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);\n"
"  vec3 specular = specularStrength * spec * sunColor * (1.0 - shadow);\n"
"\n"
"  vec3 result = mix((ambient + diffuse + specular) * (realColor + objectColor),\n"
"                    skyColor, mist);\n"
"\n"
"  FragColor = vec4(result, 1.0);\n"
"}\n"
;

// Fragment shader
#version 420 core
out vec4 FragColor;

const int NUM_CASCADES = 3;

in vec3 vertexColor;
in vec3 normal;
in vec3 fragPos;
in float aoFactor;
in float mist;
in vec4 fragPosSunSpace[NUM_CASCADES];
in float clipSpacePosZ;

uniform sampler2D shadowMap[NUM_CASCADES];

uniform float ambientBase;
uniform vec3 sunColor;
uniform vec3 sunPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform vec3 skyColor;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float cascadeEnds[NUM_CASCADES];

float ShadowCalculation(int CascadeIndex, vec4 fragPosSunSpace) {
  vec3 projCoords = fragPosSunSpace.xyz;
  projCoords = projCoords * 0.5 + 0.5; // converting -1,1 -> 0,1

  float bias = max(0.00001 * (1.0 - dot(normal, normalize(sunPos))), 0.000001);
  float closestDepth = texture(shadowMap[CascadeIndex], projCoords.xy).r;
  float currentDepth = projCoords.z;
  float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

  // Handle edge cases
  if (projCoords.z > 1.0) {
    shadow = 0.0;
  }

  return shadow;
}

void main() {
  float ambientStrength = ambientBase + 0.05f * aoFactor;
  vec3 ambient = ambientStrength * sunColor;

  vec3 lightDir = normalize(sunPos);
  vec3 viewDir = normalize(viewPos - fragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);

  float shadow = 0.0;


  // const vec3 shadowMapColors[] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};  //  uncomment for debug colors

  vec3 realColor = vertexColor;

  for (int i = 0; i < NUM_CASCADES; i++) {
    if (clipSpacePosZ <= cascadeEnds[i]) {
      shadow = ShadowCalculation(i, fragPosSunSpace[i]);
      // realColor = shadowMapColors[i];  // uncomment for debug colors
      break;
    }
  }

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diffuseStrength * diff * sunColor * (1.0 - shadow);

  float shininess = 128;
  float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
  vec3 specular = specularStrength * spec * sunColor * (1.0 - shadow);

  vec3 result = mix((ambient + diffuse + specular) * (realColor + objectColor),
                    skyColor, mist);

  FragColor = vec4(result, 1.0);
}

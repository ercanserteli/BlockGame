#pragma once

const char * vertex_simple_depth_source =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"\n"
"uniform mat4 sunSpaceMatrix;\n"
"uniform mat4 model;\n"
"\n"
"void main() { gl_Position = sunSpaceMatrix * model * vec4(aPos, 1.0); }\n"
;

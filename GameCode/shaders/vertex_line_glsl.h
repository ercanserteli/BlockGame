#pragma once

const char * vertex_line_source =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"\n"
"out vec3 vertexColor;\n"
"\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform vec3 color;\n"
"\n"
"void main() {\n"
"  gl_Position = projection * view * vec4(aPos, 1.0);\n"
"  vertexColor = color;\n"
"}\n"
;

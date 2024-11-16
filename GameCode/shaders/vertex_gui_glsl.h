#pragma once

const char * vertex_gui_source =
"// Vertex shader\n"
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"\n"
"out vec3 vertexColor;\n"
"\n"
"uniform mat4 projection;\n"
"\n"
"void main() {\n"
"  gl_Position = projection * vec4(aPos, 1.0);\n"
"  vertexColor = aColor;\n"
"}\n"
;

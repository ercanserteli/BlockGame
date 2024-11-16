#pragma once

const char * frag_gui_source =
"// Fragment shader\n"
"#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"in vec3 vertexColor;\n"
"\n"
"void main() { FragColor = vec4(vertexColor, 1.0); }\n"
;

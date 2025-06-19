#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord;
out float EyeDist;

void main(){
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 viewPos = view * worldPos;

    EyeDist = -viewPos.z; // Расстояние от камеры до вершины
    TexCoord = aTexCoord;
    gl_Position = projection * viewPos;
}

#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in mat4 WorldMat;

uniform mat4 VP;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
    FragPos = vec3(WorldMat * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(WorldMat))) * aNormal; 
    TexCoords = aTexCoord;

    gl_Position = VP * WorldMat * vec4(aPos, 1.0);
}

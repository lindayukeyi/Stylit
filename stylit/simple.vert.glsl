#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 vs_uv;

uniform float horizonOffset;

out vec3 ourColor;
out vec2 fs_uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 pos = vec3(aPos.x, aPos.y, aPos.z);
    gl_Position = vec4(aPos, 1.0f);//projection * view * model * vec4(pos, 1.0f);
    ourColor = pos;
    fs_uv = vs_uv;
}
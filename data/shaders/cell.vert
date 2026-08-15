#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aInstancePos;
layout (location = 2) in float aInstanceRad;
layout (location = 3) in float aType;
layout (location = 4) in vec4 aColor;

uniform mat4 u_ViewProj;

out float v_Type;
out vec4 v_Color;

out vec2 v_LocalPos;

void main() {
    v_Type = aType;
    v_Color = aColor;

    v_LocalPos = aPos;
    vec2 worldPos = aInstancePos + aPos * aInstanceRad;
    gl_Position = u_ViewProj * vec4(worldPos, 0.0, 1.0);
}
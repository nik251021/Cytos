#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 u_resolution;

void main() {
    vec2 uv = TexCoords;
    vec2 center = uv - 0.5;
    
    float aspect = u_resolution.x / u_resolution.y;
    center.x *= aspect;
    
    float dist = length(center);

    float fovRadius = 0.48; 
    if (dist > fovRadius) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float aberrationStrength = 0.012 * (dist / fovRadius); 
    
    float r = texture(screenTexture, uv + center * aberrationStrength).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - center * aberrationStrength).b;
    
    vec3 col = vec3(r, g, b);

    float vignette = smoothstep(fovRadius, fovRadius - 0.18, dist);
    col *= vignette;

    float centerGlow = exp(-dist * 5.0) * 0.12;
    col += centerGlow;

    float noise = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453) * 1.0;
    col -= noise * 0.01;

    col = pow(col, vec3(1.05)); 

    FragColor = vec4(col, 1.0);
}
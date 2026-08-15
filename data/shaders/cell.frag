#version 330 core
in vec2 v_LocalPos;
in float v_Type;
in vec4 v_Color;
out vec4 FragColor;

vec2 rotate(vec2 uv, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return mat2(c, -s, s, c) * uv;
}

void main() {
    int type = int(v_Type + 0.5);
    float dist = length(v_LocalPos);
    float d2 = max(abs(v_LocalPos.x), abs(v_LocalPos.y));
    
    float alpha = 1.0;
    float border = 0.0;
    vec3 shapeColor = v_Color.rgb;
    
    switch(type) {
        case -1:
            float edge = max(abs(v_LocalPos.x), abs(v_LocalPos.y));
            if (edge < 0.95) discard;
            alpha = 1.0;
            shapeColor = v_Color.rgb;
            break;

        case 0:
            if (dist > 1.0) discard;
            alpha = 1.0 - smoothstep(0.95, 1.0, dist);
            border = smoothstep(0.85, 0.95, dist);
            break;

        case 1:
            float bodyDist = length(v_LocalPos - vec2(0.0, 0.2));
            bool isTail = (abs(v_LocalPos.x) < 0.15 && v_LocalPos.y < 0.2 && v_LocalPos.y > -0.8);
            
            if (bodyDist > 0.8 && !isTail) discard;
            
            float finalDist = isTail ? 0.0 : bodyDist;
            alpha = 1.0 - smoothstep(0.75, 0.85, finalDist);
            
            border = smoothstep(0.65, 0.8, finalDist);
            break;

        case 2:
            if (dist > 1.0) discard;
            float spots = sin(v_LocalPos.x * 10.0) * sin(v_LocalPos.y * 10.0);
            shapeColor = mix(v_Color.rgb, vec3(0.0, 1.0, 0.0), smoothstep(0.5, 0.6, spots));
            break;

        case 3:
            float angle = atan(v_LocalPos.y, v_LocalPos.x);
            float spikes = sin(angle * 8.0) * 0.2 + 1.0;
            if (dist > spikes) discard;
            alpha = 1.0 - smoothstep(spikes - 0.05, spikes, dist);
            break;

        case 4:
            if (dist > 1.0) discard;
            border = smoothstep(0.6, 0.95, dist);
            break;

        case 5:
            float d = cos(atan(v_LocalPos.y, v_LocalPos.x) * 3.0);
            if (dist > 0.8 + 0.2 * d) discard;
            break;

        case 6:
            if (dist > 1.0) discard;
            shapeColor += vec3(0.5, 0.0, 0.5);
            break;
    }

    float shade = 1.0 - smoothstep(0.3, 1.0, dist);
    vec3 finalColor = shapeColor * (0.8 + 0.2 * shade);
    finalColor = mix(finalColor, vec3(0.0), border * 0.5);
    
    FragColor = vec4(finalColor, alpha);
}
#version 330 core
in vec2 v_LocalPos;
in float v_Type;
in vec4 v_Color;
out vec4 FragColor;

void main() {
    int type = int(v_Type + 0.5);
    float dist = length(v_LocalPos);
    
    if (dist > 1.0) discard;
    
    float alpha = 1.0 - smoothstep(0.92, 1.0, dist);
    
    vec3 shapeColor = v_Color.rgb;
    float border = 0.0;
    float innerGlow = 0.0;

    switch(type) {
        case -1: // Dead cell
            if (dist < 0.85) discard;
            alpha = smoothstep(0.85, 0.9, dist);
            shapeColor = v_Color.rgb * 0.5;
            break;

        case 0: // Standart
            border = smoothstep(0.75, 0.95, dist);
            innerGlow = 1.0 - smoothstep(0.0, 0.8, dist);
            break;

        case 1: // Flagellocyte
            {
                bool isTail = (abs(v_LocalPos.x) < 0.12 && v_LocalPos.y < 0.2 && v_LocalPos.y > -0.85);
                float finalDist = isTail ? abs(v_LocalPos.x) * 2.0 : dist;
                if (finalDist > 0.9 && !isTail) discard;
                
                border = smoothstep(0.7, 0.9, dist);
                shapeColor = mix(v_Color.rgb, vec3(0.1, 0.11, 0.18), 0.4);
            }
            break;

        case 2: //Photocyte
            border = smoothstep(0.7, 0.95, dist);
            float spots = sin(v_LocalPos.x * 10.0) * cos(v_LocalPos.y * 10.0);
            float spotMask = smoothstep(0.2, 0.8, spots);
            shapeColor = mix(v_Color.rgb, v_Color.rgb * 1.4 + vec3(0.0, 0.15, 0.0), spotMask * 0.3);
            break;

        case 3: // Devorocite
            {
                float angle = atan(v_LocalPos.y, v_LocalPos.x);
                float spikes = sin(angle * 8.0) * 0.15 + 0.85;
                if (dist > spikes) discard;
                alpha = 1.0 - smoothstep(spikes - 0.08, spikes, dist);
                border = smoothstep(spikes - 0.3, spikes, dist);
            }
            break;

        default:
            border = smoothstep(0.7, 0.95, dist);
            break;
    }

    float volumeShade = 1.0 - smoothstep(0.0, 0.9, dist);
    vec3 finalColor = shapeColor * (0.6 + 0.4 * volumeShade);
    
    finalColor += shapeColor * innerGlow * 0.25;
    finalColor = mix(finalColor, vec3(0.0, 0.0, 0.0), border * 0.6);

    float rim = smoothstep(0.8, 1.0, dist);
    finalColor += v_Color.rgb * rim * 0.3;

    FragColor = vec4(finalColor, alpha);
}
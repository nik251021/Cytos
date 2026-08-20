#version 330 core
in vec2 v_LocalPos;
in float v_Type;
in vec4 v_Color;
out vec4 FragColor;

void main() {
    int type = int(v_Type + 0.5);
    float dist = length(v_LocalPos);
    
    if (dist > 1.0) discard;
    
    float alpha = 1.0 - smoothstep(0.90, 1.0, dist);
    
    vec3 shapeColor = v_Color.rgb;
    float border = 0.0;
    float innerGlow = 0.0;

    switch(type) {
        case 99: // dead cell
            {
                if (dist > 0.92) discard;
                alpha = (1.0 - smoothstep(0.3, 0.85, dist)) * 0.35;
                shapeColor = vec3(0.25, 0.24, 0.22);
                border = smoothstep(0.5, 0.9, dist) * 0.3;
            }
            break;

        case 0: // Standard
            border = smoothstep(0.70, 0.96, dist);
            innerGlow = 1.0 - smoothstep(0.1, 0.85, dist);
            alpha *= 0.85; 
            break;

        case 1: // Flagellocyte
            {
                bool isTail = (abs(v_LocalPos.x) < 0.12 && v_LocalPos.y < 0.2 && v_LocalPos.y > -0.85);
                float finalDist = isTail ? abs(v_LocalPos.x) * 2.0 : dist;
                if (finalDist > 0.9 && !isTail) discard;
                
                border = smoothstep(0.65, 0.9, dist);
                shapeColor = mix(v_Color.rgb, vec3(0.08, 0.12, 0.18), 0.5);
                alpha *= 0.9;
            }
            break;

        case 2: // Photocyte
            border = smoothstep(0.7, 0.95, dist);
            float spots = sin(v_LocalPos.x * 12.0) * cos(v_LocalPos.y * 12.0) + sin(v_LocalPos.x * 6.0 + v_LocalPos.y * 6.0) * 0.5;
            float spotMask = smoothstep(-0.2, 0.6, spots);
            shapeColor = mix(v_Color.rgb, v_Color.rgb * 1.5 + vec3(0.05, 0.25, 0.05), spotMask * 0.45);
            innerGlow = 0.6;
            alpha *= 0.92;
            break;

        case 3: // Devorocite
            {
                float angle = atan(v_LocalPos.y, v_LocalPos.x);
                float teeth = abs(sin(angle * 6.0));
                float outerShape = 0.8 + teeth * 0.2;
                
                if (dist > outerShape) discard;
                
                alpha = 1.0 - smoothstep(outerShape - 0.06, outerShape, dist);
                border = smoothstep(outerShape - 0.3, outerShape, dist);
                
                shapeColor = mix(v_Color.rgb, vec3(0.3, 0.05, 0.1), 0.4);
                innerGlow = 0.4;
            }
            break;

        case 4: // Keratinocyte
            {
                float plates = abs(sin(v_LocalPos.x * 10.0) * cos(v_LocalPos.y * 10.0));
                float plateMask = smoothstep(0.2, 0.8, plates);
                
                border = smoothstep(0.55, 0.98, dist);
                
                shapeColor = mix(v_Color.rgb, vec3(0.4, 0.4, 0.45), plateMask * 0.4);
                innerGlow = 0.2;
                alpha = 0.98;
            }
            break;

        case 5: // Neurocyte (Нейроцит)
            {
                border = smoothstep(0.70, 0.96, dist);
                
                float angle = atan(v_LocalPos.y, v_LocalPos.x);
                float rays = sin(angle * 5.0) * 0.5 + 0.5;
                
                float ringWave = abs(sin(dist * 12.0)); 
                
                shapeColor = mix(v_Color.rgb, vec3(0.05, 0.25, 0.35), 0.6);
                
                innerGlow = (1.0 - dist) * 0.7 + (rays * ringWave) * 0.3;
                
                alpha *= 0.90;
            }
            break;

        case 6: // Axonocyte (Аксоноцит - проводящий жилок)
            {
                border = smoothstep(0.70, 0.96, dist);
                float fiber = abs(sin(v_LocalPos.x * 18.0));
                shapeColor = mix(v_Color.rgb, vec3(0.1, 0.3, 0.2), 0.5);
                innerGlow = fiber * 0.5;
                alpha *= 0.88;
            }
            break;

        case 7: // Sensorycyte (Универсальный сенсор / Рецептор)
            {
                border = smoothstep(0.65, 0.95, dist);
                float radarRings = abs(sin(dist * 16.0));
                float coreLens = 1.0 - smoothstep(0.0, 0.4, dist);
                
                shapeColor = mix(v_Color.rgb, vec3(0.35, 0.3, 0.05), 0.5);
                innerGlow = coreLens * 0.8 + radarRings * 0.3;
                
                alpha *= 0.92;
            }
            break;

        default:
            border = smoothstep(0.7, 0.95, dist);
            break;
    }

    vec2 lightDir = normalize(vec2(-0.4, -0.6));
    float hemi = dot(normalize(v_LocalPos + vec2(0.0001)), lightDir);
    float volumeShade = 1.0 - smoothstep(0.0, 0.95, dist);
    
    vec3 finalColor = shapeColor * (0.6 + 0.4 * volumeShade);
    
    finalColor += shapeColor * innerGlow * 0.3;
    finalColor = mix(finalColor, vec3(0.02, 0.03, 0.05), border * 0.75);

    float rim = smoothstep(0.75, 0.98, dist);
    finalColor += v_Color.rgb * rim * 0.4;

    FragColor = vec4(finalColor, alpha);
}
#version 330 core
in vec2 v_LocalPos;
in float v_Type;
in vec4 v_Color;
out vec4 FragColor;

void main() {
    int type = int(v_Type + 0.5);
    float dist = length(v_LocalPos);
    
    if (dist > 1.0) discard;
    
    // Плавное сглаживание краев клетки под микроскопом
    float alpha = 1.0 - smoothstep(0.90, 1.0, dist);
    
    vec3 shapeColor = v_Color.rgb;
    float border = 0.0;
    float innerGlow = 0.0;

    switch(type) {
        case 99: // dead cell (остатки разрушенной клетки, детрит)
            {
                if (dist > 0.92) discard;
                alpha = (1.0 - smoothstep(0.3, 0.85, dist)) * 0.35;
                shapeColor = vec3(0.25, 0.24, 0.22);
                border = smoothstep(0.5, 0.9, dist) * 0.3;
            }
            break;

        case 0: // Standard (обычная живая клетка - полупрозрачное стекло/цитоплазма)
            border = smoothstep(0.70, 0.96, dist);
            innerGlow = 1.0 - smoothstep(0.1, 0.85, dist);
            // Чуть снижаем непрозрачность стандартных клеток для эффекта "жидкой среды"
            alpha *= 0.85; 
            break;

        case 1: // Flagellocyte (жгутиковая клетка)
            {
                bool isTail = (abs(v_LocalPos.x) < 0.12 && v_LocalPos.y < 0.2 && v_LocalPos.y > -0.85);
                float finalDist = isTail ? abs(v_LocalPos.x) * 2.0 : dist;
                if (finalDist > 0.9 && !isTail) discard;
                
                border = smoothstep(0.65, 0.9, dist);
                shapeColor = mix(v_Color.rgb, vec3(0.08, 0.12, 0.18), 0.5);
                alpha *= 0.9;
            }
            break;

        case 2: // Photocyte (фотоцит с хлоропластами/зернистостью)
            border = smoothstep(0.7, 0.95, dist);
            // Органический процедурный узор внутренних органелл (хлоропластов)
            float spots = sin(v_LocalPos.x * 12.0) * cos(v_LocalPos.y * 12.0) + sin(v_LocalPos.x * 6.0 + v_LocalPos.y * 6.0) * 0.5;
            float spotMask = smoothstep(-0.2, 0.6, spots);
            shapeColor = mix(v_Color.rgb, v_Color.rgb * 1.5 + vec3(0.05, 0.25, 0.05), spotMask * 0.45);
            innerGlow = 0.6;
            alpha *= 0.92;
            break;

        case 3: // Devorocite (хищник с хитиновыми шипами)
            {
                float angle = atan(v_LocalPos.y, v_LocalPos.x);
                float spikes = sin(angle * 8.0) * 0.15 + 0.83;
                if (dist > spikes) discard;
                alpha = 1.0 - smoothstep(spikes - 0.08, spikes, dist);
                border = smoothstep(spikes - 0.35, spikes, dist);
                // Более агрессивный, плотный оттенок хищника
                shapeColor = mix(v_Color.rgb, vec3(0.2, 0.05, 0.05), 0.3);
            }
            break;

        default:
            border = smoothstep(0.7, 0.95, dist);
            break;
    }

    // Эффект сферического объема липидной капли (светотень)
    // Свет падает немного сверху-слева (смещаем центр объема)
    vec2 lightDir = normalize(vec2(-0.4, -0.6));
    float hemi = dot(normalize(v_LocalPos + vec2(0.0001)), lightDir); // защита от деления на ноль
    float volumeShade = 1.0 - smoothstep(0.0, 0.95, dist);
    
    // Базовый цвет с учетом объема
    vec3 finalColor = shapeColor * (0.5 + 0.5 * volumeShade);
    
    // Добавляем внутреннее свечение мембраны/органелл
    finalColor += shapeColor * innerGlow * 0.3;
    
    // Плотная оптическая граница мембраны (клеточная стенка)
    finalColor = mix(finalColor, vec3(0.02, 0.03, 0.05), border * 0.75);

    // Яркий преломленный блик по краю (краевой эффект светопреломления в микроскопе - Rim light)
    float rim = smoothstep(0.75, 0.98, dist);
    finalColor += v_Color.rgb * rim * 0.4;

    FragColor = vec4(finalColor, alpha);
}
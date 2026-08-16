#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 u_resolution; // Используем для расчета физически корректного круга

void main() {
    // 1. Подготовка координат
    vec2 uv = TexCoords;
    // Центрируем координаты от -0.5 до 0.5
    vec2 center = uv - 0.5;
    
    // Коррекция аспекта (важно, чтобы круг был круглым, если окно не 1:1)
    float aspect = u_resolution.x / u_resolution.y;
    center.x *= aspect;
    
    // Расстояние от центра (радиус)
    float dist = length(center);

    // 2. Имитация физической диафрагмы/тубуса (идеально круглый черный край)
    // Радиус настраивается (здесь 0.48 от меньшей стороны)
    float fovRadius = 0.48; 
    if (dist > fovRadius) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Всё, что за линзой - абсолютно черное
        return;
    }

    // 3. Хроматическая аберрация (оптическое искажение по краям)
    // Сдвигаем R и B каналы от центра, G оставляем на месте.
    // Сильнее всего проявляется ближе к краю (fovRadius).
    float aberrationStrength = 0.012 * (dist / fovRadius); 
    
    float r = texture(screenTexture, uv + center * aberrationStrength).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - center * aberrationStrength).b;
    
    vec3 col = vec3(r, g, b);

    // 4. Виньетирование (реальное падение освещенности к краям линзы)
    // Используем smoothstep для плавного, но сильного затемнения от 0.3 до 0.48
    float vignette = smoothstep(fovRadius, fovRadius - 0.18, dist);
    col *= vignette;

    // 5. Имитация Substage Illumination (яркий центр, характерный для микроскопов)
    // Добавляем мягкое свечение в центр, имитирующее конденсор
    float centerGlow = exp(-dist * 5.0) * 0.12;
    col += centerGlow;

    // 6. Тонкая оптическая зернистость (Film Grain / Sensor Noise)
    // Добавляет "жизни" цифровому или аналоговому изображению
    float noise = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453) * 1.0;
    col -= noise * 0.01; // Очень тонкий шум

    // 7. Финальная гамма-коррекция для "научного" вида
    col = pow(col, vec3(1.05)); 

    FragColor = vec4(col, 1.0);
}
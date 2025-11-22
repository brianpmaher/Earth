#version 410 core

in vec2 v_UV;
out vec4 FragColor;

void main()
{
    // Debug grid pattern
    float gridX = step(0.98, fract(v_UV.x * 32.0));
    float gridY = step(0.98, fract(v_UV.y * 32.0));
    float grid = max(gridX, gridY);
    
    vec3 color = mix(vec3(0.2, 0.3, 0.8), vec3(1.0), grid);
    FragColor = vec4(color, 1.0);
}

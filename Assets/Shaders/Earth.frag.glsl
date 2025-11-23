#version 410 core

in vec2 v_UV;
in vec2 v_GlobalUV;
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform bool u_ShowGrid;

const float PI = 3.14159265359;

void main()
{
    vec4 texColor = texture(u_Texture, v_UV);

    if (u_ShowGrid)
    {
        // Calculate Longitude and Latitude using Global UV
        float longitude = -((v_GlobalUV.x * 2.0 * PI) - PI);
        
        float mercatorY = PI * (1.0 - 2.0 * v_GlobalUV.y);
        float latitude = 2.0 * atan(exp(mercatorY)) - (PI / 2.0);
        
        // Convert to degrees
        float lonDeg = degrees(longitude);
        float latDeg = degrees(latitude);
        
        // Grid lines every 10 degrees
        float gridSpacing = 10.0;
        
        // General Grid
        float distLon = abs(mod(lonDeg + gridSpacing/2.0, gridSpacing) - gridSpacing/2.0);
        float distLat = abs(mod(latDeg + gridSpacing/2.0, gridSpacing) - gridSpacing/2.0);

        float gridLon = 1.0 - smoothstep(0.0, fwidth(lonDeg), distLon);
        float gridLat = 1.0 - smoothstep(0.0, fwidth(latDeg), distLat);
        
        // Special Lines
        float distEquator = abs(latDeg);
        float distPrime = abs(lonDeg);
        float distDate = 180.0 - abs(lonDeg);

        float gridEquator = 1.0 - smoothstep(0.0, fwidth(latDeg), distEquator);
        float gridPrime = 1.0 - smoothstep(0.0, fwidth(lonDeg), distPrime);
        float gridDate = 1.0 - smoothstep(0.0, fwidth(lonDeg), distDate);

        // Compose Color
        vec3 gridColor = vec3(1.0); // White
        float gridAlpha = max(gridLon, gridLat);

        // Green for Prime Meridian and Date Line
        float greenMix = max(gridPrime, gridDate);
        gridColor = mix(gridColor, vec3(0.0, 1.0, 0.0), greenMix);
        gridAlpha = max(gridAlpha, greenMix);

        // Red for Equator
        float redMix = gridEquator;
        gridColor = mix(gridColor, vec3(1.0, 0.0, 0.0), redMix);
        gridAlpha = max(gridAlpha, redMix);
        
        FragColor = mix(texColor, vec4(gridColor, 1.0), gridAlpha * 0.8);
    }
    else
    {
        FragColor = texColor;
    }
}

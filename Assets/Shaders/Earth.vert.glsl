#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

uniform mat4 u_ViewProjection;
uniform int u_TileX;
uniform int u_TileY;
uniform int u_TileZ;
uniform sampler2D u_ElevationTexture;

out vec2 v_UV;
out vec2 v_GlobalUV;
out float v_Elevation;

const float PI = 3.14159265359;
const float EARTH_RADIUS = 6371000.0;

void main()
{
    v_UV = a_UV;

    float scale = 1.0 / pow(2.0, float(u_TileZ));
    v_GlobalUV = (a_UV + vec2(float(u_TileX), float(u_TileY))) * scale;

    // Sample elevation
    vec4 color = texture(u_ElevationTexture, v_UV);
    float r = round(color.r * 255.0);
    float g = round(color.g * 255.0);
    float b = round(color.b * 255.0);
    
    float elevation = -10000.0 + ((r * 65536.0 + g * 256.0 + b) * 0.1);
    v_Elevation = elevation;

    // Mercator Projection to Sphere Position
    
    // Map U to Longitude [-PI, PI]
    // Invert longitude to match texture direction
    float longitude = -((v_GlobalUV.x * 2.0 * PI) - PI);

    // Map V to Latitude
    float mercatorY = PI * (1.0 - 2.0 * v_GlobalUV.y);
    float latitude = 2.0 * atan(exp(mercatorY)) - (PI / 2.0);

    // Spherical to Cartesian
    float cosLat = cos(latitude);
    float radius = 1.0 + (elevation / EARTH_RADIUS);

    float x = radius * cosLat * cos(longitude);
    float y = radius * sin(latitude);
    float z = radius * cosLat * sin(longitude);

    gl_Position = u_ViewProjection * vec4(x, y, z, 1.0);
}

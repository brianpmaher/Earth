#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

uniform mat4 u_ViewProjection;
uniform int u_TileX;
uniform int u_TileY;
uniform int u_TileZ;

out vec2 v_UV;
out vec2 v_GlobalUV;

const float PI = 3.14159265359;

void main()
{
    v_UV = a_UV;

    float scale = 1.0 / pow(2.0, float(u_TileZ));
    v_GlobalUV = (a_UV + vec2(float(u_TileX), float(u_TileY))) * scale;

    // Mercator Projection to Sphere Position
    
    // Map U to Longitude [-PI, PI]
    // Invert longitude to match texture direction
    float longitude = -((v_GlobalUV.x * 2.0 * PI) - PI);

    // Map V to Latitude
    float mercatorY = PI * (1.0 - 2.0 * v_GlobalUV.y);
    float latitude = 2.0 * atan(exp(mercatorY)) - (PI / 2.0);

    // Spherical to Cartesian
    float cosLat = cos(latitude);
    float radius = 1.0;

    float x = radius * cosLat * cos(longitude);
    float y = radius * sin(latitude);
    float z = radius * cosLat * sin(longitude);

    gl_Position = u_ViewProjection * vec4(x, y, z, 1.0);
}

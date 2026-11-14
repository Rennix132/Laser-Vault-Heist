#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 jumpLaser;
uniform vec3 crouchLaser;
uniform vec3 neonStrip;
uniform float laserHalfWidth;
uniform float neonNearZ;
uniform float neonFarZ;
uniform vec3 viewPos;
uniform vec3 viewDirection;
uniform bool flashlight;
uniform bool tiled;
uniform float emission;

vec3 illuminate(vec3 offset, vec3 color, vec3 normal, vec3 view, vec3 material, float falloff) {
    float distanceSquared = dot(offset, offset);
    vec3 light = offset * inversesqrt(max(distanceSquared, 0.0001));
    float diffuse = max(dot(normal, light), 0.0);
    vec3 halfway = normalize(light + view);
    float specular = diffuse > 0.0 ? pow(max(dot(normal, halfway), 0.0), 32.0) : 0.0;
    return color * (diffuse * material + 0.10 * specular) / (1.0 + falloff * distanceSquared);
}

vec3 laserLight(vec3 center, vec3 normal, vec3 view, vec3 material) {
    center.x = clamp(FragPos.x, -laserHalfWidth, laserHalfWidth);
    vec3 offset = center - FragPos;
    float edge = 1.0 - smoothstep(1.25, 3.6, length(offset));
    return edge * illuminate(offset, vec3(1.9, 0.075, 0.13), normal, view, material, 2.1);
}

void main() {
    vec3 material = objectColor;
    if (tiled) {
        vec2 grid = FragPos.xz * 0.8;
        vec2 cell = abs(fract(grid) - 0.5);
        vec2 pixel = max(fwidth(grid), vec2(0.001));
        vec2 seam = smoothstep(vec2(0.47) - pixel, vec2(0.47) + pixel, cell);
        float panel = mod(floor(grid.x) + floor(grid.y), 2.0);
        material *= mix(0.42, 0.85 + 0.15 * panel, 1.0 - max(seam.x, seam.y));
    }

    vec3 normal = normalize(Normal);
    vec3 view = normalize(viewPos - FragPos);
    vec3 color = material * vec3(0.006, 0.009, 0.015);
    color += laserLight(jumpLaser, normal, view, material);
    color += laserLight(crouchLaser, normal, view, material);

    float stripZ = clamp(FragPos.z, neonFarZ, neonNearZ);
    vec3 leftStrip = vec3(-neonStrip.x, neonStrip.y, stripZ);
    vec3 rightStrip = vec3(neonStrip.x, neonStrip.y, stripZ);
    color += illuminate(leftStrip - FragPos, vec3(0.045, 0.20, 0.31), normal, view, material, 0.51);
    color += illuminate(rightStrip - FragPos, vec3(0.045, 0.20, 0.31), normal, view, material, 0.51);
    if (flashlight) {
        float cone = smoothstep(0.83, 0.96, dot(-view, viewDirection));
        color += cone * illuminate(viewPos - FragPos, vec3(1.0, 0.92, 0.75), normal, view, material, 0.11);
    }
    FragColor = vec4(mix(pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2)), objectColor, emission), 1.0);
}

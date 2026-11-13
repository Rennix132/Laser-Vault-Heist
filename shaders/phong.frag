#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lampPos;
uniform vec3 movingLightPos;
uniform vec3 viewPos;
uniform vec3 viewDirection;
uniform bool flashlight;
uniform bool tiled;
uniform float emission;

vec3 illuminate(vec3 offset, vec3 color, vec3 normal, vec3 view, vec3 material) {
    float distanceSquared = dot(offset, offset);
    vec3 light = offset * inversesqrt(max(distanceSquared, 0.0001));
    float diffuse = max(dot(normal, light), 0.0);
    vec3 halfway = normalize(light + view);
    float specular = diffuse > 0.0 ? pow(max(dot(normal, halfway), 0.0), 32.0) : 0.0;
    return color * (diffuse * material + 0.24 * specular) / (1.0 + 0.045 * distanceSquared);
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
    vec3 color = 0.30 * material;
    color += illuminate(lampPos - FragPos, vec3(0.68, 0.88, 1.0), normal, view, material);
    color += illuminate(movingLightPos - FragPos, vec3(0.34, 0.82, 1.0), normal, view, material);
    if (flashlight) {
        float cone = smoothstep(0.83, 0.96, dot(-view, viewDirection));
        color += cone * illuminate(viewPos - FragPos, vec3(1.0, 0.92, 0.75), normal, view, material);
    }
    FragColor = vec4(mix(color, objectColor, emission), 1.0);
}

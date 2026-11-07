#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 UV;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main() {
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);

    vec3 ambient  = 0.15 * lightColor;
    vec3 diffuse  = max(dot(N, L), 0.0) * lightColor;
    vec3 specular = pow(max(dot(N, H), 0.0), 32.0) * 0.5 * lightColor;

    FragColor = vec4((ambient + diffuse + specular) * objectColor, 1.0);
}

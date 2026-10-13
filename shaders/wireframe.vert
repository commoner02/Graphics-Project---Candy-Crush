#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Shading mode: 0 = Flat/Basic, 1 = Gouraud, 2 = Phong, 3 = Normal Map
uniform int shadingMode;
uniform vec3 viewPos;
uniform vec4 objectColor;

out vec3 Normal;
out vec3 FragPos;
out vec3 vGouraudColor;
out vec3 LocalPos;

void main() {
    LocalPos = aPos;
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);

    // GOURAUD SHADING: Compute lighting per-vertex and interpolate color across faces
    if (shadingMode == 1) {
        vec3 lightPos1 = vec3(8.0, 16.0, 8.0);
        vec3 lightDir1 = normalize(lightPos1 - FragPos);
        vec3 lightDir2 = normalize(vec3(-0.6, 0.8, -0.4));
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir1 + viewDir);

        vec3 base = objectColor.rgb;
        float hFactor = clamp(aPos.y * 2.2, -1.0, 1.0);
        vec3 deepSyrup = base * vec3(0.78, 0.72, 0.76);
        vec3 sugarCrown = min(base * 1.16 + vec3(0.04, 0.04, 0.03), vec3(1.0));
        base = mix(deepSyrup, sugarCrown, hFactor * 0.5 + 0.5);

        vec3 ambient = 0.44 * base;
        float diff1 = max(dot(norm, lightDir1), 0.0);
        float diff2 = max(dot(norm, lightDir2), 0.0) * 0.30;
        vec3 diffuse = (diff1 + diff2) * 0.56 * base;
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * 0.85 * vec3(1.0);

        vGouraudColor = ambient + diffuse + specular;
    } else {
        vGouraudColor = objectColor.rgb;
    }
}

#version 330 core
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec4 objectColor;
uniform bool isWireframe;
uniform bool isTray;

void main() {
    if (isWireframe) {
        if (isTray) {
            // Muted slate blueprint wireframe for pyramid slabs
            FragColor = vec4(0.38, 0.42, 0.52, 1.0);
        } else {
            // Crisp high-contrast vibrant colored wireframe for shapes
            FragColor = objectColor;
        }
    } else {
        // Architectural solid mode with Blinn-Phong diffuse lighting
        vec3 lightDir = normalize(vec3(0.55, 1.0, 0.40));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);

        if (isTray) {
            vec3 trayBase = vec3(0.32, 0.36, 0.44);
            vec3 ambient = 0.40 * trayBase;
            vec3 diffuse = diff * 0.60 * trayBase;
            FragColor = vec4(ambient + diffuse, 1.0);
        } else {
            vec3 shapeBase = objectColor.rgb;
            vec3 ambient = 0.40 * shapeBase;
            vec3 diffuse = diff * 0.60 * shapeBase;
            FragColor = vec4(ambient + diffuse, objectColor.a);
        }
    }
}

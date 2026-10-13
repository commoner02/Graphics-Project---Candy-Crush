#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec3 vGouraudColor;

in vec3 LocalPos;

out vec4 FragColor;

uniform vec4 objectColor;
uniform bool isWireframe;
uniform bool isTray;
uniform bool isGolden;
uniform bool isBrushed;

// Shading mode: 0 = Flat/Basic, 1 = Gouraud, 2 = Phong, 3 = Normal Map
uniform int shadingMode;
uniform vec3 viewPos;

// 3D Hash function for fine confectioner's sugar crystal sparkles
float hash3(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453123);
}

void main() {
    // 1. Wireframe Outline Mode
    if (isWireframe) {
        if (isTray) {
            FragColor = vec4(0.28, 0.36, 0.48, 1.0); // Clean blueprint slate
        } else {
            FragColor = objectColor; // 100% full intensity vivid neon wireframe
        }
        return;
    }

    // 2. Normal Vector Visualization (Academic Normal Map Mode)
    if (shadingMode == 3) {
        vec3 n = normalize(Normal);
        FragColor = vec4(n * 0.5 + 0.5, 1.0);
        return;
    }

    // 3. Gouraud Shading Mode (Per-vertex interpolation)
    if (shadingMode == 1) {
        vec3 gColor = vGouraudColor;
        if (!isTray) {
            float lum = dot(gColor, vec3(0.299, 0.587, 0.114));
            gColor = mix(vec3(lum), gColor, 1.25);
        }
        FragColor = vec4(gColor, objectColor.a);
        return;
    }

    // Two-tone illumination: Warm Key Light + Soft Cool Fill Light
    vec3 lightPos1 = vec3(8.0, 16.0, 8.0);
    vec3 lightDir1 = normalize(lightPos1 - FragPos);
    vec3 lightDir2 = normalize(vec3(-0.6, 0.8, -0.4));
    vec3 norm = normalize(Normal);
    vec3 baseColor = objectColor.rgb;

    // -------------------------------------------------------------
    // CONFECTIONERY GRADIENT & SYRUP TONE (Non-uniform candy depth)
    // -------------------------------------------------------------
    if (!isTray) {
        // Vertical gradient along local shape height:
        // LocalPos.y ranges approx -0.35 to +0.35.
        // Base has a denser, deeper fruit syrup core.
        // Top has an illuminated, glistening sugar glaze crown.
        float hFactor = clamp(LocalPos.y * 2.2, -1.0, 1.0);
        vec3 deepSyrup = baseColor * vec3(0.78, 0.72, 0.76);
        vec3 sugarCrown = min(baseColor * 1.16 + vec3(0.04, 0.04, 0.03), vec3(1.0));
        baseColor = mix(deepSyrup, sugarCrown, hFactor * 0.5 + 0.5);

        // ARTISAN HAND-BRUSHED GLAZE STROKE (Bakery Paint Swipe)
        if (isBrushed) {
            float brushCoord = dot(LocalPos.xz, vec2(0.707, 0.707)) * 3.6 + LocalPos.y * 1.8;
            float brushProfile = sin(brushCoord * 3.14159);
            float brushSwipe = smoothstep(0.15, 0.85, brushProfile) * smoothstep(-0.25, 0.25, LocalPos.y);
            float bristle = 0.88 + 0.12 * sin(brushCoord * 28.0);
            vec3 glazeTint = min(baseColor * 1.32 + vec3(0.10, 0.08, 0.06), vec3(1.0));
            baseColor = mix(baseColor, glazeTint * bristle, brushSwipe * 0.38);
        }
    }

    float diff1 = max(dot(norm, lightDir1), 0.0);
    float diff2 = max(dot(norm, lightDir2), 0.0) * 0.30;
    float diff = diff1 + diff2;

    // 4. Flat / Basic Shading Mode
    if (shadingMode == 0) {
        vec3 ambient = (isTray ? 0.45 : (isGolden ? 0.48 : 0.42)) * baseColor;
        vec3 diffuse = diff * (isTray ? 0.55 : (isGolden ? 0.54 : 0.58)) * baseColor;
        vec3 finalCol = ambient + diffuse;
        if (!isTray) {
            float lum = dot(finalCol, vec3(0.299, 0.587, 0.114));
            finalCol = mix(vec3(lum), finalCol, 1.25);
        }
        FragColor = vec4(finalCol, objectColor.a);
        return;
    }

    // 5. Full Phong / Blinn-Phong Shading Mode (Confectionery Suite)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir1 + viewDir);

    if (isTray) {
        // Architectural Stepped Trays: Crisp, clean matte platform lighting
        vec3 ambient = 0.45 * baseColor;
        vec3 diffuse = diff * 0.55 * baseColor;
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 24.0);
        vec3 specular = spec * 0.30 * vec3(0.55, 0.60, 0.70);
        FragColor = vec4(ambient + diffuse + specular, 1.0);
        return;
    }

    // --- CANDY CONFECTIONERY SHADING (Fruit + 24K Gold) ---
    vec3 ambient = (isGolden ? 0.48 : 0.44) * baseColor;
    vec3 diffuse = diff * (isGolden ? 0.54 : 0.56) * baseColor;

    // A. Translucent Gummy Edge Glow (Fresnel Subsurface Fake)
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 2.8);
    vec3 rimGlow = baseColor * fresnel * (isGolden ? 0.75 : 0.55);

    // B. Dual-Lobe Confectionery Specular
    float sharpShininess = isGolden ? 80.0 : 96.0;
    float sharpSpec = pow(max(dot(norm, halfwayDir), 0.0), sharpShininess);

    float softShininess = isGolden ? 24.0 : 18.0;
    float softSpec = pow(max(dot(norm, halfwayDir), 0.0), softShininess);

    // 3. Silky Anisotropic Spun-Sugar Ribbon Sheen (Pulled Taffy / Ribbon Candy)
    float brushRibbon = 0.0;
    if (isBrushed) {
        vec3 tangent = normalize(vec3(-0.707, 0.20, 0.707));
        vec3 tProj = normalize(tangent - norm * dot(tangent, norm));
        float dotTH = dot(tProj, halfwayDir);
        float anisoLobe = sqrt(max(0.0, 1.0 - dotTH * dotTH));
        brushRibbon = pow(anisoLobe, isGolden ? 32.0 : 24.0) * max(dot(norm, lightDir1), 0.0) * 0.45;
    }

    vec3 specColor = isGolden ? vec3(1.0, 0.88, 0.40) : vec3(1.0, 1.0, 1.0);
    float specStrength = isGolden ? 1.30 : 0.90;
    vec3 specular = (sharpSpec * 0.85 + softSpec * 0.30 + brushRibbon) * specStrength * specColor;

    // C. Sugar Crystal Micro-Sparkles (Twinkling granulated sugar grains)
    vec3 sparkCoord = floor((LocalPos + 1.0) * 32.0);
    float sparkHash = hash3(sparkCoord);
    float sugarSparkle = 0.0;
    if (sparkHash > 0.84) {
        sugarSparkle = pow(max(dot(norm, halfwayDir), 0.0), 64.0) * 0.65;
    }

    // D. Soft Internal Emissive Candy Warmth
    vec3 candyGlow = baseColor * 0.08;

    vec3 finalColor = ambient + diffuse + specular + rimGlow + candyGlow + sugarSparkle * specColor;

    // E. Candy Game Vibrancy / Saturation Boost
    float lum = dot(finalColor, vec3(0.299, 0.587, 0.114));
    finalColor = mix(vec3(lum), finalColor, 1.25); // +25% juicy saturation boost

    FragColor = vec4(finalColor, 1.0);
}

#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec2 uResolution;
uniform float uSigma1;
uniform float uSigma2;
uniform float uContrast;  // Optional tweak (1.0 = linear, >1 boosts contrast)

const int KERNEL_RADIUS = 5;

float gaussian(float x, float sigma) {
    return exp(- (x * x) / (2.0 * sigma * sigma)) / (2.0 * 3.14159265 * sigma * sigma);
}

vec3 blur(sampler2D tex, vec2 uv, float sigma) {
    vec2 texelSize = 1.0 / uResolution;
    vec3 result = vec3(0.0);
    float totalWeight = 0.0;

    for (int x = -KERNEL_RADIUS; x <= KERNEL_RADIUS; ++x) {
        for (int y = -KERNEL_RADIUS; y <= KERNEL_RADIUS; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            float weight = gaussian(length(vec2(x, y)), sigma);
            result += texture(tex, uv + offset).rgb * weight;
            totalWeight += weight;
        }
    }

    return result / totalWeight;
}

vec3 enhanceContrast(vec3 color, float contrastFactor) {
    // Normalize DoG result around 0.5, boost contrast
    color = (color + 1.0) * 0.5;          // Map [-1,1] to [0,1]
    color = pow(color, vec3(contrastFactor));  // Apply nonlinear contrast stretch
    return clamp(color, 0.0, 1.0);
}

void main() {
    vec3 blur1 = blur(uTexture, vUV, uSigma1);
    vec3 blur2 = blur(uTexture, vUV, uSigma2);
    vec3 dog = blur1 - blur2;

    vec3 contrastBoosted = enhanceContrast(dog, uContrast);
    FragColor = vec4(contrastBoosted, 1.0);
}

#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec2 texelSize;
uniform vec2 downscaleFactor; // e.g. (8.0, 8.0)

void main() {
    vec2 fullResolution = 1.0 / texelSize;

    // Convert from [0,1] TexCoords → pixel space
    vec2 pixelCoord = TexCoords * fullResolution;
    vec2 tileOrigin = floor(pixelCoord / downscaleFactor) * downscaleFactor;

    vec3 colorSum = vec3(0.0);
    float lumSum = 0.0;

    for (int y = 0; y < int(downscaleFactor.y); ++y) {
        for (int x = 0; x < int(downscaleFactor.x); ++x) {
            vec2 offset = tileOrigin + vec2(x, y);
            vec2 sampleUV = offset / fullResolution;
            vec3 c = texture(image, sampleUV).rgb;
            colorSum += c;
            lumSum += dot(c, vec3(0.299, 0.587, 0.114));
        }
    }

    float totalSamples = downscaleFactor.x * downscaleFactor.y;
    vec3 avgColor = colorSum / totalSamples;
    float avgLum = lumSum / totalSamples;

    FragColor = vec4(avgColor, avgLum);
}

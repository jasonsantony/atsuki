#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec2 texelSize;

void main() {

    // Prevent sobel from edge detecting literal image edges
    if (TexCoords.x < texelSize.x || TexCoords.x > 1.0 - texelSize.x ||
        TexCoords.y < texelSize.y || TexCoords.y > 1.0 - texelSize.y) {
        FragColor = vec4(0.0);
        return;
    }

    float kernelX[9] = float[](
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    );
    float kernelY[9] = float[](
        -1, -2, -1,
         0,  0,  0,
         1,  2,  1
    );

    float gx = 0.0;
    float gy = 0.0;
    int i = 0;

    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            vec2 offset = vec2(x, y) * texelSize;
            float intensity = texture(image, TexCoords + offset).r;
            gx += kernelX[i] * intensity;
            gy += kernelY[i] * intensity;
            i++;
        }
    }

    float edge = sqrt(gx * gx + gy * gy);
    float bw = edge > 0.2 ? 1.0 : 0.0;
    FragColor = vec4(vec3(bw), 1.0);
}

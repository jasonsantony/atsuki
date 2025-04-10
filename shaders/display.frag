#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;

void main() {
    float lum = texture(image, TexCoords).r;
    FragColor = vec4(vec3(lum), 1.0); // should be 0 or 1 in your case
}

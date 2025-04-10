#version 330 core

in vec2 TexCoords;
out float FragColor;

uniform sampler2D image;

void main() {
    vec3 color = texture(image, TexCoords).rgb;
    FragColor = dot(color, vec3(0.299, 0.587, 0.114));
}

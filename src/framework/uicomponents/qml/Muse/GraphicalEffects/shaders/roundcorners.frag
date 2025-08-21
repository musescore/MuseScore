#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float radius;
    vec2 sourceSize;
};

layout(binding = 1) uniform sampler2D source;

void main() {
    vec4 sourceColor = texture(source, qt_TexCoord0);

    // Convert texture coordinates to pixel coordinates
    vec2 pixelPos = qt_TexCoord0 * sourceSize;

    // Calculate the distance to the nearest corner of the rounded rectangle
    vec2 center = sourceSize * 0.5;
    vec2 halfSize = sourceSize * 0.5;

    // Distance from center to edge, accounting for corner radius
    vec2 cornerDistance = abs(pixelPos - center) - (halfSize - radius);

    // Calculate the signed distance to the rounded rectangle boundary
    float dist = length(max(cornerDistance, 0.0)) + min(max(cornerDistance.x, cornerDistance.y), 0.0) - radius;

    // Use smoothstep by way of anti-aliasing
    float alpha = 1.0 - smoothstep(-0.4, 0.4, dist);

    fragColor = sourceColor * alpha * qt_Opacity;
}

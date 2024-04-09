#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrColorBuffer;
uniform sampler2D blurColorBuffer;
uniform bool bloom;
uniform float exposure;

void main()
{
    vec3 hdrColor = texture(hdrColorBuffer, TexCoords).rgb;
    vec3 bloomColor = texture(blurColorBuffer, TexCoords).rgb;
    if(bloom)
        hdrColor += bloomColor;

    // HDR -> LDR (tone mapping)
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    FragColor = vec4(result, 1.0);
}
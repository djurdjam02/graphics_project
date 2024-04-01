#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform sampler2D texture1;
uniform bool sl;
uniform SpotLight spotLight;

// added spotlight for icicles because without it they were still visible even when they weren't in the
// radius of the spotlight's direction

void main()
{
    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.a < 0.1)
        discard;
    if(sl) {
        vec3 lightDir = normalize(spotLight.position - FragPos);
        float theta = dot(lightDir, normalize(-spotLight.direction));
        float epsilon = spotLight.cutOff - spotLight.outerCutOff;
        float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
        texColor *= intensity;
    }
    FragColor = texColor;
}

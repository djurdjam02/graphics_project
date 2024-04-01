#version 330 core

in vec3 FragPos;
out vec4 FragColor;

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

uniform bool sl;
uniform SpotLight spotLight;
uniform vec3 myColor;
// added spotlight here for exactly the same reason as with the icicles

void main()
{
    vec3 finalColor = myColor;
    if(sl) {
        vec3 lightDir = normalize(spotLight.position - FragPos);
        float theta = dot(lightDir, normalize(-spotLight.direction));
        float epsilon = spotLight.cutOff - spotLight.outerCutOff;
        float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);
        finalColor *= intensity;
    }
    FragColor = vec4(finalColor, 1.0);
}
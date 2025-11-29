#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;

// Material
uniform vec3  materialSpecular;
uniform float materialShininess;

// Luz
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

void main()
{
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    vec3 ambient  = lightAmbient * objectColor;

    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = lightDiffuse * diff * objectColor;

    vec3 viewDir     = normalize(viewPos - FragPos);
    vec3 reflectDir  = reflect(-lightDir, norm);
    float spec       = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular    = lightSpecular * spec * materialSpecular;

    vec3 result = ambient + diffuse + specular;
    FragColor  = vec4(result, 1.0);
}

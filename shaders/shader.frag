#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    //Ambint light
    vec3 ambient =  light.ambientIntensity * vec3(1.0f,1.0f,1.0f);
  	
    //Point light
    vec3 norm = normalize(Normal);

    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.color * light.diffuseIntensity * light.diffuse * diff; 

    float exponant = texture2D(material.specular, TexCoords.xy).r * 255;
    if (exponant == 0) {exponant = material.shininess;}

    //Specular light
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), exponant);
    vec3 specular = light.color * light.specular * spec;  
   


    vec3 result = (ambient + diffuse + specular) * texture2D(material.diffuse, TexCoords.xy).rgb;

    FragColor = vec4(result, 1.0);
} 
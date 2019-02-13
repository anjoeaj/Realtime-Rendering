#version 330 core
#define PI 3.14159265
in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;
in BumpMapTangents {
    mat3 TBNmat;
    vec3 TangentFragPos;
	vec3 T;
	vec3 B;
	vec3 N;
} bumpMapTangents;


struct DirectionalLight 
{
	vec3 colour;
	float ambientIntensity;
	vec3 direction;
	float diffuseIntensity;
};

struct PointLight
{
	vec3 position;
	float constant;
    float linear;
    float quadratic;
} pointLight;

struct Material
{
	float specularIntensity;
	float shininess;
	vec3  albedo;
    float metallic;
    float roughness;
	
	int illuminationType;
};



in vec3 FragPos;

out vec4 colour;

uniform vec3 eyePosition;
uniform vec3 pointLightPosition;
uniform samplerCube skybox;
uniform int textureMapType;

uniform sampler2D theTexture;
uniform sampler2D textureNormal;

uniform DirectionalLight directionalLight;
uniform Material material;

vec3 newNormal = Normal;

uniform float normalMappingIntensity;
float attenuation;

const vec3 color_specular = vec3 (1.0, 1.0, 1.0);

void SetPointLightProperties(){
	//pointLight.position = vec3(5.0,5.0,5.0);
	pointLight.constant = 1.0f;
	pointLight.linear = 0.07f;
	pointLight.quadratic = 0.017f;

	float distance    = length(pointLightPosition - FragPos);
	attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + 
    		    pointLight.quadratic * (distance * distance));  
}

float FindDistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float FindGeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float FindGeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = FindGeometrySchlickGGX(NdotV, roughness);
    float ggx1  = FindGeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}   


vec4 calculateCookTorranceIllumination()
{
	
	//These values have been temporarily sourced here by hardcoding. Ideally this should come from the main loop by updating the uniforms.
	vec3 albedo1 = vec3(1.0f,1.0f,1.0f);
	float roughness1 = 0.3;
	float metallic1 = 0.9;
	
	//Additional darkening of the model has been done to make it look a little black and glossy.
	vec3 ambientColour = vec3(directionalLight.colour) * directionalLight.ambientIntensity * vec3(0.18); //to darken the model
	//vec3 ambientColour = vec3(directionalLight.colour) * directionalLight.ambientIntensity;
	vec3 normal = normalize(Normal);
	
	vec3 lightDir = normalize(directionalLight.direction); 
	float distance = length(directionalLight.direction); 
	float NdotL = clamp(dot(normal, lightDir), 0.0f, 1.0f);
	//directionalLight is used, so we dont need to calculate radiance and attenuation
	//float attenuation = 1.0 / (distance * distance); 
    //vec3 radiance     = directionalLight.colour * attenuation; 
	
	vec3 viewDir = normalize((-eyePosition) - FragPos);
	vec3 halfVec = normalize(lightDir + viewDir);
	
	//precalculation of dot products
	
	float NdotH = clamp(dot(normal, halfVec), 0.0f, 1.0f);
	float NdotV = clamp(dot(normal, viewDir), 0.0f, 1.0f);
	float VdotH = clamp(dot(viewDir, halfVec), 0.0f, 1.0f);
	
	
	//Fresnel Equation - initially set to 0.04
	vec3 F0 = vec3 (0.04);
	F0 = mix(F0, albedo1, metallic1);
	vec3 F = fresnelSchlick(max(dot(halfVec, viewDir), 0.0), F0);
	
	//Normal Distribution Function
	float NDF = FindDistributionGGX(normal, halfVec, roughness1);    
	
	//Geometry Function
	float G = FindGeometrySmith(normal, viewDir, lightDir, roughness1);

	//Approximate Cook-Torrance BRDF
	vec3 numerator = NDF * G * F;
	float denom = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
	vec3 specular = numerator / max(denom, 0.001);  
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
  
	kD *= 1.0 - metallic1;	
    vec3 Lo = vec3(0.0);
    Lo += (kD * albedo1 / PI + specular)  * NdotL; // no radiance is considered here because we use directional light
	return(vec4(ambientColour + Lo, 1.0f));
}

vec4 cookTorrance(){
	return calculateCookTorranceIllumination();
}

vec4 pointLightShading(){
	// ambient
    vec3 ambient = directionalLight.colour * directionalLight.ambientIntensity;//* texture(material.diffuse, TexCoords).rgb;
  	
    // diffuse 
    //vec3 norm = normalize(Normal);
	vec3 norm = normalize(newNormal);
    vec3 lightDir = normalize(pointLightPosition - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = directionalLight.colour * directionalLight.diffuseIntensity * diff ;//* texture(material.diffuse, TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(eyePosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = directionalLight.colour * material.specularIntensity * spec ;//* texture(material.specular, TexCoords).rgb;  
    
    // attenuation
    float distance    = length(pointLightPosition - FragPos);
    float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation;   
        
    vec3 result = ambient + diffuse + specular;
    return vec4(texture(theTexture, TexCoord).rgb * result, 1.0);
	
}

vec4 Phong(){
	
	vec4 ambientColour = vec4(directionalLight.colour, 1.0f) * directionalLight.ambientIntensity;
	
	float diffuseFactor = max(dot(normalize(newNormal), normalize(directionalLight.direction)), 0.0f);
	vec4 diffuseColour = vec4(directionalLight.colour, 1.0f) * directionalLight.diffuseIntensity * diffuseFactor;
	
	vec4 specularColour = vec4(0, 0, 0, 0);
	vec3 fragToEye;
	if(diffuseFactor > 0.0f)
	{
		fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(directionalLight.direction, normalize(newNormal)));
		
		float specularFactor = dot(fragToEye, reflectedVertex);
		if(specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularColour = vec4(directionalLight.colour * material.specularIntensity * specularFactor, 1.0f);
		}
	}
	
	//diffuse color and real texture
	vec3 mainTex = texture(theTexture, TexCoord).rgb;
	
	//calculate an ambient Colour
	vec3 mainTexModified = normalMappingIntensity * mainTex;
	
	//apply point light
	ambientColour  *= attenuation; 
	diffuseColour  *= attenuation;
	specularColour *= attenuation; 
	
	return vec4(mainTexModified, 1.0) * (ambientColour + diffuseColour + specularColour);
}

void alterNormals(){
	
	newNormal = texture(textureNormal, TexCoord).rgb;
	
	//-1 to 1 range // tangent space
	newNormal = normalize(newNormal * 2.0 - 1.0);
	
	newNormal = normalize(bumpMapTangents.TBNmat * newNormal);
	
}

vec4 normalMapping(){
	//now get the normals from the new texture
	vec3 newNormal = texture(textureNormal, TexCoord).rgb;
	
	//-1 to 1 range // tangent space
	newNormal = normalize(newNormal * 2.0 - 1.0);
	
	newNormal = normalize(bumpMapTangents.TBNmat * newNormal);
	

	
	//diffuse color and real texture
	vec3 diffuseColor = texture(theTexture, TexCoord).rgb;
	
	//calculate an ambient Colour
	vec3 ambient = normalMappingIntensity * diffuseColor;
	
	vec3 tangentLightPos =  directionalLight.direction;//vec3(10.0,10.0,10.0);
	
	vec3 tangentFragPos =  bumpMapTangents.TangentFragPos;
	
	vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
	lightDir = bumpMapTangents.TBNmat * lightDir;
	float diff = max(dot(lightDir, newNormal), 0.0);
	vec3 diffuse = diff * diffuseColor;
	
	vec3 TangentViewPos = eyePosition;
	
	vec3 viewDir = normalize(TangentViewPos - tangentFragPos);
	viewDir = bumpMapTangents.TBNmat * viewDir;
    vec3 reflectDir = reflect(-lightDir, newNormal);
    vec3 halfwayDir = normalize(newNormal + viewDir);  
    float spec = pow(max(dot(newNormal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.5) * spec;
	
	//return vec4 (bumpMapTangents.T, 1.0f);
	return vec4(ambient + diffuse + specular, 1.0);
	
}

vec4 sinusoidalBumpMapping(){
		return vec4(0.0f);
}

vec4 Toon(){
	//Implement toon shading
	//For a different range of values for intensity, apply different colors
	vec4 ambientColour = vec4(directionalLight.colour, 1.0f) * directionalLight.ambientIntensity;
	
	float diffuseFactor = max(dot(normalize(Normal), normalize(directionalLight.direction)), 0.0f);
	vec4 diffuseColour = vec4(directionalLight.colour, 1.0f) * directionalLight.diffuseIntensity * diffuseFactor;
	
	
	vec3 initColor = vec3(1.0f,1.0f,1.0f);
	//specularColour.xyz = ComputeCookTorranceReflection(directionalLight.direction.xyz, fragToEye.xyz, Normal.xyz, initColor.xyz);
	//specularColour = vec4(specularColour.xyz, 1.0f);
	//colour = texture(theTexture, TexCoord) * (ambientColour + diffuseColour + specularColour);
	colour = (ambientColour + diffuseColour );
	
	//Implement toon shading
	//For a different range of values for intensity, apply different colors
	
	const vec4 brown_1 = vec4(245.0,223.0,217.0,155.0)/255.0; //lightest
	const vec4 brown_2 = vec4(235.0,190.0,176.0,155.0)/255.0;
	const vec4 brown_3 = vec4(191.0,82.0,49.0,155.0)/255.0;
	const vec4 brown_4 = vec4(84.0,36.0,22.0,155.0)/255.0;
	const vec4 brown_5 = vec4(57.0,24.0,15.0,155.0)/255.0; //darkest
	
	if(diffuseFactor > 0.9)
		colour *= brown_5;
	if(diffuseFactor > 0.7)
		colour *= brown_4;
	if(diffuseFactor > 0.4)
		colour *= brown_3;
	if(diffuseFactor > 0.25)
		colour *= brown_2;
	else
		colour *= brown_1;
	
	return colour;
}

void main()
{
	//printf("Illum type is %d", material.illuminationType);
	//colour = Phong();
	SetPointLightProperties();
	if(textureMapType == 1){
		alterNormals();
		//colour = normalMapping();
		//colour = Phong();
		colour = pointLightShading();
		//colour = vec4 (bumpMapTangents.T, 1.0f);
	}else {
		colour = pointLightShading();
	}
		
	/*
	if(material.illuminationType == 0)
		colour = Phong();
	if(material.illuminationType == 1)
		colour = Toon();
	if(material.illuminationType == 2)
		colour = cookTorrance();
	*/
	
	/*
	//Reflectance starts
	vec3 ViewDir = normalize(FragPos - eyePosition);
	vec3 HalfVec = normalize(ViewDir - directionalLight.direction);
	
	//reflection
	vec3 ReflectedRay = reflect(ViewDir, normalize(Normal));
    vec3 ReflectedColour = texture(skybox, ReflectedRay).rgb;
	
	colour = vec4(ReflectedColour, 1.0f);
	
	//refraction
	float ratio = 1.00 / 2.42;
    //vec3 I = normalize(FragPos - eyePosition);
    vec3 RefractedRay = refract(ViewDir, normalize(-Normal), ratio);
	vec3 RefractedColour = texture(skybox, RefractedRay).rgb;
	colour = vec4(RefractedColour, 1.0f);
	
	//Fresnel component - 
	//Wikipedia
	//In 3D computer graphics, Schlick's approximation, named after Christophe Schlick, 
	//is a formula for approximating the contribution of the Fresnel factor in 
	//the specular reflection of light from a non-conducting interface (surface) between two media.
	
	//According to Schlick's model, the specular reflection coefficient R can be approximated by:
	//diamond f0 = 0.45, ratio = 1/2.42
	//glass f0 = 0.31 ratio = 1/1.52
	
	
	vec3 F0 = vec3 (0.45);
	//F0 = mix(F0, albedo1, metallic1);
	//vec3 reflectionCoefficient = fresnelSchlick(max(dot(HalfVec, ViewDir), 0.0), F0);
	vec3 reflectionCoefficient = fresnelSchlick(max(dot(HalfVec, ViewDir), 0.0), F0);

	//With Fresnel component
	colour = vec4(mix(RefractedColour, ReflectedColour, reflectionCoefficient), 1.0);
	
	/*
	//Chromatic Abberation
	float ratioRed = ratio - 0.05;
	float ratioGreen = ratio - 0.03;
	float ratioBlue = ratio - 0.01;
	
	vec3 RedRefraction = refract(ViewDir, normalize(-Normal), ratioRed);
	vec3 GreenRefraction = refract(ViewDir, normalize(-Normal), ratioGreen);
	vec3 BlueRefraction = refract(ViewDir, normalize(-Normal), ratioBlue);
	
	vec3 abberatedColor;
	abberatedColor.r = vec3(texture(skybox, RedRefraction)).r;
	abberatedColor.g = vec3(texture(skybox, GreenRefraction)).g;
	abberatedColor.b = vec3(texture(skybox, BlueRefraction)).b;
	
    colour = vec4(mix(ReflectedColour, abberatedColor, reflectionCoefficient), 1.0);
	*/
	//chromatic abberation
	
}

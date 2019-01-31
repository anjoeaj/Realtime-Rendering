#version 330
#define PI 3.14159265
in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;

struct DirectionalLight 
{
	vec3 colour;
	float ambientIntensity;
	vec3 direction;
	float diffuseIntensity;
};

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



uniform sampler2D theTexture;
uniform DirectionalLight directionalLight;
uniform Material material;

const vec3 color_specular = vec3 (1.0, 1.0, 1.0);


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

vec4 Phong(){
	vec4 ambientColour = vec4(directionalLight.colour, 1.0f) * directionalLight.ambientIntensity;
	
	float diffuseFactor = max(dot(normalize(Normal), normalize(directionalLight.direction)), 0.0f);
	vec4 diffuseColour = vec4(directionalLight.colour, 1.0f) * directionalLight.diffuseIntensity * diffuseFactor;
	
	vec4 specularColour = vec4(0, 0, 0, 0);
	vec3 fragToEye;
	if(diffuseFactor > 0.0f)
	{
		fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(directionalLight.direction, normalize(Normal)));
		
		float specularFactor = dot(fragToEye, reflectedVertex);
		if(specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularColour = vec4(directionalLight.colour * material.specularIntensity * specularFactor, 1.0f);
		}
	}
	
	return texture(theTexture, TexCoord) * (ambientColour + diffuseColour + specularColour);
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
	if(material.illuminationType == 0)
		colour = Phong();
	if(material.illuminationType == 1)
		colour = Toon();
	if(material.illuminationType == 2)
		colour = cookTorrance();
	
	
	//vec3 initColor = vec3(1.0f,1.0f,1.0f);
	//specularColour.xyz = ComputeCookTorranceReflection(directionalLight.direction.xyz, fragToEye.xyz, Normal.xyz, initColor.xyz);
	//specularColour = vec4(specularColour.xyz, 1.0f);
	//colour = texture(theTexture, TexCoord) * (ambientColour + diffuseColour + specularColour);
	
	

	//colour = (ambientColour + diffuseColour + specularColour);
	
	
}

const int NUM_SAMPLES = 1024;
const float speed = 4;
void rayCasting(){
	vec3 fragToEye = normalize(eyePosition - FragPos);
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		float t = i/(NUM_SAMPLES - 1.0) * speed;
		vec3 p = (FragPos + fragToEye * t);

		if (max(abs(p.x), max(abs(p.y), abs(p.z))) <= 1) { //intersection
		  
		  //color = forwardStep(color, tf(p));

		}
	}
}	
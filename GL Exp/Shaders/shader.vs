#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

out vec4 vCol;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

out BumpMapTangents {
    mat3 TBNmat;
    vec3 TangentFragPos;
	vec3 T;
	vec3 B;
	vec3 N;
} bumpMapTangents;


void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	vCol = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);
	
	TexCoord = tex;
	//Tang = tangent;
	//BiTang = bitangent;
	
	Normal = mat3(transpose(inverse(model))) * norm;
	
	FragPos = (model * vec4(pos, 1.0)).xyz;
	
	//vec3 T = normalize(Normal * tangent);
	//vec3 N = normalize(Normal * norm);
	//T = normalize(T - dot(T, N) * N);
	//vec3 B = cross(N, T);
	
	vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(norm,    0.0)));
	
	mat3 TBN = transpose(mat3(T, B, N));
	bumpMapTangents.TangentFragPos = TBN * FragPos;
	bumpMapTangents.TBNmat  = TBN;
	bumpMapTangents.T = T;
	
}
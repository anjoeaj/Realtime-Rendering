#include "Model.h"

Model::Model()
{
}

void Model::RenderModel()
{
	for (size_t i = 0; i < meshList.size(); i++)
	{
		unsigned int materialIndex = meshToTex[i];

		if (materialIndex < textureList.size() && textureList[materialIndex])
		{
			textureList[materialIndex]->UseTexture();
		}

		meshList[i]->RenderMesh ();
	}
}

void Model::LoadModel(const std::string & fileName)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

	if (!scene)
	{
		printf("Model (%s) failed to load: %s", fileName, importer.GetErrorString());
		return;
	}

	LoadNode(scene->mRootNode, scene);

	LoadMaterials(scene);
}

void Model::LoadNode(aiNode * node, const aiScene * scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

//void Model::LoadMesh(aiMesh * mesh, const aiScene * scene)
//{
//	std::vector<GLfloat> vertices;
//	std::vector<unsigned int> indices;
//
//	for (size_t i = 0; i < mesh->mNumVertices; i++)
//	{
//		vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
//		if (mesh->mTextureCoords[0])
//		{
//			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
//		}
//		else {
//			vertices.insert(vertices.end(), { 0.0f, 0.0f });
//		}
//		vertices.insert(vertices.end(), { -mesh->mNormals[i].x, -mesh->mNormals[i].y, -mesh->mNormals[i].z });
//	}
//
//	for (size_t i = 0; i < mesh->mNumFaces; i++)
//	{
//		aiFace face = mesh->mFaces[i];
//		for (size_t j = 0; j < face.mNumIndices; j++)
//		{
//			indices.push_back(face.mIndices[j]);
//		}
//	}
//
//	Mesh* newMesh = new Mesh();
//	newMesh->CreateMesh(&vertices[0], &indices[0], vertices.size(), indices.size());
//	meshList.push_back(newMesh);
//	meshToTex.push_back(mesh->mMaterialIndex);
//}

void Model::LoadMesh(aiMesh * mesh, const aiScene * scene)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
		if (mesh->mTextureCoords[0])
		{
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		}
		else {
			vertices.insert(vertices.end(), { 0.0f, 0.0f });
		}
		vertices.insert(vertices.end(), { -mesh->mNormals[i].x, -mesh->mNormals[i].y, -mesh->mNormals[i].z });

		//insert tangents
		if (mesh->mTangents)
		{
			vertices.insert(vertices.end(), { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z });
		}
		else {
			vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f });
		}
		

		//insert bitangents
		if (mesh->mBitangents)
		{
			vertices.insert(vertices.end(), { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z });
		}
		else {
			vertices.insert(vertices.end(), { 0.0f, 0.0f, 0.0f });
		}
		
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	Mesh* newMesh = new Mesh();
	newMesh->CreateMesh(&vertices[0], &indices[0], vertices.size(), indices.size());
	meshList.push_back(newMesh);
	meshToTex.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene * scene)
{
	textureList.resize(scene->mNumMaterials);

	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		textureList[i] = nullptr;

		aiString ambient_texture_path, diffuse_texture_path, specular_texture_path, bump_texture_path, normal_texture_path;
		material->GetTexture(aiTextureType_AMBIENT, 0, &ambient_texture_path);
		material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_texture_path);
		material->GetTexture(aiTextureType_SPECULAR, 0, &specular_texture_path);
		material->GetTexture(aiTextureType_HEIGHT, 0, &bump_texture_path);
		material->GetTexture(aiTextureType_NORMALS, 0, &normal_texture_path);
/*
		printf("AmbientTexture: %s", ambient_texture_path.C_Str());
		printf("DiffuseTexture: %s", diffuse_texture_path.C_Str());
		printf("SpecularTexture: %s", specular_texture_path.C_Str());
		printf("BumpTexture: %s", bump_texture_path.C_Str());
		printf("Normal tex: %s", normal_texture_path.C_Str());*/

		int c2c = material->GetTextureCount(aiTextureType_DIFFUSE);
		int cc = material->GetTextureCount(aiTextureType_NORMALS);
		

		/*if (normal_texture_path.length != 0) {
			int idx = std::string(normal_texture_path.data).rfind("\\");
			std::string filename = std::string(normal_texture_path.data).substr(idx + 1);

			std::string texPath = std::string("Textures/") + filename;

			textureList[i] = new Texture(texPath.c_str());

			if (!textureList[i]->LoadTextureNormal())
			{
				printf("Failed to load normal texture at: %s\n", texPath);
				delete textureList[i];
				textureList[i] = nullptr;
			}
		}*/

		if (diffuse_texture_path.length != 0) {
			int idx = std::string(diffuse_texture_path.data).rfind("\\");
			std::string filename = std::string(diffuse_texture_path.data).substr(idx + 1);

			std::string texPath = std::string("Textures/") + filename;

			textureList[i] = new Texture(texPath.c_str());

			if (needsMipmap ? !textureList[i]->LoadTexture() : !textureList[i]->LoadTexture(false))
			{
				printf("Failed to load texture at: %s\n", texPath);
				delete textureList[i];
				textureList[i] = nullptr;
			}

			idx = std::string(normal_texture_path.data).rfind("\\");
			filename = std::string(normal_texture_path.data).substr(idx + 1);

			texPath = std::string("Textures/") + filename;

			//textureList[i] = new Texture(texPath.c_str());
			textureList[i]->SetFileLoc(texPath.c_str());

			if (!textureList[i]->LoadTextureNormal())
			{
				printf("Failed to load normal texture at: %s\n", texPath);
				delete textureList[i];
				textureList[i] = nullptr;
			}
		}

		/*if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				int idx = std::string(path.data).rfind("\\");
				std::string filename = std::string(path.data).substr(idx + 1);

				std::string texPath = std::string("Textures/") + filename;

				textureList[i] = new Texture(texPath.c_str());

				if (!textureList[i]->LoadTexture())
				{
					printf("Failed to load texture at: %s\n", texPath);
					delete textureList[i];
					textureList[i] = nullptr;
				}
			}else if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
			{
				int idx = std::string(path.data).rfind("\\");
				std::string filename = std::string(path.data).substr(idx + 1);

				std::string texPath = std::string("Textures/") + filename;

				textureList[i] = new Texture(texPath.c_str());

				if (!textureList[i]->LoadTexture())
				{
					printf("Failed to load texture at: %s\n", texPath);
					delete textureList[i];
					textureList[i] = nullptr;
				}
			}
		}else if (material->GetTextureCount(aiTextureType_NORMALS))
		{
			
		}*/

		if (!textureList[i])
		{
			textureList[i] = new Texture("Textures/plain.png");
			textureList[i]->LoadTextureA();
		}
	}
}

void Model::ClearModel()
{
	for (size_t i = 0; i < meshList.size(); i++)
	{
		if (meshList[i])
		{
			delete meshList[i];
			meshList[i] = nullptr;
		}
	}

	for (size_t i = 0; i < textureList.size(); i++)
	{
		if (textureList[i])
		{
			delete textureList[i];
			textureList[i] = nullptr;
		}
	}
}

Model::~Model()
{
}

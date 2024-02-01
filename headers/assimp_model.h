#ifndef ASSIMP_MODEL_H
#define ASSIMP_MODEL_H


#include <map>
#include <vector>
#include <glad/glad.h>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "texture.h"

#define GLCheckError() (glGetError() == GL_NO_ERROR)
#define INVALID_MATERIAL 0xFFFFFFFF

using namespace std;

class BasicMesh
{
public:
    BasicMesh() {};

    ~BasicMesh();

    bool LoadMesh(const std::string& Filename, unsigned int NumInstances);

    void Render(Shader shader);

    vector<glm::mat4> WorldMats;

    void PopulateBuffers();

protected:

    void Clear();
    virtual void InitSingleMesh(unsigned int MeshIndex, const aiMesh* paiMesh);

    struct BasicMeshEntry {
        BasicMeshEntry()
        {
            NumIndices = 0;
            BaseVertex = 0;
            BaseIndex = 0;
            MaterialIndex = INVALID_MATERIAL;
        }
        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };
    vector<BasicMeshEntry> m_Meshes;
    vector<unsigned int> m_Indices;

    const aiScene* m_pScene;


    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        VERTEX_BUFFER = 1,
        WORLD_MAT_BUFFER = 2,
        NUM_BUFFERS = 3
    };

    GLuint m_VAO = 0;

    GLuint m_Buffers[NUM_BUFFERS] = { 0 };

private:
    struct Vertex {
        aiVector3D Position;
        aiVector2D TexCoords;
        aiVector3D Normal;
    };
    vector<Vertex> m_Vertices;

    unsigned int NumInst = 0;
    vector<Material> m_Materials;

    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices);
    void InitAllMeshes(const aiScene* pScene);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index);
    void InitMatrices();

    bool LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    bool LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    bool LoadDiffuseTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int index);
    void LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadSpecularTextureFromFile(const string& dir, const aiString& Path, int MaterialIndex);

    void LoadColors(const aiMaterial* pMaterial, int index);
    string GetDirFromFilename(const string& Filename);
    Assimp::Importer m_Importer;
};


#endif
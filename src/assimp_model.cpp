#include "assimp_model.h"

using namespace std;

#define POSITION_LOCATION  0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION    2
#define WORLD_LOCATION 3

// La destructurtion
BasicMesh::~BasicMesh()
{
    Clear();
}


void BasicMesh::Clear()
{
    if (m_Buffers[0] != 0) {
        glDeleteBuffers(NUM_BUFFERS, m_Buffers);
    }
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}


bool BasicMesh::LoadMesh(const string& Filename, unsigned int NumInstances)
{
    // Effacer les anciennes donnees
    Clear();

    // Creation du VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    // m_bBueffer contient
    glGenBuffers(NUM_BUFFERS, m_Buffers);

    bool Ret = false;

    assert(NumInstances > 0);
    NumInst = NumInstances;
    // Importer le modele avec assimp "aiScene"

    m_pScene = m_Importer.ReadFile(Filename.c_str(), 
        aiProcess_Triangulate |                 // Triangule les faces
        aiProcess_JoinIdenticalVertices |       // Fusionne les vertices identiques
        aiProcess_GenUVCoords |                 // Genere les coordonnes de texture
        aiProcess_SortByPType |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FindInvalidData |
        aiProcess_FlipUVs |
        aiProcess_FlipWindingOrder |
        aiProcess_CalcTangentSpace |
        aiProcess_PreTransformVertices |
        aiProcess_GenSmoothNormals
    );
    
    if (!m_pScene) {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
        return false;
    }
    printf("Loaded model '%s'\n", Filename.c_str());
    Ret = InitFromScene(m_pScene, Filename);

    // Unbind VAO
    glBindVertexArray(0);

    return Ret;
}


bool BasicMesh::InitFromScene(const aiScene* pScene, const string& Filename)
{
    m_Meshes.resize(pScene->mNumMeshes);
    m_Materials.resize(pScene->mNumMaterials);

    unsigned int NumVertices = 0;
    unsigned int NumIndices = 0;

    CountVerticesAndIndices(pScene, NumVertices, NumIndices);
    printf("Total vertices %d, total indices %d\n", NumVertices, NumIndices);

    m_Vertices.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
    WorldMats.reserve(NumInst);

    InitAllMeshes(pScene);

    if (!InitMaterials(pScene, Filename)) {
        return false;
    }
    InitMatrices();
    PopulateBuffers();

    return GLCheckError();
}

void BasicMesh::InitMatrices()
{
    for (unsigned int i = 0; i < NumInst; i++) {
        if (i == 0) { WorldMats.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 1.0f))); }
        else
        WorldMats.push_back(glm::mat4(1.0f));
	}
}

void BasicMesh::CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices)
{
    printf("Num meshes      %d\n", pScene->mNumMeshes);
    printf("Num materials   %d\n", pScene->mNumMaterials);
    printf("Num textures    %d\n", m_pScene->mNumTextures);
    printf("Num lights      %d\n", m_pScene->mNumLights);
    printf("Num cameras     %d\n", m_pScene->mNumCameras);

    for (unsigned int i = 0 ; i < pScene->mNumMeshes; i++) {

        //Save the material index for this mesh
        m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        printf("Num mMaterialIndex %u\n", pScene->mMeshes[i]->mMaterialIndex);

        //Number of indices in this mesh
        m_Meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
        printf("Num indices %d\n", m_Meshes[i].NumIndices);

        //To start at the end of the last mesh
        m_Meshes[i].BaseVertex = NumVertices;
        printf("Base vertex %d\n", m_Meshes[i].BaseVertex);
        
        //To start at the end of the last mesh
        m_Meshes[i].BaseIndex = NumIndices;
        printf("Base index %d\n", m_Meshes[i].BaseIndex);

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Meshes[i].NumIndices;
    }
}

void BasicMesh::InitAllMeshes(const aiScene* pScene)
{
    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitSingleMesh(i, paiMesh);
    }
}


void BasicMesh::InitSingleMesh(unsigned int MeshIndex, const aiMesh* paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    const aiVector3D Normal(0.0f, 1.0f, 0.0f);
    Vertex v;

    // Pour chaque vertex
    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {

        // Recuperer les coordonnes du vertex
        const aiVector3D& pPos = paiMesh->mVertices[i];
        v.Position = aiVector3D(pPos.x, pPos.y, pPos.z);

        // Recupere les vecteurs de la normale sinon on met une normale par defaut
        v.Normal = paiMesh->HasNormals() ? paiMesh->mNormals[i] : Normal;

        // Recupere les coordonnes de la texture sinon on met une texture par defaut
        const aiVector3D& pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        v.TexCoords = aiVector2D(pTexCoord.x, pTexCoord.y);

        // Ajoute la structure au tableau de vertex
        m_Vertices.push_back(v);
    }

    // Pour chaque face
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        // Recupere les indices de la face
        const aiFace& Face = paiMesh->mFaces[i];
        m_Indices.push_back(Face.mIndices[0]);
        m_Indices.push_back(Face.mIndices[1]);
        m_Indices.push_back(Face.mIndices[2]);
    }
}


void BasicMesh::PopulateBuffers()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[VERTEX_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Vertices[0]) * m_Vertices.size(), &m_Vertices[0], GL_STATIC_DRAW);

    size_t NumFloats = 0;
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
    
    NumFloats += 3;
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));
    
    NumFloats += 2;
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(NumFloats * sizeof(float)));


    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_BUFFER]);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(6, 1);

    
}

void BasicMesh::Render(Shader shader)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInst, &WorldMats[0], GL_DYNAMIC_DRAW);

    glBindVertexArray(m_VAO);

    for (unsigned int i = 0 ; i < m_Meshes.size() ; i++) {
        const unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;
        assert(MaterialIndex < m_Materials.size());

        glUniform1f(shader.materialShininess, m_Materials[MaterialIndex].shine);

        glUniform3f(shader.lightAmbient, 
            			m_Materials[MaterialIndex].AmbientColor.r,
            			m_Materials[MaterialIndex].AmbientColor.g,
            			m_Materials[MaterialIndex].AmbientColor.b
        			);

        glUniform1i(shader.materialDiffuse, 0);
        if (m_Materials[MaterialIndex].pDiffuse) {
            m_Materials[MaterialIndex].pDiffuse->Bind(GL_TEXTURE0);
        }
        glUniform3f(shader.lightDiffuse, 
            m_Materials[MaterialIndex].DiffuseColor.r,
            m_Materials[MaterialIndex].DiffuseColor.g,
            m_Materials[MaterialIndex].DiffuseColor.b
            );

        glUniform1i(shader.materialSpecular, 1);
        if (m_Materials[MaterialIndex].pSpecularExponent) {
            m_Materials[MaterialIndex].pSpecularExponent->Bind(GL_TEXTURE1);
        }
        glUniform3f(shader.lightSpecular, 
            			m_Materials[MaterialIndex].SpecularColor.r,
            			m_Materials[MaterialIndex].SpecularColor.g,
            			m_Materials[MaterialIndex].SpecularColor.b
        			);
        glDrawElementsInstancedBaseVertex(GL_TRIANGLES,
                                          m_Meshes[i].NumIndices,
                                          GL_UNSIGNED_INT,
                                          (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
                                          NumInst,
                                          m_Meshes[i].BaseVertex);
    }
    glBindVertexArray(0);
}


bool BasicMesh::InitMaterials(const aiScene* pScene, const string& Filename)
{
    string Dir = GetDirFromFilename(Filename);

    bool Ret = true;

    for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) {

        const aiMaterial* pMaterial = pScene->mMaterials[i];
        aiString MaterialName = pMaterial->GetName();
        printf("Loading material %d name %s\n", i, MaterialName.C_Str());

        LoadTextures(Dir, pMaterial, i);
        LoadColors(pMaterial, i);
    }

    return Ret;
}

string BasicMesh::GetDirFromFilename(const string& Filename)
{

    string::size_type SlashIndex;

    SlashIndex = Filename.find_last_of("\\");
    if (SlashIndex == -1) {
        SlashIndex = Filename.find_last_of("/");
    }
    string Dir;

    if (SlashIndex == string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    return Dir;
}

void BasicMesh::LoadTextures(const string& Dir, const aiMaterial* pMaterial, int index)
{  
    printf("Texture count               %d\n", pMaterial->GetTextureCount(aiTextureType_NONE));
    printf("Diffuse texture count       %d\n", pMaterial->GetTextureCount(aiTextureType_DIFFUSE));
    printf("Specular texture count      %d\n", pMaterial->GetTextureCount(aiTextureType_SHININESS));
    printf("Ambient texture count       %d\n", pMaterial->GetTextureCount(aiTextureType_AMBIENT));
    printf("Normal texture count        %d\n", pMaterial->GetTextureCount(aiTextureType_NORMALS)); 
    printf("Height texture count        %d\n", pMaterial->GetTextureCount(aiTextureType_HEIGHT));
    printf("Opacity texture count       %d\n", pMaterial->GetTextureCount(aiTextureType_OPACITY));
    printf("Displacement texture count  %d\n", pMaterial->GetTextureCount(aiTextureType_DISPLACEMENT));
    printf("Lightmap texture count      %d\n", pMaterial->GetTextureCount(aiTextureType_LIGHTMAP));
    printf("Reflection texture count    %d\n", pMaterial->GetTextureCount(aiTextureType_REFLECTION));
    printf("Emissive texture count      %d\n", pMaterial->GetTextureCount(aiTextureType_EMISSIVE));
    printf("Unknown texture count       %d\n", pMaterial->GetTextureCount(aiTextureType_UNKNOWN));

    LoadDiffuseTexture(Dir, pMaterial, index);
    LoadSpecularTexture(Dir, pMaterial, index);
}


bool BasicMesh::LoadDiffuseTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    bool ret = false;
    m_Materials[MaterialIndex].pDiffuse = NULL;
    
    //Si le material a une texture diffuse
    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        // Essayer de Recuperer la texture diffuse
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture) {    
                // Si la texture est disponible Charger la texture diffuse
                ret = LoadDiffuseTextureEmbedded(paiTexture, MaterialIndex);
            } else {
                // Sinon charger la texture diffuse depuis un fichier
                ret = LoadDiffuseTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
    return ret;
}


bool BasicMesh::LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    m_Materials[MaterialIndex].pDiffuse = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].pDiffuse->Load(paiTexture->mWidth, paiTexture->pcData);
    if (!m_Materials[MaterialIndex].pDiffuse->Load()) {
        printf("Error loading diffuse texture embedded \n");
        return false;
    }
    printf("Embeddeded diffuse texture type '%s' at index %d\n", paiTexture->achFormatHint,MaterialIndex);
    return true;
}


bool BasicMesh::LoadDiffuseTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string p(Path.data);

    for (int i = 0 ; i < p.length() ; i++) {
        if (p[i] == '\\') 
            p[i] = '/';
    }
    if (p.substr(0, 2) == ".\\") {
        p = p.substr(2, p.size() - 2);
    }
    string FullPath = Dir + "/" + p;
    m_Materials[MaterialIndex].pDiffuse = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].pDiffuse->Load()) {
        printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
        return false;
    }
    printf("Loaded diffuse texture '%s' at index %d\n", FullPath.c_str(), MaterialIndex);
    return true;
}


void BasicMesh::LoadSpecularTexture(const string& Dir, const aiMaterial* pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].pSpecularExponent = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
        
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path) == AI_SUCCESS) {
            const aiTexture* paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture) {
                LoadSpecularTextureEmbedded(paiTexture, MaterialIndex);
            } else {
                LoadSpecularTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}


void BasicMesh::LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex)
{
    printf("Embeddeded specular texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pSpecularExponent = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].pSpecularExponent->Load(paiTexture->mWidth, paiTexture->pcData);
}


void BasicMesh::LoadSpecularTextureFromFile(const string& Dir, const aiString& Path, int MaterialIndex)
{
    string p(Path.data);

    for (int i = 0; i < p.length(); i++) {
        if (p[i] == '\\')
            p[i] = '/';
    }
    if (p.substr(0, 2) == ".\\") {
        p = p.substr(2, p.size() - 2);
    }
    string FullPath = Dir + "/" + p;

    m_Materials[MaterialIndex].pSpecularExponent = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].pSpecularExponent->Load()) {
        printf("Error loading specular texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else {
        printf("Loaded specular texture '%s'\n", FullPath.c_str());
    }
}

void BasicMesh::LoadColors(const aiMaterial* pMaterial, int index)
{
    aiColor3D AmbientColor(0.0f, 0.0f, 0.0f);
    aiColor3D AllOnes(1.0f, 1.0f, 1.0f);

    int ShadingModel = 0;
    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS) {
        printf("Shading model %d\n", ShadingModel);
    }

    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS) {
        printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
        m_Materials[index].AmbientColor = AmbientColor*0.2;
    } else {
        m_Materials[index].AmbientColor = AllOnes;
    }


    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, m_Materials[index].DiffuseColor) == AI_SUCCESS) {
        printf("Loaded diffuse color [%f %f %f]\n", m_Materials[index].DiffuseColor.r, m_Materials[index].DiffuseColor.g, m_Materials[index].DiffuseColor.b);
    }

    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, m_Materials[index].SpecularColor) == AI_SUCCESS) {
        printf("Loaded specular color [%f %f %f]\n", m_Materials[index].SpecularColor.r, m_Materials[index].SpecularColor.g, m_Materials[index].SpecularColor.b);

    }

    if (pMaterial->Get(AI_MATKEY_SHININESS, m_Materials[index].shine) == AI_SUCCESS) {
		printf("Loaded shininess %f\n", m_Materials[index].shine);
	}

}

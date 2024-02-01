#ifndef TEXTURE_H
#define TEXTURE_H
#include <string>
#include <glad/glad.h>
#include <iostream>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

//#include "util.h"

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName);

    Texture(GLenum TextureTarget);

    bool Load();

    bool Load(unsigned int BufferSize, void* pImageData);

    void Load(const std::string& Filename);

    void LoadRaw(int Width, int Height, int BPP, unsigned char* pImageData);

    void Bind(GLenum TextureUnit);

    void GetImageSize(int& ImageWidth, int& ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

    GLuint GetTexture() const { return m_textureObj; }

private:
    void LoadInternal(void* pImageData);

    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
};

class Material {

public:
    aiColor3D AmbientColor = aiColor3D(0.0f, 0.0f, 0.0f);
    aiColor3D DiffuseColor = aiColor3D(0.0f, 0.0f, 0.0f);
    aiColor3D SpecularColor = aiColor3D(0.0f, 0.0f, 0.0f);
    float shine = 0.0f;

    Texture* pDiffuse = NULL;
    Texture* pSpecularExponent = NULL;
};


#endif
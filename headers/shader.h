#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

std::string get_file_contents(const char* filename);
std::string get_file(const char* filename);

class Shader
{
public:
	
	GLuint ID;
	GLuint viewPos;
	GLuint materialDiffuse;
	GLuint materialSpecular;
	GLuint materialShininess;
	GLuint lightPositon;
	GLuint lightColor;
	GLuint lightAmbient;
	GLuint lightDiffuse;
	GLuint lightSpecular;
	GLuint lightAmbientIntensity;
	GLuint lightDiffuseIntensity;

	Shader(const char* vertexFile, const char* fragmentFile);

	
	void Activate();
	
	void Delete();
private:
	
	void compileErrors(unsigned int shader, const char* type);
	void initUniforms();
};


#endif
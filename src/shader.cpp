#include"shader.h"

std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

std::string get_file(const char* filename) {
	std::string code;
	std::ifstream stream(filename, std::ios::in);
	if (stream.is_open()) {
		std::string line = "";
		while (getline(stream, line)) {
			code += "\n" + line;
		}
		stream.close();
	}
	else {
		std::cout << "Impossible to open " << filename << std::endl;
		exit(-1);
	}
	return code;
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	std::string vertexCode = get_file(vertexFile);
	const char* vertexSource = vertexCode.c_str();

	std::string fragmentCode = get_file(fragmentFile);
	const char* fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	compileErrors(vertexShader, "VERTEX");

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	compileErrors(fragmentShader, "FRAGMENT");

	
	ID = glCreateProgram();
	if (ID == 0)
	{
		std::cout << "Error creating shader program!" << std::endl;
		exit(1); 
	}

	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	glValidateProgram(ID);
	compileErrors(ID, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	initUniforms();
}

void Shader::initUniforms() {
	viewPos = glGetUniformLocation(ID, "viewPos");
	materialDiffuse = glGetUniformLocation(ID, "material.diffuse");
	materialSpecular = glGetUniformLocation(ID, "material.specular");
	materialShininess = glGetUniformLocation(ID, "material.shininess");
	lightPositon = glGetUniformLocation(ID, "light.position");
	lightColor = glGetUniformLocation(ID, "light.color");
	lightAmbient = glGetUniformLocation(ID, "light.ambient");
	lightDiffuse = glGetUniformLocation(ID, "light.diffuse");
	lightSpecular = glGetUniformLocation(ID, "light.specular");
	lightAmbientIntensity = glGetUniformLocation(ID, "light.ambientIntensity");
	lightDiffuseIntensity = glGetUniformLocation(ID, "light.diffuseIntensity");
}
void Shader::Activate()
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}

void Shader::compileErrors(unsigned int shader, const char* type)
{
	GLint hasCompiled = GL_FALSE;
	GLint Success = GL_FALSE;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
		glGetProgramiv(shader, GL_VALIDATE_STATUS, &Success);
		if (Success == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_VALIDATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}
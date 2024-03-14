#include "Shader.h"
#include <glad/glad.h>
#include <fstream>

#include "Logger/Logger.h"


Shader::Shader(const std::string& vsPath, const std::string& fsPath)
	: m_UniformLocations {}, m_Shader {}
{
	std::string vsCode;
	std::string fsCode;
	std::ifstream vsStream;
	std::ifstream fsStream;

	vsStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fsStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		vsStream.open(vsPath);
		fsStream.open(fsPath);

		std::stringstream ss[2];
		ss[0] << vsStream.rdbuf();
		ss[1] << fsStream.rdbuf();

		vsStream.close();
		fsStream.close();

		vsCode = ss[0].str();
		fsCode = ss[1].str();
	}
	catch (std::ifstream::failure e)
	{
		LOG_ERROR("Cannot read the shader files: \ninfo: \n\t", e.what());
	}

	const char* vsSource = vsCode.c_str();
	const char* fsSource = fsCode.c_str();

	uint32_t vs = compileShader(ShaderType::VertexShader, vsSource);
	uint32_t fs = compileShader(ShaderType::FragmentShader, fsSource);
	
	if (!linkProgram(vs, fs))
	{
		__debugbreak();
	}
}

Shader::~Shader()
{
	if (!m_Shader)
	{
		LOG_ERROR("Invaild shader program. [In destructor]");
		return;
	}

	glDeleteProgram(m_Shader);
}

void Shader::bind() const
{
	glUseProgram(m_Shader);
}

void Shader::unbind() const
{
	glUseProgram(0);
}

void Shader::setUnifrom1f(const std::string& name, float value)
{
	if (m_UniformLocations.find(name) != m_UniformLocations.end())
	{
		glUniform1f(m_UniformLocations.at(name), value);
		return;
	}

	int32_t location = glGetUniformLocation(m_Shader, name.c_str());
	m_UniformLocations[name] = location;
	glUniform1f(location, value);
}

void Shader::setUnifrom2f(const std::string& name, float v0, float v1)
{
	if (m_UniformLocations.find(name) != m_UniformLocations.end())
	{
		glUniform2f(m_UniformLocations.at(name), v0, v1);
		return;
	}

	int32_t location = glGetUniformLocation(m_Shader, name.c_str());
	m_UniformLocations[name] = location;
	glUniform2f(location, v0, v1);
}

void Shader::setUnifrom3f(const std::string& name, float v0, float v1, float v2)
{
	if (m_UniformLocations.find(name) != m_UniformLocations.end())
	{
		glUniform3f(m_UniformLocations.at(name), v0, v1, v2);
		return;
	}

	int32_t location = glGetUniformLocation(m_Shader, name.c_str());
	m_UniformLocations[name] = location;
	glUniform3f(location, v0, v1, v2);
}

uint32_t Shader::compileShader(const ShaderType type, const std::string& source)
{
	GLenum shaderType = {};
	if (type == Shader::ShaderType::VertexShader) shaderType = GL_VERTEX_SHADER;
	else if (type == Shader::ShaderType::FragmentShader) shaderType = GL_FRAGMENT_SHADER;
	else 
	{
		LOG_ERROR("Invalid Shader Type, {0}", (uint8_t)type);
		__debugbreak();
	}

	uint32_t shader = glCreateShader(shaderType);
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	int32_t result = {};
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		int32_t length = {};
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		
		GLchar* infoLog = new GLchar[length];
		glGetShaderInfoLog(shader, length, &length, infoLog);

		LOG_ERROR("Failed to compile {0} shader!", shaderType == GL_VERTEX_SHADER ? "Vertex" : "Fragment");
		LOG_ERROR("Error Infomation: \n");
		LOG_TRACE("{}", infoLog);

		delete[] infoLog;
		glDeleteShader(shader);

		return 0;
	}

	return shader;
}

bool Shader::linkProgram(const uint32_t vs, const uint32_t fs)
{
	m_Shader = glCreateProgram();
	glAttachShader(m_Shader, vs);
	glAttachShader(m_Shader, fs);

	glLinkProgram(m_Shader);

	int32_t result = {};
	glGetProgramiv(m_Shader, GL_LINK_STATUS, &result);
	if (!result)
	{
		int32_t length = {};
		glGetProgramiv(m_Shader, GL_INFO_LOG_LENGTH, &length);

		GLchar* infoLog = new GLchar[length];
		glGetProgramInfoLog(m_Shader, length, &length, infoLog);

		LOG_ERROR("Failed to link shader program!");
		LOG_ERROR("Error Infomation: \n");
		LOG_TRACE("{}", infoLog);

		delete[] infoLog;
		glDeleteProgram(m_Shader);

		return false;
	}

	glDetachShader(m_Shader, vs);
	glDetachShader(m_Shader, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	glValidateProgram(m_Shader);

	return true;
}

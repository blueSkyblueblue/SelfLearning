#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
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

	uint32_t vs = compileShader(ShaderType::VertexShader, vsCode);
	uint32_t fs = compileShader(ShaderType::FragmentShader, fsCode);
	linkProgram(vs, fs);
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

void Shader::setUniformMat4(const std::string& name, glm::mat4 mat)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setUniform1f(const std::string& name, float value)
{
	glUniform1f(getUniformLocation(name), value);
}

void Shader::setUniform2f(const std::string& name, float v0, float v1)
{
	glUniform2f(getUniformLocation(name), v0, v1);
}

void Shader::setUniform3f(const std::string& name, float v0, float v1, float v2)
{
	glUniform3f(getUniformLocation(name), v0, v1, v2);
}

uint32_t Shader::compileShader(const ShaderType type, const std::string& source)
{
	GLenum shaderType = (type == ShaderType::VertexShader) ?
		GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;

	uint32_t shader = glCreateShader(shaderType);
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	hasError(GL_COMPILE_STATUS, shader);

	return shader;
}

bool Shader::linkProgram(const uint32_t vs, const uint32_t fs)
{
	m_Shader = glCreateProgram();
	glAttachShader(m_Shader, vs);
	glAttachShader(m_Shader, fs);

	glLinkProgram(m_Shader);
	hasError(GL_LINK_STATUS, m_Shader, true);

	glValidateProgram(m_Shader);
	hasError(GL_VALIDATE_STATUS, m_Shader, true);

	glDetachShader(m_Shader, vs);
	glDetachShader(m_Shader, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);
	return true;
}

int32_t Shader::getUniformLocation(const std::string& name)
{
	if (m_UniformLocations.find(name) != m_UniformLocations.end())
	{
		return m_UniformLocations[name];
	}

	int32_t location = glGetUniformLocation(m_Shader, name.c_str());
	m_UniformLocations[name] = location;
	return location;
}

bool Shader::hasError(uint32_t status, uint32_t id, bool isProgram)
{
	int32_t result = {};
	if (isProgram)
	{
		glGetProgramiv(id, status, &result);
		if (!result)
		{
			int32_t length = {};
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

			char* infoLog = new char[length];
			glGetProgramInfoLog(id, length, &length, infoLog);

			LOG_ERROR("Failed to link shader program!");
			LOG_ERROR("Error Infomation: \n");
			LOG_TRACE("{}", infoLog);

			delete[] infoLog;
			glDeleteProgram(id);
			return true;
		}

		return false;
	}

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		int32_t length = {};
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

		GLchar* infoLog = new GLchar[length];
		glGetShaderInfoLog(id, length, &length, infoLog);

		LOG_ERROR("Error Infomation: \n");
		LOG_TRACE("{}", infoLog);

		delete[] infoLog;
		glDeleteShader(id);
		return true;
	}

	return false;
}

#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader
{
public:
	Shader(const std::string& vsPath, const std::string& fsPath);
	~Shader();

	void bind() const;
	void unbind() const;
public:
	//Shader(Shader&& shader) 
	//	: m_Shader{ shader.m_Shader }, m_UniformLocations{ std::move(shader.m_UniformLocations) }
	//{
	//}
public:
	void setUniform1f(const std::string& name, float value);
	void setUniform2f(const std::string& name, float v0, float v1);
	void setUniform3f(const std::string& name, float v0, float v1, float v2);
	void setUniformMat4(const std::string& name, glm::mat4 mat);
private:
	enum class ShaderType : int8_t
	{
		NONE = -1, VertexShader, FragmentShader
	};

	uint32_t compileShader(const ShaderType type, const std::string& source);
	bool linkProgram(const uint32_t vs, const uint32_t fs);
	int32_t getUniformLocation(const std::string& name);
	bool hasError(uint32_t status, uint32_t id, bool isProgram = false);
private:
	uint32_t m_Shader;

	std::unordered_map<std::string, int32_t> m_UniformLocations;
};


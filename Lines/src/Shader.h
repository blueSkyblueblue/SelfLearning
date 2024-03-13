#pragma once
#include <string>

class Shader
{
public:
	Shader(const std::string& vsPath, const std::string& fsPath);
	~Shader();

	void bind() const;
	void unbind() const;
private:
	enum class ShaderType : int8_t
	{
		NONE = -1, VertexShader, FragmentShader
	};

	uint32_t compileShader(const ShaderType type, const std::string& source);
	bool linkProgram(const uint32_t vs, const uint32_t fs);
private:
	uint32_t m_Shader;
};


#pragma once

#include <set>
#include <string>
#include <map>
#include <vector>

enum class ShaderDataType {
	None,
	Mat2, Mat3, Mat4,
	Vec2, Vec3, Vec4,
	Int, Float,
	Sampler2D
};

enum class ShaderDeclarationQualifier {
	None,
	In, Out, Uniform,
	Layout, Flat
};

// Order should be sorted in the same order the stage is called in the render pipeline, vertex first, fragment last
enum class ShaderStageType {
	None = 0,
	Vertex = 1,
	Fragment = 2
};

// Struct representation of a shader variable
struct ShaderVariable {

	std::string identifier;
	ShaderDataType type;

	bool flat = false;
	int layoutLocation = -1;
};

struct ShaderStage {

	ShaderStageType shaderType = ShaderStageType::None;
	std::string shaderSource;

	std::vector<ShaderVariable> inputs;
	std::vector<ShaderVariable> outputs;
	std::vector<ShaderVariable> uniforms;

	bool operator < (const ShaderStage& other) const { return static_cast<int>(shaderType) < static_cast<int>(other.shaderType); };
};

struct ShaderVertexBufferElement {

	std::string name;
	ShaderDataType type = ShaderDataType::None;
	uint32_t size = 0;
	uint32_t offset = 0;

	ShaderVertexBufferElement() = default;
	ShaderVertexBufferElement(std::string& pname, ShaderDataType ptype) : name(pname), type(ptype) {}

};

class ShaderVertexBufferLayout {

public:
	ShaderVertexBufferLayout() = default;
	~ShaderVertexBufferLayout() = default;

	const std::vector<ShaderVertexBufferElement>& getElements() const { return m_BufferElements; }
	const uint32_t getStride() const { return m_Stride; }

	std::vector<ShaderVertexBufferElement>::iterator begin() { return m_BufferElements.begin(); }
	std::vector<ShaderVertexBufferElement>::iterator end() { return m_BufferElements.end(); }
	std::vector<ShaderVertexBufferElement>::const_iterator begin() const { return m_BufferElements.begin(); }
	std::vector<ShaderVertexBufferElement>::const_iterator end() const { return m_BufferElements.end(); }

private:
	std::vector<ShaderVertexBufferElement> m_BufferElements;
	uint32_t m_Stride = 0;

	friend class ShaderBuilder;
};

class Shader {

public:
	Shader() = default;
	~Shader() {  }

	bool hasStage(ShaderStageType type) { return m_ShaderStages.find(type) != m_ShaderStages.end(); }
	ShaderStage& getStage(ShaderStageType type) { /*TODO: Assert*/ return m_ShaderStages.find(type)->second; }

	const ShaderVertexBufferLayout& getVertexBufferLayout() const { return m_VertexBufferLayout; }

	// TODO:
	// bool validateModel(Model model); // validates a given model to check if the model inputs match the shader inputs
	// VertexBufferLayout getVertexBufferLayout(); // gets the layout of the vertex buffer
	// UniformBuffer getUniformBufferLayout(); // gets the layout of the uniform buffer
	// FramebufferSpecification getFramebufferSpecification(); // gets the framebuffer specifications of this shader, what color attachments etc
	// probably more

private:
	std::map<ShaderStageType,ShaderStage> m_ShaderStages;
	ShaderVertexBufferLayout m_VertexBufferLayout;

	friend class ShaderBuilder;

};

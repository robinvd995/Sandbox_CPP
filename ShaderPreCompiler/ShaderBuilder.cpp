#include "ShaderBuilder.h"

#include <fstream>
#include <iostream>

#include <algorithm>

std::string WordBuilder::build() {
	std::string s = std::string(m_Buffer, m_Iterator);
	m_Iterator = 0;
	return s;
}

void SectionBuilder::append(char c) {
	bool illegal = false;
	for (int i = 0; i < SECTION_ILLEGAL_CHARACTER_SIZE; i++) {
		if (c == s_IllegalCharacters[i]) illegal = true;
	}

	if (!illegal) m_Buffer[m_Iterator++] = c;
}

void SectionBuilder::back(uint32_t amount) {
	if (m_Iterator < amount) m_Iterator = 0;
	else m_Iterator -= amount;
}

std::string SectionBuilder::build() {
	std::string s = std::string(m_Buffer, m_Iterator);
	m_Iterator = 0;
	return s;
}

bool BuilderDeclaration::hasQualifier(ShaderDeclarationQualifier qualifier) {
	for (ShaderDeclarationQualifier q : m_Qualifiers)
		if (q == qualifier) return true;
	return false;
}

std::string BuilderDeclaration::getQualifierParameter(ShaderDeclarationQualifier qualifier, const std::string& parameter) {
	auto pmap = m_Parameters.find(qualifier);
	if (pmap == m_Parameters.end()) return "";

	auto it = pmap->second.find(parameter);
	if (it == pmap->second.end()) return "";

	return it->second;
}

const char SectionBuilder::s_IllegalCharacters[SECTION_ILLEGAL_CHARACTER_SIZE] = {'\r', '\n', '\t'};

std::map<std::string, ShaderDeclarationQualifier> ShaderBuilder::s_DeclarationQualifiers = {
	{"in", ShaderDeclarationQualifier::In}, {"out", ShaderDeclarationQualifier::Out}, {"uniform", ShaderDeclarationQualifier::Uniform},
	{"layout", ShaderDeclarationQualifier::Layout}, {"flat", ShaderDeclarationQualifier::Flat}
};

std::map<std::string, ShaderDataType> ShaderBuilder::s_DeclarationTypes = {
	{"mat2", ShaderDataType::Mat2}, {"mat3", ShaderDataType::Mat3}, {"mat4", ShaderDataType::Mat4},
	{"vec2", ShaderDataType::Vec2}, {"vec3", ShaderDataType::Vec3}, {"vec4", ShaderDataType::Vec4},
	{"int", ShaderDataType::Int}, {"float", ShaderDataType::Float},
	{"sampler2D", ShaderDataType::Sampler2D}
};

std::map<std::string, ShaderSection> ShaderBuilder::s_ShaderSections = {
	{"glsl_common", ShaderSection::Glsl_Common},
	{"vertex", ShaderSection::Vertex}, {"fragment", ShaderSection::Fragment}
};

void ShaderBuilder::pushWord() {
	if (m_WordBuilder.size() > 0) {
		std::string word = m_WordBuilder.build();

		switch (getState()) {
			case BuilderState::Normal:
				m_CurrentLine.keywords.push_back(word);
			break;

			case BuilderState::Parameter:
				switch (m_ParameterState) 	{
					case 0: m_ParameterId = word; break;
					case 1: m_ParameterValue = word; break;
				}

			break;
		}
	}
}

void ShaderBuilder::pushScope() {

	if (m_CurrentLine.keywords.empty()) return;

	auto it = s_ShaderSections.find(m_CurrentLine.keywords.back().identifier);
	if(it != s_ShaderSections.end()) {
		m_CurrentSection = it->second;
		m_SectionStartScopeDepth = m_ScopeDepth;
		m_SectionDeclarations.insert({ it->second, std::vector<BuilderDeclaration>() });
	}

	finishLine();

	m_ScopeDepth++;
}

void ShaderBuilder::popScope() {
	m_ScopeDepth--;

	pushWord();
	finishLine();

	if (m_ScopeDepth == m_SectionStartScopeDepth) {

		m_SectionBuilder.back();
		std::string sectionSource = m_SectionBuilder.build();
		m_SectionSource.insert({ m_CurrentSection, sectionSource });
		m_CurrentSection = ShaderSection::None;
	}
}

void ShaderBuilder::startParameters() {
	m_ParameterState = 0;
	m_ParameterId = "";
	m_ParameterValue = "";
	pushState(BuilderState::Parameter);
}

void ShaderBuilder::stopParameters() {
	m_ParameterState = 0;
	popState();
}

void ShaderBuilder::finishLine() {
	if (m_CurrentLine.keywords.size() == 0) return;
	
	if(m_CurrentSection != ShaderSection::None && m_ScopeDepth == 1) {
		
		BuilderDeclaration declaration{};
		bool isDeclaration = false;

		for (auto itt = m_CurrentLine.keywords.begin(); itt != m_CurrentLine.keywords.end(); itt++) {
		//for (int i = 0; i < m_CurrentLine.keywords.size(); i++) {
			//BuilderLineKeyword& keyword = m_CurrentLine.keywords[i];
			auto it = s_DeclarationTypes.find(itt->identifier);
			if (it != s_DeclarationTypes.end()) {
				isDeclaration = true;
				declaration.m_DataType = it->second;
				m_CurrentLine.keywords.erase(itt);
				break;
			}
		}

		if(isDeclaration) {
			for (int i = 0; i < m_CurrentLine.keywords.size(); i++) {
				BuilderLineKeyword& keyword = m_CurrentLine.keywords[i];
				auto it = s_DeclarationQualifiers.find(keyword.identifier);

				if(it == s_DeclarationQualifiers.end()) {
					if (declaration.m_Identifier.empty()) {
						declaration.m_Identifier = keyword.identifier;

						// TODO: if identifier is parameterized 'has a ( after the identifier' it is a function declaration
					}
					else
						std::cout << "Found duplicate declaration identifier!" << std::endl;
				}
				else {
					declaration.m_Qualifiers.push_back(it->second);
					
					if(!keyword.parameterMap.empty()) {
						declaration.m_Parameters.insert({ it->second, keyword.parameterMap });
					}
				}
			}

		}

		if (!declaration.m_Qualifiers.empty()) {
			m_SectionDeclarations[m_CurrentSection].push_back(declaration);
		}

	}

	/*BuilderDeclaration declaration{};
	bool foundQualifier = false;

	for(int i = 0; i < m_CurrentLine.keywords.size(); i++) {
		BuilderLineKeyword& keyword = m_CurrentLine.keywords[i];

		auto it = s_DeclarationQualifiers.find(keyword.identifier);
		if(it != s_DeclarationQualifiers.end()) {
			declaration.qualifiers.push_back(it->second);
			foundQualifier = true;
		}
		else if (foundQualifier) {
			auto it = s_DeclarationTypes.find(keyword.identifier);
			if(it != s_DeclarationTypes.end()) {
				if (declaration.dataType == ShaderDataType::None)
					declaration.dataType = it->second;
				else
					std::cout << "Found duplicate type identifier!" << std::endl;
			}
			else{
				if (declaration.identifier.empty())
					declaration.identifier = keyword.identifier;
				else
					std::cout << "Found duplicate declaration identifier!" << std::endl;
			}
		}
	}

	if(!declaration.qualifiers.empty()) {
		m_SectionDeclarations[m_CurrentSection].push_back(declaration);
	}*/

	m_CurrentLine = BuilderLine{};
}

void ShaderBuilder::pushOperatorAssign() {
	switch (getState()) {
		case BuilderState::Normal: {
			std::string s = "=";
			m_CurrentLine.keywords.push_back(s);
		}break;
		case BuilderState::Parameter:
			m_ParameterState = 1;
			break;
	}
}

void ShaderBuilder::pushSeperator() {
	switch (getState()) {
		case BuilderState::Normal: break;
		case BuilderState::Parameter:
			auto& pmap = m_CurrentLine.keywords[m_CurrentLine.keywords.size() - 1].parameterMap;
			pmap.insert({ m_ParameterId, m_ParameterValue });
			m_ParameterState = 0;
		break;
	}
}

void ShaderBuilder::parseFile(const std::string& filepath) {

	m_BuilderState.push_back(BuilderState::Normal);

	std::ifstream input(filepath);

	for (std::string line; getline(input, line); ) {

		m_PrevChar = ' ';
		m_IsComment = false;

		for (int i = 0; i < line.size(); i++) {
			char c = line[i];

			if (m_CurrentSection != ShaderSection::None) m_SectionBuilder.append(c);

			if (m_IsComment) continue;

			switch (c) {
				case '{': pushScope(); break;
				case '}': finishLine(); popScope(); break;

				case '(': pushWord(); startParameters(); break;
				case ')': pushWord(); pushSeperator(); stopParameters(); break;

				case ',': pushSeperator(); break;

				case ';': pushWord(); finishLine(); break;

				case '=': pushOperatorAssign(); break;

				case '/': if (line[i + 1] == '/') { m_IsComment = true; } break;

				case ' ': case '\t': case '\n': case '\r':
					pushWord();
					break;

				default: m_WordBuilder.append(c); break;
			}

			m_PrevChar = line[i];
		}
	}

}

void ShaderBuilder::parseStages(Shader& shader, uint32_t* error) {

	static const size_t shaderStageSize = 2;
	static const ShaderStageType shaderStages[] = { ShaderStageType::Vertex, ShaderStageType::Fragment };
	static const ShaderSection shaderSections[] = { ShaderSection::Vertex, ShaderSection::Fragment };

	for (int i = 0; i < shaderStageSize; i++) {

		ShaderStageType stagetype = shaderStages[i];
		ShaderSection sectiontype = shaderSections[i];

		auto it_source = m_SectionSource.find(sectiontype);

		if (it_source != m_SectionSource.end()) {

			ShaderStage stage;
			stage.shaderSource = it_source->second;

			auto it_declarations = m_SectionDeclarations.find(sectiontype);
			if (it_declarations != m_SectionDeclarations.end()) {

				for (BuilderDeclaration declaration : it_declarations->second) {

					ShaderVariable variable;
					variable.identifier = declaration.m_Identifier;
					variable.type = declaration.m_DataType;
					variable.flat = declaration.hasQualifier(ShaderDeclarationQualifier::Flat);

					std::string locationStr = declaration.getQualifierParameter(ShaderDeclarationQualifier::Layout, "location");
					if (!locationStr.empty()) { variable.layoutLocation = std::atoi(locationStr.c_str()); }

					if (declaration.hasQualifier(ShaderDeclarationQualifier::In))              stage.inputs.push_back(variable);
					else if (declaration.hasQualifier(ShaderDeclarationQualifier::Out))        stage.outputs.push_back(variable);
					else if (declaration.hasQualifier(ShaderDeclarationQualifier::Uniform))    stage.uniforms.push_back(variable);
				}
			}

			shader.m_ShaderStages.insert({ stagetype, stage });

		}
	}

	// Shader stage validation
	if (!shader.hasStage(ShaderStageType::Vertex)) { *error = 1; return; }
	if (shader.getStage(ShaderStageType::Vertex).inputs.empty()) { *error = 2; return; }

	if (!shader.hasStage(ShaderStageType::Fragment)) { *error = 3; return; }
	if (shader.getStage(ShaderStageType::Fragment).outputs.empty()) { *error = 4; return; }

	// No errors
	*error = 0;
}

void ShaderBuilder::setVertexBufferLayout(Shader& shader, uint32_t* error) {

	// TODO: Every input needs to set its layout location, if not an error

	ShaderStage& stage = shader.getStage(ShaderStageType::Vertex);

	std::sort(stage.inputs.begin(), stage.inputs.end(), [](const ShaderVariable& var0, const ShaderVariable& var1) -> bool {
		if (var0.layoutLocation == -1) return 0;
		if (var1.layoutLocation == -1) return 1;
		return var0.layoutLocation < var1.layoutLocation;
	});

	shader.m_VertexBufferLayout.m_BufferElements.resize(stage.inputs.size());

	uint32_t offset = 0;

	int i = 0;
	for(auto& input : stage.inputs) {
		ShaderVertexBufferElement element;
		element.name = input.identifier;
		element.offset = offset;
		element.size = Util::getShaderDataTypeSize(input.type);
		element.type = input.type;
		shader.m_VertexBufferLayout.m_BufferElements[i] = element;

		offset += element.size;
		i++;
	}

	shader.m_VertexBufferLayout.m_Stride = offset;
	*error = 0;
}

Shader ShaderBuilder::build(const std::string& filepath) {

	// Create an new instance of a shader
	Shader shader;
	uint32_t errorCode = 0;

	// Parse file to get the variables and the sources
	parseFile(filepath);

	// Parse all the shader stages into the shader
	parseStages(shader, &errorCode);
	if(errorCode > 0) {
		std::cout << "Error with code '" << errorCode << "' occured while parsing stages!" << std::endl;
		return Shader();
	}

	// Determine the vertex buffer layout
	setVertexBufferLayout(shader, &errorCode);
	if (errorCode > 0) {
		std::cout << "Error with code '" << errorCode << "' occured while setting the vertex buffer layout!" << std::endl;
		return Shader();
	}

	return shader;
}

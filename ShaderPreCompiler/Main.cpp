#include <iostream>

#include "ShaderBuilder.h"

int main()
{
    ShaderBuilder sb;
    Shader s = sb.build("assets/Test.glsl");

    ShaderStage& vertex = s.getStage(ShaderStageType::Vertex);
    for(auto& uniform : vertex.uniforms) {
        std::cout << uniform.identifier << std::endl;
    }

    return 0;
}


// 3D Shader

glsl_common {
	#version 330 core
}

vertex {
	
	layout(location = 0) in vec3 a_Position;
	layout(location = 1) in vec2 a_TexCoord;
	
	uniform mat4 u_Projection;
	uniform mat4 u_View;
	uniform mat4 u_Transform;
	
	uniform int u_Tag;
	uniform int u_Selected;
	
	out vec2 v_TexCoord;
	flat out int v_Tag;
	flat out int v_Selected;
	
	void main() {
		v_Selected = u_Selected;
		v_Tag = u_Tag;
		v_TexCoord = a_TexCoord;
	
		gl_Position = u_Projection * u_View * u_Transform * vec4(a_Position, 1.0);
	}
}

fragment {

	layout(location = 0) out vec4 frag_color;
	layout(location = 1) out int entity_id;
	layout(location = 2) out vec4 selected_color;
	
	in vec2 v_TexCoord;
	flat in int v_Tag;
	flat in int v_Selected;
	
	uniform sampler2D u_Texture;
	
	void main() {
		vec4 texColor = texture(u_Texture, v_TexCoord);
		frag_color = texColor;
		entity_id = v_Tag;
		if(v_Selected == 1) {
			selected_color = vec4(1.0f);
		}
		else {
			selected_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}
	}
}

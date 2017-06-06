// Fragment shader
#version 150

in vec4 v_position;

out vec4 frag_color;

in vec3 N; // normal
in vec3 L; // light direction
in vec3 V; // view direction

vec3 ambient_color;
vec3 diffuse_color;
vec3 specular_color;

in vec3 shader_switch;

// GUI-related
in float toonA;
in float toonB;
uniform vec4 u_stroke_color;

uniform sampler2D u_texture_0;
uniform sampler2D u_texture_1;
uniform sampler2D u_texture_2;
uniform sampler2D u_texture_3;
uniform sampler2D u_texture_4;
uniform sampler2D u_texture_5;

void main()
{
	// Ambient
	float ambient = 0.3;
	ambient_color = vec3(0.1, 0.0, 0.0);

	// Diffuse
	float diffuse = max(0.0, dot(N, L));
	diffuse_color = vec3(0.7, 0.7, 0.7) * diffuse;

	// Specular (Blinn-Phong)
	vec3 halfwayDir = normalize(L+V);
	float specular_power = 64;
	float normalization = (8.0 + specular_power) / 8.0;
	float specular = pow(max(dot(N, halfwayDir), 0.0), specular_power) * normalization;
	specular_color = 0.04*vec3(1.0, 1.0, 1.0) * specular;

	vec3 result = ambient_color*shader_switch.x + diffuse_color*shader_switch.y + specular_color*shader_switch.z;

	// Gamma correction
	float gamma = 2.2;
	result.rgb = pow(result.rgb, vec3(1.0/gamma));

	// UV-mapping
	vec4 norm_pos = normalize(v_position);
	float u = 0.5 + atan(norm_pos.z, norm_pos.x)/(2*3.14);
	float v = 0.5 - asin(norm_pos.y)/3.14;

	// Scale texture
	v*=8;
	u*=8;
	vec2 uv = vec2(u,v);

	// cross-hatching shader
	float step = 1.0/6.0;
	float shade = ambient*shader_switch.y + 0.6*diffuse*shader_switch.x + specular*shader_switch.z;

	if(shade < step){
		frag_color = mix(texture(u_texture_5, uv), texture(u_texture_4, uv), 6*(shade));
	}else if(shade < step*2){
		frag_color = mix(texture(u_texture_4, uv), texture(u_texture_3, uv), 6*(shade-step));
	}else if(shade < step*3){
		frag_color = mix(texture(u_texture_3, uv), texture(u_texture_2, uv), 6*(shade-2*step));
	}else if(shade < step*4){
		frag_color = mix(texture(u_texture_2, uv), texture(u_texture_1, uv), 6*(shade-3*step));
	}else if(shade < step*5){
		frag_color = mix(texture(u_texture_1, uv), texture(u_texture_0, uv), 6*(shade-4*step));
	}else{
		frag_color = mix(texture(u_texture_0, uv), vec4(1.0), 6*(shade-5*step));
	}
	
	if(frag_color.r < 0.93){
		frag_color = mix(frag_color, vec4(u_stroke_color.r, u_stroke_color.g, u_stroke_color.b, 1.0), 0.5);
	}

	// Cel shading outline
	if (dot(V, N) < mix(toonA, toonB, max(0.0, dot(N,L)))){
		frag_color = vec4(1.0,1.0,1.0,1.0) * vec4(0.0,0.0,0.0,1.0);
	}
}
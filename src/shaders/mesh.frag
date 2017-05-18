// Fragment shader
#version 150

in vec3 v_normal;
in vec3 v_color;

out vec4 frag_color;

in vec3 N; // normal
in vec3 L; // light direction
in vec3 V; // view direction

in vec3 fragPos;
vec3 viewPos = vec3(0.0, 0.0, 8.0);

vec3 ambient_color;
vec3 diffuse_color;
vec3 specular_color;

in vec3 shader_switch;
in float showNormalsAsRgb;
in float useGammaCorrection;

uniform samplerCube u_cubemap;

void main()
{
    //vec3 N = normalize(v_normal);
    //frag_color = vec4(0.5 * N + 0.5, 1.0);



	
	// Ambient
	ambient_color = vec3(0.1, 0.0, 0.0);

	// Diffuse
	float diffuse = max(0.0, dot(N, L));
	diffuse_color = vec3(0.7, 0.7, 0.7) * diffuse;

	// Specular (Phong)
	/*float specularStrength = 1.5f;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-L, N);
	float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	specular_color = vec3(1.0, 1.0, 1.0) * specular * specularStrength;*/

	// Specular (Blinn-Phong)
	vec3 halfwayDir = normalize(L+V);
	float specular_power = 64;
	float normalization = (8.0 + specular_power) / 8.0;
	float specular = pow(max(dot(N, halfwayDir), 0.0), specular_power) * normalization;
	specular_color = 0.04*vec3(1.0, 1.0, 1.0) * specular;

	vec3 result = ambient_color*shader_switch.x + diffuse_color*shader_switch.y + specular_color*shader_switch.z;
	//result *= v_color;

	// Gamma correction
	if(useGammaCorrection > 0.5){
		float gamma = 2.2;
		result.rgb = pow(result.rgb, vec3(1.0/gamma));
	}

	if(showNormalsAsRgb < 0.5){
		frag_color = vec4(result, 1.0);
	} else {
		frag_color = vec4(N,1.0);
	}

	//frag_color = vec4(1.0);
	// Cel shading outline
	// Bra parametrar för imgui
	if (dot(V, N) < mix(0.7f, 0.3f, max(0.0, dot(N,L)))){
		frag_color = vec4(1.0,1.0,1.0,1.0) * vec4(0.0,0.0,0.0,1.0);
	}
}

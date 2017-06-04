// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal;
uniform mat4 u_mvp;


uniform mat4 u_mv;
uniform mat4 u_v;
vec3 u_light_position;
uniform vec3 u_light_color;
out vec3 v_color;

out vec3 N;
out vec3 L;
out vec3 V;

uniform vec3 u_shader_switch;
out vec3 shader_switch;
uniform float u_showNormalsAsRgb;
out float showNormalsAsRgb;
uniform float u_useGammaCorrection;
out float useGammaCorrection;

//gui-related
uniform float u_toonA;
uniform float u_toonB;
out float toonA;
out float toonB;

//out vec3 viewPos;
out vec3 fragPos;

void main()
{
    //gui-related
	 toonA = u_toonA;
	 toonB = u_toonB;

    v_normal = a_normal;

	// Cel shading outline
	//float offset = 0.0f;
	//vec4 cPos = vec4(a_position + a_normal * offset);
	//gl_Position = u_mvp * cPos;

	gl_Position = u_mvp * a_position;

	//viewPos = u_v;

	shader_switch = u_shader_switch;
	showNormalsAsRgb = u_showNormalsAsRgb;
	useGammaCorrection = u_useGammaCorrection;

	// Position of the light source
	u_light_position = vec3(8.0, 12.0, 5.0);

	// Transform the vertex position to view space (eye coordinates)
	vec3 position_eye = vec3(u_mv * a_position);
	fragPos = position_eye;

	// Calculate the view-space normal
	N = normalize(mat3(u_mv) * a_normal);

	// Calculate the view-space light direction
	L = normalize(u_light_position - position_eye);

	// Calculate the view vector
	V = normalize(-position_eye);

	// Calculate the diffuse (Lambertian) reflection term
	//float diffuse = max(0.0, dot(N, L));

	// Set the surface color
	v_color = vec3(0.7, 0.7, 0.7);
}
// Vertex shader
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;

uniform mat4 u_mvp;
out vec4 gl_Position;
out vec4 v_position;

uniform mat4 u_mv;
uniform mat4 u_v;
vec3 u_light_position;
uniform vec3 u_light_color;

out vec3 N;
out vec3 L;
out vec3 V;

uniform vec3 u_shader_switch;
out vec3 shader_switch;

//gui-related
uniform float u_toonA;
uniform float u_toonB;
uniform float u_crossH;
out float toonA;
out float toonB;
out float crossH;

void main()
{
    //gui-related
	toonA = u_toonA;
	toonB = u_toonB;
	crossH = u_crossH;

	gl_Position = u_mvp * a_position;
	v_position = a_position;

	shader_switch = u_shader_switch;

	// Position of the light source
	u_light_position = vec3(8.0, 12.0, 5.0);

	// Transform the vertex position to view space (eye coordinates)
	vec3 position_eye = vec3(u_mv * a_position);

	// Calculate the view-space normal
	N = normalize(mat3(u_mv) * a_normal);

	// Calculate the view-space light direction
	L = normalize(u_light_position - position_eye);

	// Calculate the view vector
	V = normalize(-position_eye);
}
// Fragment shader
#version 150

in vec3 v_normal;
in vec3 v_color;
in vec4 v_position;

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

//gui-related
in float toonA;
in float toonB;
in float crossH;

//uniform samplerCube u_cubemap;

uniform sampler2D u_texture;

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

	frag_color = vec4(1.0,0.1,0.3,1.0);
	


	//tonal shading
	if(diffuse < 0.2){
	}else if(diffuse < 0.2){
		frag_color = vec4(0.5, 0.1, 0.0 ,1.0);
	}else if(diffuse < 0.4){
		frag_color = vec4(0.5, 1.0, 0.3 ,1.0);
	}else if(diffuse < 0.6){
		frag_color = vec4(0.7, 0.7, 0 ,1.0);
	}else if(diffuse < 0.8){
		frag_color = vec4(0, 0, 0.9 ,1.0);
	}
	
	// Cel shading outline
	// Bra parametrar för imgui
	if (dot(V, N) < mix(toonA, toonB, max(0.0, dot(N,L)))){
		frag_color = vec4(1.0,1.0,1.0,1.0) * vec4(0.0,0.0,0.0,1.0);
	}
	
	
	// cross-hatching shader

	

if ( crossH +length(result) < 0.85) /*length(result)*/
{
   // hatch from left top corner to right bottom
   if (mod(gl_FragCoord.x + gl_FragCoord.y, 10.0) == 0.0) 
   {
      frag_color = vec4(0.0, 0.0, 0.0, 1.0);
   }
}

if ( crossH +length(result)< 0.75) /*length(result)*/
{
   // hatch from right top corner to left boottom
   if (mod(gl_FragCoord.x - gl_FragCoord.y, 10.0) == 0.0) 
   {
      frag_color = vec4(0.0, 0.0, 0.0, 1.0);
   }
}

if ( crossH +length(result)< 0.5) /*length(result)*/
{
   // hatch from left top to right bottom
   if (mod(gl_FragCoord.x + gl_FragCoord.y - 5.0, 10.0) == 0.0) 
   {
      frag_color = vec4(1.0, 1.0, 1.0, 1.0);
   }
}

if ( crossH +length(result)< 0.25) /*length(result)*/
{
   // hatch from right top corner to left bottom
   if (mod(gl_FragCoord.x - gl_FragCoord.y - 5.0, 10.0) == 0.0) 
   {
      frag_color = vec4(0.0, 0.0, 0.0, 1.0);
   }
}

	//frag_color.rgb = v_color.rgb;
			frag_color = texture(u_texture, v_position.xy);

}
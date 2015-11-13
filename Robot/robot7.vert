
attribute vec4 position; 
attribute vec4 color;
attribute vec4 normal; 

varying vec4 pcolor;

uniform mat4 local2clip;
uniform mat4 local2eye;
uniform mat4 normal_local2eye; 

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;
uniform vec4 light_pos;

uniform vec4 mat_ambient;
uniform vec4 mat_diffuse;
uniform vec4 mat_specular;
uniform float mat_shine; 

//
// this shader just pass the vertex position and color along, doesn't actually do anything 
// Note that this means the vertex position is assumed to be in clip space already 
//
void main(){
     
          gl_Position = local2clip * position;
     // pcolor = vec4(0,0,1,1);
     
//     pcolor = color; 
//     pcolor = light_specular;
//     pcolor = mat_specular; 
//     pcolor = mat_diffuse;
//     vec3 c = normalize(vec3(normal[0], normal[1], normal[2]));
//     vec3 c = normalize(vec3(position[0], position[1], position[2]));
       vec3 c = normalize(vec3(gl_Position[0], gl_Position[1], gl_Position[2]*0.1)); 
       vec4 a = postion; 
       pcolor = vec4(c[0], c[1], c[2], 1.0); 

}

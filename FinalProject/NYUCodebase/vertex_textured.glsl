attribute vec4 position;
attribute vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

varying vec2 texCoordVar;
varying vec4 v_position;

void main()
{
    
	vec4 p =  modelMatrix  * position;
    texCoordVar = texCoord;
    v_position = p;
	gl_Position = projectionMatrix * viewMatrix *  p;
}

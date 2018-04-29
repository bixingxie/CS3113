attribute vec4 position;
attribute vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

varying vec2 texCoordVar;
varying vec4 v_position;

void main()
{
    
	vec4 p =  projectionMatrix * modelMatrix  * position;
    texCoordVar = texCoord;
    v_position = p;
	gl_Position = viewMatrix *  p;
}

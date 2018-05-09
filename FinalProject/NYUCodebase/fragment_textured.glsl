
uniform sampler2D diffuse;
uniform vec2 lightPos[4];
uniform float lightIntensity;

varying vec2 texCoordVar;
varying vec2 varPosition;

vec3 point_light_col = vec3(0.999, 0.999, 0.999);
//float point_light_intensity = 3.0;

float attenuate(float dist, float a, float b){
    return 1.0 / (1.0 + a*dist + b*dist*dist);
}

void main() {
//Lighting
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 brightness = vec3(0.0, 0.0, 0.0);

    for(int i=0; i<4; i++){
        brightness += attenuate(distance(lightPos[i], varPosition)/lightIntensity, 1.0, 2.0) * lightColor;
    }

    vec4 textureColor = texture2D(diffuse, texCoordVar);
    gl_FragColor.xyz = textureColor.xyz * brightness;
    gl_FragColor.a = textureColor.a;

//No Lighting
//    gl_FragColor = texture2D(diffuse, texCoordVar);
}

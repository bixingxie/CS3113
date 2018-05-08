
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
//
//    vec4 frag_color = texture2D(diffuse, texCoordVar);
//    if(frag_color.a < 1.0)
//        discard;
//
//    float distance = distance(lightPos, varPosition);
//    float diffuse = 0.0;
//
//    if (distance <= lightIntensity)
//        diffuse =  1.0 - abs(distance / lightIntensity);
//
//    gl_FragColor = vec4(min(frag_color.rgb * ((point_light_col * diffuse)), frag_color.rgb), 1.0);
//    
//        vec3 brightness = attenuate(distance(lightPos, varPosition)/lightIntensity, 1.0, 2.0) * lightColor;
//

    vec3 lightColor = vec3(1.0, 1.0, 1.0);
//
//
//
    vec3 brightness = vec3(0.0, 0.0, 0.0);

    for(int i=0; i<4; i++){
        brightness += attenuate(distance(lightPos[i], varPosition)/lightIntensity, 1.0, 2.0) * lightColor;
    }

    vec4 textureColor = texture2D(diffuse, texCoordVar);
    gl_FragColor.xyz = textureColor.xyz * brightness;
    gl_FragColor.a = textureColor.a;

//    gl_FragColor = texture2D(diffuse, texCoordVar);
}


uniform sampler2D diffuse;
uniform vec4 lightPos;
uniform float lightIntensity;
varying vec2 texCoordVar;
varying vec4 v_position;

vec3 point_light_col = vec3(0.999, 0.999, 0.999);
//float point_light_intensity = 3.0;


void main() {
//
//    vec4 frag_color = texture2D(diffuse, texCoordVar);
//    if(frag_color.a < 1.0)
//        discard;
//    
//    float distance = distance(lightPos.xy, v_position.xy);
//    float diffuse = 0.0;
//
//    if (distance <= lightIntensity)
//        diffuse =  1.0 - abs(distance / lightIntensity);
//
//    gl_FragColor = vec4(min(frag_color.rgb * ((point_light_col * diffuse)), frag_color.rgb), 1.0);

    gl_FragColor = texture2D(diffuse, texCoordVar);
}

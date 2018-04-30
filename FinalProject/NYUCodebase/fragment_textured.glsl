
uniform sampler2D diffuse;
uniform vec4 playerPosition;
varying vec2 texCoordVar;
varying vec4 v_position;


vec2 point_light_pos = vec2(-0.4, -0.3);
vec3 point_light_col = vec3(0.999, 0.999, 0.999);
float point_light_intensity = 0.3;


void main() {
    
//    vec4 frag_color = texture2D(diffuse, texCoordVar);
//    if(frag_color.a < 1.0)
//        discard;
//
//    float distance = distance(point_light_pos, v_position.xy);
//    float distance = distance(playerPosition.xy, v_position.xy);
//    float diffuse = 0.0;
//
//    if (distance <= point_light_intensity)
//        diffuse =  1.0 - abs(distance / point_light_intensity);
//
//    gl_FragColor = vec4(min(frag_color.rgb * ((point_light_col * diffuse)+0.15), frag_color.rgb), 1.0);


////    vec4 frag_color = texture2D(diffuse, texCoordVar);
////
////    float distance = distance(point_light_pos, v_position.xy);
////
////
////
////    frag_color.a = lerp(0, 1, 1-distance/8.15f);
////
//    gl_FragColor = frag_color;
//
    gl_FragColor = texture2D(diffuse, texCoordVar);
}

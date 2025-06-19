#version 330 core
in vec2 TexCoord;
in float EyeDist;

out vec4 FragColor;

uniform sampler2D texSampler;
uniform vec3 fogColor;
uniform float fogStart, fogEnd;
uniform bool enableFog;

void main(){
    vec4 texColor = texture(texSampler, TexCoord);
    if (texColor.a < 0.1)
        discard;

    if (enableFog) {
        float fogFactor = clamp((EyeDist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
        FragColor = mix(texColor, vec4(fogColor, 1.0), fogFactor);
    } else {
        FragColor = texColor;
    }
}

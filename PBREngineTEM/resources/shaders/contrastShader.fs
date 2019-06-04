#version 430 core

in vec2 TexCoords;

out vec4 out_Colour;

uniform sampler2D screenTexture;

uniform float brightness = 1.2;
uniform float contrast = 0.1;
uniform float saturation = 1.65;

void main(void)
{

 out_Colour = texture(screenTexture, TexCoords);
 
 // calculate saturation
 vec3 luminanceWeights = vec3(0.299, 0.587, 0.114);
 float luminance = dot(out_Colour.rgb, luminanceWeights);
 out_Colour = mix(vec4(luminance), out_Colour, saturation);
 
 // calculate contrast
 out_Colour.rgb = (out_Colour.rgb - 0.5) * (1.0 + contrast) + 0.5;
 
 // calculate brightness
 out_Colour.rgb *= brightness;

 //out_Colour = vec4(1.0 - out_Colour.r, 1.0 - out_Colour.g, 1.0 - out_Colour.b, 1.0);
}
/// VERTEX
#version 330 core

layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

int main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    vTexCoord = aTexCoord;
}

/// FRAGMENT
#version 330 core

in vec2 vTexCoord;
uniform sampler2D texture1;
out FragColor;

void main()
{
    FragColor = texture(texture1, vTexCoord);
}

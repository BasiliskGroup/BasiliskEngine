#version 330 core

layout (location = 0) out vec4 gSolid;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gUV;
layout (location = 3) out vec4 gNormal;
layout (location = 4) out vec4 gTangent;
layout (location = 5) out vec4 gBitangent;


in vec2 uv;
in vec3 position;
in mat3 TBN;


void main()
{ 
    gSolid      = vec4(1.0);
    gPosition   = vec4(position, 1.0);
    gUV         = vec4(uv, 0.0, 1.0);
    gNormal     = vec4(TBN[2], 1.0);
    gTangent    = vec4(TBN[0], 1.0);
    gBitangent  = vec4(TBN[1], 1.0);
}
#version 300 es

precision mediump float;

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec2 vUV;


out vec3 atrPosition;
out vec3 atrNormal;
out vec3 atrTangent;
out vec3 atrBitangent;
out vec2 atrUV;
out mat3 atrTbn;

uniform mat4 _Model;
uniform mat4 _ViewProjection;

void main()
{
	vec3 t = normalize(mat3(_Model) * vTangent);
	vec3 b = normalize(mat3(_Model) * vBitangent);
	vec3 n = normalize(mat3(_Model) * vNormal);
	mat3 tbn = transpose(mat3(t, b, n));

	atrPosition = mat3(_Model) * vPos;
	atrNormal = n;
	atrTangent = t;
	atrBitangent = b;
	atrUV = vUV;
	atrTbn = tbn;

	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}
#version 330 core

in vec4 vertexOCS;
in vec3 normalOCS;
in vec3 fmatamb;
in vec3 fmatdiff;
in vec3 fmatspec;
in float fmatshin;

uniform vec4 lightPos;
uniform vec3 lightCol;

out vec4 FragColor;

vec3 ambientLight = vec3(0.2, 0.2, 0.2);

vec3 Lambert (vec3 NormOCS, vec3 L)
{
  // We assume that vectors are normalized

  // Color initialization with the ambient color
  vec3 resultCol = ambientLight * fmatamb;

  // Add the diffuse component
  if (dot (L, NormOCS) > 0)
    resultCol += lightCol * fmatdiff * dot (L, NormOCS);

  return (resultCol);
}

vec3 Phong (vec3 NormOCS, vec3 L, vec4 vertOCS)
{
  // We assume that vectors are normalized

  // Color initialization with the Lambert color
  vec3 resultCol = Lambert (NormOCS, L);

  // R and V computation
  if (dot(NormOCS, L) < 0)
    // There is no specular component
    return resultCol;

  vec3 R = reflect(-L, NormOCS); // equivalent to: normalize (2.0*dot(NormOCS,L)*NormOCS - L);
  vec3 V = normalize(-vertOCS.xyz);

  if ((dot(R, V) < 0) || (fmatshin == 0))
    // There is no specular component
    return resultCol;

  // We add the specular component
  float shine = pow(max(0.0, dot(R, V)), fmatshin);
  return (resultCol + fmatspec * lightCol * shine);
}

void main()
{
  vec3 L = normalize(lightPos.xyz - vec3(vertexOCS).xyz);
  FragColor = vec4(Phong(normalize(normalOCS), L, vertexOCS), 1);
}

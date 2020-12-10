#version 150

in vec3 position;
in vec3 leftVertices, topVertices, rightVertices, bottomVertices;
in vec4 color;
out vec4 col;

uniform int mode;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  if(mode == 0) {
  	gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  	col = color;
  }
  else if (mode == 1) {
  	vec3 tempPosition;
  	tempPosition.x = position.x;
  	tempPosition.y = (leftVertices.y + topVertices.y + rightVertices.y + bottomVertices.y) / 4.0f;
  	tempPosition.z = position.z;
  	
  float eps1 = 0.0000012;
	float eps2 = 0.00004;
  	vec4 outputColor = max(color, vec4(eps1)) / max(position.y, eps2) * tempPosition.y;
  	gl_Position = projectionMatrix * modelViewMatrix * vec4(tempPosition, 1.0f);
  	col = outputColor;
  }
}
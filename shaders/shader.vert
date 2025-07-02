#version 450

// Hardcoded triangle positions (clip space)
vec2 positions[3] = vec2[](
	vec2(0.0, -0.5),	// Vertex 0: bottom
	vec2(0.5, 0.5),		// Vertex 1: top right
	vec2(-0.5, 0.5)		// Vertex 2: top left
);

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
#version 330 core

// Input vertex attributes - corresponding to your Vertex structure
layout(location = 0) in vec3 aPosition;  // position
layout(location = 1) in vec3 aNormal;    // normal
layout(location = 2) in vec2 aTexCoord;  // tex_coord
layout(location = 3) in vec3 aTangent;   // tangent
layout(location = 4) in vec3 aBitangent; // bitangent
layout(location = 5) in vec4 aColor;     // color

// Output to fragment shader
out vec3 FragPos;           // World space position
out vec3 Normal;            // World space normal
out vec2 TexCoord;          // UV coordinates
out vec3 Tangent;           // World space tangent
out vec3 Bitangent;         // World space bitangent
out vec4 VertexColor;       // Vertex color
out mat3 TBN;               // Tangent space matrix
out vec4 FragPosLightSpace; // Position in light space for shadow mapping

// Uniform variables
uniform mat4 uModel;            // Model matrix
uniform mat4 uView;             // View matrix
uniform mat4 uProjection;       // Projection matrix
uniform mat3 uNormalMatrix;     // Normal matrix (for normal transformation)
uniform mat4 uLightSpaceMatrix; // Light space matrix for shadow mapping

void main()
{
    // Calculate world space position
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    FragPos       = worldPos.xyz;

    // Calculate world space normal
    Normal = normalize(uNormalMatrix * aNormal);

    // Calculate world space tangent space vectors
    Tangent   = normalize(uNormalMatrix * aTangent);
    Bitangent = normalize(uNormalMatrix * aBitangent);

    // Build TBN matrix for normal mapping
    TBN = mat3(Tangent, Bitangent, Normal);

    // Pass UV coordinates and vertex color
    TexCoord    = aTexCoord;
    VertexColor = aColor;

    // Calculate position in light space for shadow mapping
    FragPosLightSpace = uLightSpaceMatrix * worldPos;

    // Calculate final vertex position
    gl_Position = uProjection * uView * worldPos;
}

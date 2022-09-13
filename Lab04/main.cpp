#define NOMINMAX
#include <limits>

// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>

#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>
// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"
#include "stb_image.h"
#include "model.h"
#include "shader.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>

unsigned int loadTexture(const char* path);
unsigned int loadCubemap();

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME "swimmer.obj"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;

//data used when constructing vaos and tex data and needs to be made global 
typedef struct
{
	unsigned int vao_data;
	unsigned int tex_data;
} Init;

typedef struct
{
	unsigned int vao_data;
	int numStripsi;
	int numTrisPerStripi;
	unsigned int tex_data;
} InitTerrain;

#pragma endregion SimpleTypes
using namespace std;
GLuint shaderProgramID;

ModelData mesh_data;
unsigned int mesh_vao = 0;
int width = 1920;
int height = 1080;

// camera
Camera camera(glm::vec3(10.0f, 5.0f, 6.0f));
float lastX = (float)width / 2.0;
float lastY = (float)height / 2.0;
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float fov = 45.0f;

// lighting
Init lightCubeData;
glm::vec3 lightPos(1.2f, 5.0f, 2.0f);
glm::vec3 lightColour(1.0f, 1.0f, 1.0f);
glm::vec3 objectColour(1.0f, 0.5f, 0.31f);
float lightVertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

//terrain
InitTerrain terrainData;
//fog 
GLfloat fogColour[] = { 0.5f, 0.5f, 0.5f, 1.0f };

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


//skybox 
Init skyboxData;
float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

unsigned int swimmerTex;
GLuint loc1, loc2, loc3;

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name, 
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	); 

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		printf("%i bones in mesh\n", mesh->mNumBones);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

Init initSkybox() {
	Init data;
	unsigned int texture = loadCubemap();
	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	data.vao_data = skyboxVAO;
	data.tex_data = texture;
	return data;
}

Init initLightCube() {
	Init data;
	unsigned int tex = 0;
	// skybox VAO
	unsigned int lightVAO, lightVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);

	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), &lightVertices, GL_STATIC_DRAW);

	glBindVertexArray(lightVAO);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	data.vao_data = lightVAO;
	data.tex_data = tex;
	return data;
}

InitTerrain initTerrain() {
	InitTerrain dataForTerrain;
	// load and create a texture
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char* data = stbi_load("C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/Textures/heightmap.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	std::vector<float> vertices;
	float yScale = 64.0f / 256.0f, yShift = 35.0f;
	float xScale = 64.0f / 256.0f;
	int rez = 1;
	unsigned bytePerPixel = nrChannels;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			unsigned char* pixelOffset = data + (j + width * i) * bytePerPixel;
			unsigned char y = pixelOffset[0];

			// vertex
			vertices.push_back((-height / 2.0f + height * i / (float)height)*xScale);   // vx
			vertices.push_back((int)y * yScale - yShift);   // vy
			vertices.push_back(-width / 2.0f + width * j / (float)width);   // vz
		}
	}
	std::cout << "Loaded " << vertices.size() / 3 << " vertices" << std::endl;
	stbi_image_free(data);

	std::vector<unsigned> indices;
	for (unsigned i = 0; i < height - 1; i += rez)
	{
		for (unsigned j = 0; j < width; j += rez)
		{
			for (unsigned k = 0; k < 2; k++)
			{
				indices.push_back(j + width * (i + k * rez));
			}
		}
	}
	std::cout << "Loaded " << indices.size() << " indices" << std::endl;

	const int numStrips = (height - 1) / rez;
	const int numTrisPerStrip = (width / rez) * 2 - 2;
	std::cout << "Created lattice of " << numStrips << " strips with " << numTrisPerStrip << " triangles each" << std::endl;
	std::cout << "Created " << numStrips * numTrisPerStrip << " triangles total" << std::endl;
	//load map texture
	unsigned int texture = loadTexture("C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/sand.jpg");
	// first, configure the cube's VAO (and terrainVBO + terrainIBO)
	unsigned int terrainVAO, terrainVBO, terrainIBO;
	glGenVertexArrays(1, &terrainVAO);
	glBindVertexArray(terrainVAO);

	glGenBuffers(1, &terrainVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &terrainIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

	dataForTerrain.vao_data = terrainVAO;
	dataForTerrain.numStripsi = numStrips;
	dataForTerrain.numTrisPerStripi = numTrisPerStrip;
	dataForTerrain.tex_data = texture;
	return dataForTerrain;
}

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	mesh_data = load_mesh(MESH_NAME);
	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);
	//	This is for texture coordinates which you don't currently need, so I have commented it out
	unsigned int vt_vbo = 0;
	glGenBuffers(1, &vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);
	
	unsigned int vao = 0;
	glBindVertexArray(vao);
	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//	This is for texture coordinates which you don't currently need, so I have commented it out
	glEnableVertexAttribArray (loc3);
	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}
#pragma endregion VBO_FUNCTIONS

void fogDisplay(){
	glFogfv(GL_FOG_COLOR, fogColour);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glHint(GL_FOG_HINT, GL_NICEST);
	glFogf(GL_FOG_START, 0.1f);
	glFogf(GL_FOG_END, 10.0f);
};

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_FOG);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//projection and view matrices
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 10000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	//model shader
	Shader modelShader("Shaders/modelVS.txt", "Shaders/modelFS.txt");
	Model swimmerModel("C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/swimmer/swimmer.obj");
	
	modelShader.use();
	modelShader.setInt("material.diffuse", 0);
	modelShader.setInt("material.specular", 1);

	// light properties
	modelShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	modelShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
	modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

	// material properties
	modelShader.setFloat("material.shininess", 64.0f);
	modelShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	modelShader.setVec3("lightPos", lightPos);
	modelShader.setVec3("viewPos", camera.Position);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, swimmerTex);
	modelShader.setMat4("projection", projection);
	modelShader.setMat4("view", view);
	//render model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(5.0f, -2.0f, 3.0f));
	modelShader.setMat4("model", model);
	swimmerModel.Draw(modelShader);

	//light cube shader
	Shader lightCubeShader("Shaders/lightcube_vs.txt", "Shaders/lightcube_fs.txt");
	lightCubeShader.use();

	//draw light cube
	model = glm::mat4(1.0f);
	model = glm::translate(model, lightPos);
	lightCubeShader.setMat4("model", model);
	lightCubeShader.setMat4("projection", projection);
	lightCubeShader.setMat4("view", view);
	unsigned int lightCubeVAO = lightCubeData.vao_data;
	glBindVertexArray(lightCubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// activate camera shader
	Shader camShader("Shaders/camVS.txt", "Shaders/camFS.txt");
	camShader.use();
	model = glm::mat4(1.0f);
	// pass projection matrix to shader (note that in this case it could change every frame)
	camShader.setMat4("view", view);
	camShader.setMat4("model", model);
	camShader.setMat4("projection", projection);
	camShader.setVec3("cameraPos", camera.Position);
	
	// per-frame time logic
	// --------------------
	float currentFrame = static_cast<float>(timeGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	//terrain shader
	Shader terrainShader("Shaders/terrainVS.txt", "Shaders/terrainFS.txt");
	terrainShader.use();
	model = glm::mat4(1.0f);
	terrainShader.setMat4("view", view);
	terrainShader.setMat4("model", model);
	terrainShader.setMat4("projection", projection);
	unsigned int terrainVAO = terrainData.vao_data;
	int terrainNumStrips = terrainData.numStripsi;
	int terrainNumTrisPerStrip = terrainData.numTrisPerStripi;
	unsigned int terrainTexture = terrainData.tex_data;
	
	glBindVertexArray(terrainVAO);
	for (unsigned strip = 0; strip < terrainNumStrips; strip++)
	{
		glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
			terrainNumTrisPerStrip + 2,   // number of indices to render
			GL_UNSIGNED_INT,     // index data type
			(void*)(sizeof(unsigned) * (terrainNumTrisPerStrip + 2) * strip)); // offset to starting index
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, terrainTexture);
	//skybox shader
	Shader skyboxShader("Shaders/skymapVS.vert", "Shaders/skymapFS.frag");
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);
	// skybox cube
	unsigned int skyboxVAO = skyboxData.vao_data;
	unsigned int cubemapTexture = skyboxData.tex_data;
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	glBindVertexArray(skyboxVAO);
	skyboxShader.use();
	view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	skyboxShader.setMat4("view", view);
	skyboxShader.setMat4("projection", projection);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
	//fog
	fogDisplay();
	glutSwapBuffers();
}

void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Draw the next frame
	glutPostRedisplay();
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubemap()
{
	vector<std::string> faces
	{
	"C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/Textures/underwater/uw_rt.jpg",
	"C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/Textures/underwater/uw_lf.jpg",
	"C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/Textures/underwater/uw_up.jpg",
	"C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/Textures/underwater/uw_dn.jpg",
	"C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/Textures/underwater/uw_ft.jpg",
	"C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/Textures/underwater/uw_bk.jpg"
	};
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void init()
{
	// initialise data needed only once
	swimmerTex = loadTexture("C:/Users/Andrew/Documents/College/4th-Year/Graphics Project/pattern.jpg");
	skyboxData = initSkybox();
	lightCubeData = initLightCube();
	terrainData = initTerrain();
}

void processInput(unsigned char key, int x, int y)
{
	switch (key) {
	case 'd':
		camera.ProcessKeyboard(RIGHT, deltaTime);
		break;
	case 'a':
		camera.ProcessKeyboard(LEFT, deltaTime);
		break;
	case 'w':
		camera.ProcessKeyboard(FORWARD, deltaTime);
		break;
	case 's':
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		break;
	case 27:
		exit(0);
	}
	;	
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(int xposIn, int yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Ocean");
	glViewport(0, 0, width, height);


	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(processInput);
	glutPassiveMotionFunc(mouse_callback);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback( int xoffset, int yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


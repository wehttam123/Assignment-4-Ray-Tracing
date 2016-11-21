// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include "ImageBuffer.h"

// Specify that we want the OpenGL core profile before including GLFW headers
#ifndef LAB_LINUX
	#include <glad/glad.h>
#else
	#define GLFW_INCLUDE_GLCOREARB
	#define GL_GLEXT_PROTOTYPES
#endif
#include <GLFW/glfw3.h>

using namespace std;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
	// OpenGL names for vertex and fragment shaders, shader program
	GLuint  vertex;
	GLuint  fragment;
	GLuint  program;

	// initialize shader and program names to zero (OpenGL reserved value)
	MyShader() : vertex(0), fragment(0), program(0)
	{}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
	// load shader source from files
	string vertexSource = LoadSource("vertex.glsl");
	string fragmentSource = LoadSource("fragment.glsl");
	if (vertexSource.empty() || fragmentSource.empty()) return false;

	// compile shader source into shader objects
	shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	// link shader program
	shader->program = LinkProgram(shader->vertex, shader->fragment);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
	// unbind any shader programs and destroy shader objects
	glUseProgram(0);
	glDeleteProgram(shader->program);
	glDeleteShader(shader->vertex);
	glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
	// OpenGL names for array buffer objects, vertex array object
	GLuint  vertexBuffer;
	GLuint  colourBuffer;
	GLuint  vertexArray;
	GLsizei elementCount;

	// initialize object names to zero (OpenGL reserved value)
	MyGeometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
	{}
};

struct ray
{
	// OpenGL names for array buffer objects, vertex array object
	vector<float> position;
	vector<float> direction;
	vector<float> intersection;
};

void GeneratePoint(MyGeometry *geometry, MyShader *shader, vector <vector<GLfloat>> & coordinates, vector<vector <GLfloat>> & colour)
{
		GLfloat vertices[409600][2];
		for (int i = 0; i < 409600; i++) {
			vertices[i][0] = coordinates.at(i).at(0);
			vertices[i][1] = coordinates.at(i).at(1);
		}

		GLfloat colours[409600][3];
		for (int i = 0; i < 409600; i++) {
			colours[i][0] = colour.at(i).at(0);
			colours[i][1] = colour.at(i).at(1);
			colours[i][2] = colour.at(i).at(2);
		}

    geometry->elementCount = 409600;

    // these vertex attribute indices correspond to those specified for the
    // input variables in the vertex shader

    const GLuint VERTEX_INDEX = 0;
    const GLuint COLOUR_INDEX = 1;

    // create an array buffer object for storing our vertices
    glGenBuffers(1, &geometry->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // create another one for storing our colours
    glGenBuffers(1, &geometry->colourBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);

    // create a vertex array object encapsulating all our vertex attributes
    glGenVertexArrays(1, &geometry->vertexArray);
    glBindVertexArray(geometry->vertexArray);

    // associate the position array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glVertexAttribPointer(VERTEX_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(VERTEX_INDEX);

    // assocaite the colour array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glVertexAttribPointer(COLOUR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(COLOUR_INDEX);

    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // check for OpenGL errors and return false if error occurred
    CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
	// unbind and destroy our vertex array object and associated buffers
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &geometry->vertexArray);
	glDeleteBuffers(1, &geometry->vertexBuffer);
	glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(MyGeometry *geometry, MyShader *shader)
{
	// clear screen to a dark grey colour
	//glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glUseProgram(shader->program);
	glBindVertexArray(geometry->vertexArray);
	glDrawArrays(GL_POINTS, 0, geometry->elementCount);

	// reset state to default (no shader or geometry bound)
	glBindVertexArray(0);
	glUseProgram(0);

	// check for an report any OpenGL errors
	CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(640, 640, "Assignment #4: Raytracing", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, KeyCallback);
	glfwMakeContextCurrent(window);

	//Intialize GLAD
	#ifndef LAB_LINUX
	if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}
	#endif

	// query and print out information about our OpenGL environment
	QueryGLVersion();

	// call function to load and compile shader programs
	MyShader shader;
	if (!InitializeShaders(&shader)) {
		cout << "Program could not initialize shaders, TERMINATING" << endl;
		return -1;
	}

	// call function to create and fill buffers with geometry data
	MyGeometry geometry;
//	if (!InitializeGeometry(&geometry))
	//	cout << "Program failed to intialize geometry!" << endl;

		vector<vector<GLfloat>> vertices;
		vertices.resize(409600, vector<GLfloat>(2, 0.0));

		vector<vector<GLfloat>> colours;
		colours.resize(409600, vector<GLfloat>(3, 0.0));

		vector<ray> rays (409600);
		vector<float> position (2, 0);
		vector<float> direction (3, 0);
		vector<float> intersection (1, 0.0);

		vector<float> camera (3, 0);
		camera[2] = -640.0;

		vector<vector<float>> lights;
		lights.resize(50, vector<float>(3, 0.0));
		vector<vector<float>> spheres;
		spheres.resize(50, vector<float>(4, 0.0));
		vector<vector<float>> planes;
		planes.resize(50, vector<float>(6, 0.0));
		vector<vector<float>> triangles;
		triangles.resize(50, vector<float>(9, 0.0));


		int width = 640.0;
		int height = 640.0;

	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();

		//ray generation

		float row = -1;
		float col = -1;
		int PixelCount = 0;

		for (float i = 0; i < height; i++) {
			for (float j = 0; j < width; j++) {
				row = (2*(j/width))-1;

				//rays[PixelCount] = new struct <ray>;
				rays.push_back(ray());

				position[0] = row; position[1] = col;
				rays[PixelCount].position = position;

				direction[0] = row; /*(row/sqrt(pow(row, 2.0)+pow(col, 2.0)+pow(-2.0, 2.0))+camera[0]);*/direction[1] = col; /*(col/sqrt(pow(row, 2.0)+pow(col, 2.0)+pow(-2.0, 2.0)))+camera[1];*/ direction[2] = -2; /*-2.0/sqrt(pow(row, 2.0)+pow(col, 2.0)+pow(2.0, -2.0));*/
				rays[PixelCount].direction = direction;

				intersection[0] = -1.0;
				rays[PixelCount].intersection = intersection;

				vertices.at(PixelCount).at(0) = row;
				vertices.at(PixelCount).at(1) = col;
				//colours.at(PixelCount).at(0) = rays[PixelCount].direction[0];
				//std::cout << "dir " << rays[PixelCount].direction[0] << " " << rays[PixelCount].direction[1] << " " << rays[PixelCount].direction[2] << std::endl;
				//colours.at(PixelCount).at(1) = rays[PixelCount].direction[1];
				//colours.at(PixelCount).at(2) = rays[PixelCount].direction[2];
				PixelCount++;
			}
			col = (2*(i/height))-1;
		}
		GeneratePoint(&geometry, &shader, vertices, colours);
		RenderScene(&geometry, &shader);

		//read from file

		int lightCount = 0;
		int sphereCount = 0;
		int planeCount = 0;
		int triangleCount = 0;

				ifstream File;
				File.open("scene1.txt");
				if (File.is_open()){
		    string word;
		    while (File >> word)
		    {
		        //cout << word << '\n';
						if (word == "light") {
							File >> word;
							if (word == "{"){
							File >> word;
							if (word != "x"){
								lights.at(lightCount).at(0) = (float)atof(word.c_str());
								File >> word; lights.at(lightCount).at(1) = (float)atof(word.c_str());
								File >> word; lights.at(lightCount).at(2) = (float)atof(word.c_str());
								lightCount++;}}
						} else if (word == "sphere") {
							File >> word;
							if (word == "{"){
							File >> word;
							if (word != "x"){
								spheres.at(sphereCount).at(0) = (float)atof(word.c_str());
								File >> word; spheres.at(sphereCount).at(1) = (float)atof(word.c_str());
								File >> word; spheres.at(sphereCount).at(2) = (float)atof(word.c_str());
								File >> word; spheres.at(sphereCount).at(3) = (float)atof(word.c_str());
								sphereCount++;}}
						} else if (word == "plane"){
							File >> word;
							if (word == "{"){
							File >> word;
							if (word != "xn"){
								planes.at(planeCount).at(0) = (float)atof(word.c_str());
								File >> word; planes.at(planeCount).at(1) = (float)atof(word.c_str());
								File >> word; planes.at(planeCount).at(2) = (float)atof(word.c_str());
								File >> word; planes.at(planeCount).at(3) = (float)atof(word.c_str());
								File >> word; planes.at(planeCount).at(4) = (float)atof(word.c_str());
								File >> word; planes.at(planeCount).at(5) = (float)atof(word.c_str());
								planeCount++;}}
						}else if (word == "triangle"){
							File >> word;
							if (word == "{"){
							File >> word;
							if (word != "x1"){
								triangles.at(triangleCount).at(0) = (float)atof(word.c_str());
								File >> word; triangles.at(triangleCount).at(1) = (float)atof(word.c_str());
								File >> word; triangles.at(triangleCount).at(2) = (float)atof(word.c_str());
								File >> word; triangles.at(triangleCount).at(3) = (float)atof(word.c_str());
								File >> word; triangles.at(triangleCount).at(4) = (float)atof(word.c_str());
								File >> word; triangles.at(triangleCount).at(5) = (float)atof(word.c_str());
								File >> word; triangles.at(triangleCount).at(6) = (float)atof(word.c_str());
								File >> word; triangles.at(triangleCount).at(7) = (float)atof(word.c_str());
								File >> word; triangles.at(triangleCount).at(8) = (float)atof(word.c_str());
								triangleCount++;}}
						}
		    }}

				//intersection

				//sphere intersection
				float proj = 0;
				vector<float> projecton (3,0);
				for (int i = 0; i < sphereCount; i++) {
					for (int j = 0; j < 409600; j++){
						proj = ( (spheres.at(i).at(0) * rays[j].direction[0])
						+ (spheres.at(i).at(1) * rays[j].direction[1])
						+ (spheres.at(i).at(2) * rays[j].direction[2]) )/
						(pow((pow(rays[j].direction[0], 2.0)+pow(rays[j].direction[1], 2.0)+pow(rays[j].direction[2], 2.0)), 2.0 ));
						projecton[0] = proj*rays[j].direction[0];
						projecton[1] = proj*rays[j].direction[1];
						projecton[2] = proj*rays[j].direction[2];
						proj = sqrt(pow(rays[j].direction[0]-projecton[0], 2.0)+pow(rays[j].direction[1]-projecton[1], 2.0)+pow(rays[j].direction[2]-projecton[2], 2.0));
						if (proj < 0) {proj = proj * -1;}
						if (proj <= (float)spheres.at(i).at(3)) {
							if (rays[j].intersection[0] == -1.0){
								rays[j].intersection[0] = proj;
							} else if(rays[j].intersection[0] > proj){
								rays[j].intersection[0] = proj;
							}

							if(rays[j].intersection[0] == proj){
								colours.at(j).at(0) = 0.5;
								colours.at(j).at(1) = 0.5;
								colours.at(j).at(2) = 0.5;
							}
						}
					}
				}

				//triangle intersection
				float t = 0.0;
				float u = 0.0;
				float v = 0.0;

				float a = 0.0; float b = 0.0; float c = 0.0;
				float d = 0.0; float e = 0.0; float f = 0.0;
				float g = 0.0; float h = 0.0; float k = 0.0;

				//a(ei - fh) - b(di - fg) + c(dh - eg)

				for (int i = 0; i < triangleCount; i++) {
					for (int j = 0; j < 409600; j++){
						//p + t * d = (1-u-v) * p0 + u * p1 + v * p2
						a = -triangles.at(i).at(0); b = triangles.at(i).at(3) - triangles.at(i).at(0); c = triangles.at(i).at(6) - triangles.at(i).at(0);
						d = -triangles.at(i).at(1); e = triangles.at(i).at(4) - triangles.at(i).at(1); f = triangles.at(i).at(7) - triangles.at(i).at(1);
						g = -triangles.at(i).at(2); h = triangles.at(i).at(5) - triangles.at(i).at(2); k = triangles.at(i).at(8) - triangles.at(i).at(2);

						t = a*((e*k) - (f*h)) - b*((d*k) - (f*g)) + c*((d*h) - (e*g));

						a = -rays[j].direction[0]; b = triangles.at(i).at(3) - triangles.at(i).at(0); c = triangles.at(i).at(6) - triangles.at(i).at(0);
						d = -rays[j].direction[1]; e = triangles.at(i).at(4) - triangles.at(i).at(1); f = triangles.at(i).at(7) - triangles.at(i).at(1);
						g = -rays[j].direction[2]; h = triangles.at(i).at(5) - triangles.at(i).at(2); k = triangles.at(i).at(8) - triangles.at(i).at(2);

						t = t/(a*((e*k) - (f*h)) - b*((d*k) - (f*g)) + c*((d*h) - (e*g)));


						a = -rays[j].direction[0]; b = -triangles.at(i).at(0); c = triangles.at(i).at(6) - triangles.at(i).at(0);
						d = -rays[j].direction[1]; e = -triangles.at(i).at(1); f = triangles.at(i).at(7) - triangles.at(i).at(1);
						g = -rays[j].direction[2]; h = -triangles.at(i).at(2); k = triangles.at(i).at(8) - triangles.at(i).at(2);
						u = a*((e*k) - (f*h)) - b*((d*k) - (f*g)) + c*((d*h) - (e*g));

						a = -rays[j].direction[0]; b = triangles.at(i).at(3) - triangles.at(i).at(0); c = triangles.at(i).at(6) - triangles.at(i).at(0);
						d = -rays[j].direction[1]; e = triangles.at(i).at(4) - triangles.at(i).at(1); f = triangles.at(i).at(7) - triangles.at(i).at(1);
						g = -rays[j].direction[2]; h = triangles.at(i).at(5) - triangles.at(i).at(2); k = triangles.at(i).at(8) - triangles.at(i).at(2);

						u = u/(a*((e*k) - (f*h)) - b*((d*k) - (f*g)) + c*((d*h) - (e*g)));


						a = -rays[j].direction[0]; b = triangles.at(i).at(3) - triangles.at(i).at(0); c = -triangles.at(i).at(0);
						d = -rays[j].direction[1]; e = triangles.at(i).at(4) - triangles.at(i).at(1); f = -triangles.at(i).at(1);
						g = -rays[j].direction[2]; h = triangles.at(i).at(5) - triangles.at(i).at(2); k = -triangles.at(i).at(2);
						v = a*((e*k) - (f*h)) - b*((d*k) - (f*g)) + c*((d*h) - (e*g));

						a = -rays[j].direction[0]; b = triangles.at(i).at(3) - triangles.at(i).at(0); c = triangles.at(i).at(6) - triangles.at(i).at(0);
						d = -rays[j].direction[1]; e = triangles.at(i).at(4) - triangles.at(i).at(1); f = triangles.at(i).at(7) - triangles.at(i).at(1);
						g = -rays[j].direction[2]; h = triangles.at(i).at(5) - triangles.at(i).at(2); k = triangles.at(i).at(8) - triangles.at(i).at(2);

						v = v/(a*((e*k) - (f*h)) - b*((d*k) - (f*g)) + c*((d*h) - (e*g)));

						if(u >= 0){
							if(v >= 0){
								if(u+v <= 1.0){

									if (rays[j].intersection[0] == -1.0){
										rays[j].intersection[0] = t;
									} else if(rays[j].intersection[0] > t){
										rays[j].intersection[0] = t;
									}

									if(i >= 0){
										if(i < 4){
											if(rays[j].intersection[0] == t){
												colours.at(j).at(0) = 0.0;
												colours.at(j).at(1) = 0.0;
												colours.at(j).at(2) = 1.0;
											}
										} else if(i < 6){
											if(rays[j].intersection[0] == t){
												colours.at(j).at(0) = 1.0;
												colours.at(j).at(1) = 1.0;
												colours.at(j).at(2) = 1.0;
											}
										} else if(i < 8){
											if(rays[j].intersection[0] == t){
												colours.at(j).at(0) = 0.0;
												colours.at(j).at(1) = 1.0;
												colours.at(j).at(2) = 0.0;
											}
										} else if(i < 10){
											if(rays[j].intersection[0] == t){
												colours.at(j).at(0) = 1.0;
												colours.at(j).at(1) = 0.0;
												colours.at(j).at(2) = 0.0;
											}
										} else if(i < 12){
											if(rays[j].intersection[0] == t){
												colours.at(j).at(0) = 0.5;
												colours.at(j).at(1) = 0.5;
												colours.at(j).at(2) = 0.5;
											}
										}
									}
								}
							}
						}

					}
				}

				//plane intersection

				t = 0.0;
				for (int i = 0; i < planeCount; i++) {
					for (int j = 0; j < 409600; j++){
						t = ((planes.at(i).at(3) * planes.at(i).at(0)) + (planes.at(i).at(4) * planes.at(i).at(1)) + (planes.at(i).at(5) * planes.at(i).at(2)))/
						((rays[j].direction[0] * planes.at(i).at(0)) + (rays[j].direction[1] * planes.at(i).at(1)) + (rays[j].direction[2] * planes.at(i).at(2)));

						if (rays[j].intersection[0] == -1.0){
							rays[j].intersection[0] = t;
						} else if(rays[j].intersection[0] > t){
							rays[j].intersection[0] = t;
						}

						if(rays[j].intersection[0] == t){
							colours.at(j).at(0) = 0.7;
							colours.at(j).at(1) = 0.7;
							colours.at(j).at(2) = 0.7;
						}
					}
				}

				GeneratePoint(&geometry, &shader, vertices, colours);
				RenderScene(&geometry, &shader);

		File.close();
	}

	// clean up allocated resources before exit
	DestroyGeometry(&geometry);
	DestroyShaders(&shader);
	glfwDestroyWindow(window);
	glfwTerminate();

	cout << "Goodbye!" << endl;
	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "OpenGL ERROR:  ";
		switch (flag) {
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM" << endl; break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE" << endl; break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION" << endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY" << endl; break;
		default:
			cout << "[unknown error code]" << endl;
		}
		error = true;
	}
	return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
	string source;

	ifstream input(filename.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	}
	else {
		cout << "ERROR: Could not load shader source from file "
			<< filename << endl;
	}

	return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
	// allocate shader object name
	GLuint shaderObject = glCreateShader(shaderType);

	// try compiling the source as a shader of the given type
	const GLchar *source_ptr = source.c_str();
	glShaderSource(shaderObject, 1, &source_ptr, 0);
	glCompileShader(shaderObject);

	// retrieve compile status
	GLint status;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
		cout << "ERROR compiling shader:" << endl << endl;
		cout << source << endl;
		cout << info << endl;
	}

	return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	// allocate program object name
	GLuint programObject = glCreateProgram();

	// attach provided shader objects to this program
	if (vertexShader)   glAttachShader(programObject, vertexShader);
	if (fragmentShader) glAttachShader(programObject, fragmentShader);

	// try linking the program with given attachments
	glLinkProgram(programObject);

	// retrieve link status
	GLint status;
	glGetProgramiv(programObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
		cout << "ERROR linking shader program:" << endl;
		cout << info << endl;
	}

	return programObject;
}

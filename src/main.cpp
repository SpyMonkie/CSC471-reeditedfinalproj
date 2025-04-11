/*
 * Example two meshes and two shaders (could also be used for Program 2)
 * includes modifications to shape and initGeom in preparation to load
 * multi shape objects
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>
#include <chrono>
#include <thread>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "Spline.h"
#include "stb_image.h"
#include "AssimpModel.h"
#include "Animator.h"
#include "LightTrail.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define NUM_LIGHTS 4
#define MAX_BONES 100

class Application : public EventCallbacks {

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Our shader program
	std::shared_ptr<Program> solidColorProg;

	std::shared_ptr<Program> prog2;

	std::shared_ptr<Program> texProg;

	std::shared_ptr<Program> skyProg;

	std::shared_ptr<Program> assimptexProg;

	GLuint skyMapTexture;

	//ground VAO
	GLuint GroundVertexArrayID;

	shared_ptr<Texture> texture0;

	shared_ptr<Texture> barrierTexture;

	shared_ptr<Texture> gamingroomwallTexture;

	shared_ptr<Texture> gamingroomfloorTexture;

	shared_ptr<Texture> streetTexture;

	shared_ptr<Texture> sidewalkTexture;

	shared_ptr<Texture> skyTexture;

	shared_ptr<Texture> monitor1Texture;
	shared_ptr<Texture> monitor2Texture;
	shared_ptr<Texture> monitor3Texture;

	shared_ptr<Shape> sky; // big sphere

	shared_ptr<Shape> cube_sky;

	vector<shared_ptr<Shape>> mans;

	vector<shared_ptr<Shape>> gaming_rooms;

	vector<shared_ptr<Shape>> gaming_chairs;

	vector<shared_ptr<Shape>> room_lamps;

	vector<shared_ptr<Shape>> gaming_monitors;

	shared_ptr<Shape> gaming_room_floor;

	vector<shared_ptr<Shape>> gaming_room_walls;

	vector<string> gaming_monitors_names;

	AssimpModel *stickfigure_running;

	Animation *stickfigure_anim;

	Animator *stickfigure_animator;

	float AnimDeltaTime = 0.0f;
	float AnimLastFrame = 0.0f;

	// vector<string> light_cycles_names;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	float lightTrans = -2;

	int change_mat = 0;

	//animation data
	float sTransx = -2.5;
	float sTransy = -1.3;
	float vTransz = 0;
	float vThetax = 0.0f;
	float vThetay = 0.0f;
	float vTransx = 2;
	float vTransy = 0;

	vec3 manTrans = vec3(-2.5, -1.3, 0);
	vec3 manScale = vec3(0.01, 0.01, 0.01);
	vec3 manRot = vec3(radians(0.0f), radians(90.0f), radians(0.0f));

	vec3 manMoveDir = vec3(sin(manRot.y), 0, cos(manRot.y));

	// initial position of light cycles
	vec3 start_lightcycle1_pos = vec3(-384, -11, 31);
	vec3 start_lightcycle2_pos = vec3(-365, -11, 9.1);


	float theta = 0.0f; // controls yaw
	// float theta = radians(90.0f); // controls yaw
	// float phi = 0.0f; // controls pitch
	float phi = radians(-30.0f); // controls pitch

	float radius = 5.0f;

	float wasd_sens = 0.5f;

	vec3 eye = vec3(-6, 1.03, 0);
	// vec3 lookAt = vec3(-1.58614, -0.9738, 0.0436656);
	vec3 lookAt = manTrans;
	vec3 up = vec3(0, 1, 0);


	vector<std::string> night_city_faces {
		"night_city_right.jpg",
		"night_city_left.jpg",
		"night_city_up.jpg",
		"night_city_down.jpg",
		"night_city_front.jpg",
		"night_city_back.jpg"
	};

	int debug = 0;
	int debug_pos = 1;

	enum State {
		START,
		BEFORE_DESKTOP,
		IN_DESKTOP,
	};

	enum View_State {
		START_VIEW,
		START_ANIM_VIEW,
		IN_DESKTOP_VIEW,
	};

	enum Man_State {
		WALKING,
		STANDING,
	};

	Man_State manState = STANDING;
	State currState = START;
	View_State currViewState = START_VIEW;
	// State currState = IN_DESKTOP;
	// View_State currViewState = IN_DESKTOP_VIEW;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//update global camera rotate
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE) {
			// eye = eye + wasd_sens * normalize(lookAt - eye); // w vector
			// lookAt = lookAt + wasd_sens * normalize(lookAt - eye); // w vector
			// manTrans.x += 0.2f;
			// eye.x += 0.2f;
			manTrans += manMoveDir * 0.2f;
			eye += manMoveDir * 0.2f;
			lookAt = manTrans;


			manState = WALKING;

			// cout << "pressing W" << endl;


			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}
		} else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			manState = STANDING;

			// cout << "releasing W" << endl;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE) {
			// eye = eye - wasd_sens * normalize(lookAt - eye); // w vector
			// lookAt = lookAt - wasd_sens * normalize(lookAt - eye); // w vector
			// manTrans.x -= 0.2f;
			// eye.x -= 0.2f;
			manTrans -= manMoveDir * 0.2f;
			eye -= manMoveDir * 0.2f;
			lookAt = manTrans;


			manState = WALKING;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}

		} else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			manState = STANDING;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE) {
			// eye = eye - wasd_sens * normalize(cross(lookAt - eye, up)); // u vector
			// lookAt = lookAt - wasd_sens * normalize(cross(lookAt - eye, up)); // u vector

			// manTrans.z -= 0.2f;
			// eye.z -= 0.2f;
			vec3 right = normalize(cross(manMoveDir, up));
			manTrans -= right * 0.2f;
			eye -= right * 0.2f;
			lookAt = manTrans;


			manState = WALKING;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}

		} else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			manState = STANDING;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE) {
			// eye = eye + wasd_sens * normalize(cross(lookAt - eye, up)); // u vector
			// lookAt = lookAt + wasd_sens * normalize(cross(lookAt - eye, up)); // u vector

			// manTrans.z += 0.2f;
			// eye.z += 0.2f;
			vec3 right = normalize(cross(manMoveDir, up));
			manTrans += right * 0.2f;
			eye += right * 0.2f;
			lookAt = manTrans;


			manState = WALKING;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}
		} else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			manState = STANDING;
		}
		if (glfwGetKey(window, GLFW_KEY_Q)){
			lightTrans += 1.0;
		}
		if (glfwGetKey(window, GLFW_KEY_E)){
			lightTrans -= 1.0;
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS && (currViewState == START_VIEW || currViewState == START_ANIM_VIEW)) {
			change_mat = (change_mat + 1) % 5;
		}

		if (key == GLFW_KEY_M && action == GLFW_PRESS && currViewState == IN_DESKTOP_VIEW) {
			if (change_mat == 8) {
				change_mat = 0;
			} else {
				change_mat += 1;
			}
		}

		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}

		if (key == GLFW_KEY_G && action == GLFW_PRESS && currViewState == IN_DESKTOP_VIEW) {
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && (currViewState == START_VIEW)) {
		}
	}

	void scrollCallback(GLFWwindow *window, double deltaX, double deltaY)
	{

			float sensitivity = 0.7f;

			theta = theta + deltaX * sensitivity;

			phi = phi - deltaY * sensitivity;

			if (phi > radians(80.0f))
			{
				phi = radians(80.0f);
			}
			if (phi < radians(-80.0f))
			{
				phi = radians(-80.0f);
			}

			updateCameraVectors();



	}

	void updateCameraVectors() {
		vec3 front;
		front.x = radius * cos(phi) * cos(theta);
		front.y = radius * sin(phi);
		front.z = radius * cos(phi) * cos((pi<float>()/2) - theta);

		eye = manTrans - front;
		lookAt = manTrans;

		manRot.y = theta + radians(-90.0f);
		manRot.y = - manRot.y;
		manRot.x = phi;

		manMoveDir = vec3(sin(manRot.y), 0, cos(manRot.y));

		// lookAt = eye + front;


	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}


	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		// Initialize the GLSL program.
		solidColorProg = make_shared<Program>();
		solidColorProg->setVerbose(true);
		solidColorProg->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/solid_frag.glsl");
		solidColorProg->init();
		solidColorProg->addUniform("P");
		solidColorProg->addUniform("V");
		solidColorProg->addUniform("M");
		solidColorProg->addUniform("solidColor");
		solidColorProg->addAttribute("vertPos");
		solidColorProg->addAttribute("vertNor");

		prog2 = make_shared<Program>();
		prog2->setVerbose(true);
		prog2->setShaderNames(resourceDirectory + "/simple_light_vert.glsl", resourceDirectory + "/simple_light_frag.glsl");
		prog2->init();
		prog2->addUniform("P");
		prog2->addUniform("V");
		prog2->addUniform("M");
		prog2->addUniform("MatAmb");
		prog2->addAttribute("vertPos");
		prog2->addAttribute("vertNor");
		prog2->addUniform("MatDif");
		prog2->addUniform("MatSpec");
		prog2->addUniform("MatShine");
		for (int i = 0; i < NUM_LIGHTS; i++) {
			prog2->addUniform("lightPos[" + to_string(i) + "]");
			prog2->addUniform("lightColor[" + to_string(i) + "]");
			prog2->addUniform("lightIntensity[" + to_string(i) + "]");
		}
		prog2->addUniform("numLights");
		prog2->addUniform("hasEmittance");
		prog2->addUniform("MatEmitt");
		prog2->addUniform("MatEmittIntensity");
		prog2->addUniform("discardCounter");
		prog2->addUniform("activateDiscard");
		prog2->addUniform("randFloat1");
		prog2->addUniform("randFloat2");
		prog2->addUniform("randFloat3");
		prog2->addUniform("randFloat4");

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("Texture0");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");
		texProg->addUniform("MatAmb");
		texProg->addUniform("MatSpec");
		texProg->addUniform("MatShine");
		for (int i = 0; i < NUM_LIGHTS; i++) {
			texProg->addUniform("lightPos[" + to_string(i) + "]");
			texProg->addUniform("lightColor[" + to_string(i) + "]");
			texProg->addUniform("lightIntensity[" + to_string(i) + "]");
		}

		texProg->addUniform("numLights");

		skyProg = make_shared<Program>();
		skyProg->setVerbose(true);
		skyProg->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		skyProg->init();
		skyProg->addUniform("P");
		skyProg->addUniform("V");
		skyProg->addUniform("M");
		skyProg->addUniform("skybox");
		skyProg->addAttribute("vertPos");
		skyProg->addAttribute("vertNor");
		skyProg->addUniform("MatShine");
		skyProg->addUniform("lightPos[0]");
		// skyProg->addUniform("lightPos[1]");
		skyProg->addUniform("lightColor[0]");
		// skyProg->addUniform("lightColor[1]");
		skyProg->addUniform("lightIntensity[0]");
		// skyProg->addUniform("lightIntensity[1]");
		// for (int i = 0; i < 1; i++) {
		// 	skyProg->addUniform("lightPos[" + to_string(i) + "]");
		// 	skyProg->addUniform("lightColor[" + to_string(i) + "]");
		// 	skyProg->addUniform("lightIntensity[" + to_string(i) + "]");
		// }
		skyProg->addUniform("numLights");


		assimptexProg = make_shared<Program>();
			assimptexProg->setVerbose(true);
			assimptexProg->setShaderNames(resourceDirectory + "/assimp_tex_vert.glsl", resourceDirectory + "/assimp_tex_frag.glsl");
			assimptexProg->init();
			assimptexProg->addUniform("P");
			assimptexProg->addUniform("V");
			assimptexProg->addUniform("M");

			assimptexProg->addUniform("texture_diffuse1");
			assimptexProg->addUniform("texture_specular1");
			// assimptexProg->addUniform("texture_normal1");
			// assimptexProg->addUniform("texture_height1");
			assimptexProg->addUniform("texture_roughness1");
			assimptexProg->addUniform("texture_metalness1");
			assimptexProg->addUniform("texture_emission1");
			assimptexProg->addAttribute("vertPos");
			assimptexProg->addAttribute("vertNor");
			assimptexProg->addAttribute("vertTex");
			// assimptexProg->addAttribute("vertTan");
			// assimptexProg->addAttribute("vertBitan");
			assimptexProg->addAttribute("boneIds");
			assimptexProg->addAttribute("weights");
			for (int i = 0; i < MAX_BONES; i++) {
				assimptexProg->addUniform("finalBonesMatrices[" + to_string(i) + "]");
			}
			assimptexProg->addUniform("MatAmb");
			assimptexProg->addUniform("MatDif");
			assimptexProg->addUniform("MatSpec");
			assimptexProg->addUniform("MatShine");
			for (int i = 0; i < NUM_LIGHTS; i++) {
				assimptexProg->addUniform("lightPos[" + to_string(i) + "]");
				assimptexProg->addUniform("lightColor[" + to_string(i) + "]");
				assimptexProg->addUniform("lightIntensity[" + to_string(i) + "]");
			}
			assimptexProg->addUniform("numLights");
			assimptexProg->addUniform("hasTexture");


		//read in a load the texture
		// texture0 = make_shared<Texture>();
		// texture0->setFilename(resourceDirectory + "/asphalt-texture-close-up.jpg");
		// texture0->init();
		// texture0->setUnit(0);
		// texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		gamingroomfloorTexture = make_shared<Texture>();
		gamingroomfloorTexture->setFilename(resourceDirectory + "/carpet_texture1.jpg");
		gamingroomfloorTexture->init();
		gamingroomfloorTexture->setUnit(1);
		gamingroomfloorTexture->setWrapModes(GL_REPEAT, GL_REPEAT);

		gamingroomwallTexture = make_shared<Texture>();
		gamingroomwallTexture->setFilename(resourceDirectory + "/gaming_wall.jpg");
		gamingroomwallTexture->init();
		gamingroomwallTexture->setUnit(2);
		// gamingroomwallTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		gamingroomwallTexture->setWrapModes(GL_REPEAT, GL_REPEAT);

		updateCameraVectors();



	}

	void initGeom(const std::string& resourceDirectory)
	{

		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		string errStr;


		vector<tinyobj::shape_t> TOshapes4;
		vector<tinyobj::material_t> objMaterials4;

		bool rc = tinyobj::LoadObj(TOshapes4, objMaterials4, errStr, (resourceDirectory + "/rest_of_gaming_room.obj").c_str());

		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (size_t i = 0; i < TOshapes4.size(); ++i) {
				auto gaming_room = make_shared<Shape>(false);
				gaming_room->createShape(TOshapes4[i]);
				gaming_room->init();
				gaming_rooms.push_back(gaming_room);

				if (debug) {
				cout << "gaming_room name: " << TOshapes4[i].name << endl;
				}
			}

			this->gaming_rooms = gaming_rooms;
		}

		vector<tinyobj::shape_t> TOshapes5;
		vector<tinyobj::material_t> objMaterials5;

		rc = tinyobj::LoadObj(TOshapes5, objMaterials5, errStr, (resourceDirectory + "/gaming_room_walls.obj").c_str());

		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (size_t i = 0; i < TOshapes5.size(); ++i) {
				auto gaming_room_wall = make_shared<Shape>(true);
				gaming_room_wall->createShape(TOshapes5[i]);
				gaming_room_wall->init();
				gaming_room_walls.push_back(gaming_room_wall);

				if (debug) {
				cout << "gaming_room name: " << TOshapes5[i].name << endl;
				}
			}

			this->gaming_rooms = gaming_rooms;
		}

		vector<tinyobj::shape_t> TOshapes6;
		vector<tinyobj::material_t> objMaterials6;

		rc = tinyobj::LoadObj(TOshapes6, objMaterials6, errStr, (resourceDirectory + "/gaming_room_floor.obj").c_str());

		if (!rc) {
			cerr << errStr << endl;
		} else {
			gaming_room_floor = make_shared<Shape>(true);
			gaming_room_floor->createShape(TOshapes6[0]);
			gaming_room_floor->init();
		}

		vector<tinyobj::shape_t> TOshapes10;
		vector<tinyobj::material_t> objMaterials10;

 		rc = tinyobj::LoadObj(TOshapes10, objMaterials10, errStr, (resourceDirectory + "/Monitors.obj").c_str());

		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (size_t i = 0; i < TOshapes10.size(); ++i) {
				auto gaming_monitor = make_shared<Shape>(false);
				gaming_monitor->createShape(TOshapes10[i]);
				gaming_monitor->init();
				gaming_monitors.push_back(gaming_monitor);
				if (debug) {
					cout << "gaming_monitor size: " << TOshapes10[i].name << endl;
				}
			}

			this->gaming_monitors = gaming_monitors;
		}

		vector<tinyobj::shape_t> TOshapes11;
		vector<tinyobj::material_t> objMaterials11;

 		rc = tinyobj::LoadObj(TOshapes11, objMaterials11, errStr, (resourceDirectory + "/gaming_chair.obj").c_str());

		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (size_t i = 0; i < TOshapes11.size(); ++i) {
				auto gaming_chair = make_shared<Shape>(false);
				gaming_chair->createShape(TOshapes11[i]);
				gaming_chair->init();
				gaming_chairs.push_back(gaming_chair);
				if (debug) {
					cout << "gaming_chair size: " << TOshapes11[i].name << endl;
				}
			}

			this->gaming_chairs = gaming_chairs;
		}

		vector<tinyobj::shape_t> TOshapes12;
		vector<tinyobj::material_t> objMaterials12;

 		rc = tinyobj::LoadObj(TOshapes12, objMaterials12, errStr, (resourceDirectory + "/room_lamp.obj").c_str());

		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (size_t i = 0; i < TOshapes12.size(); ++i) {
				auto room_lamp = make_shared<Shape>(false);
				room_lamp->createShape(TOshapes12[i]);
				room_lamp->init();
				room_lamps.push_back(room_lamp);

				if (debug) {
					cout << "room_lamp size: " << TOshapes12[i].name << endl;
				}
			}

			this->room_lamps = room_lamps;
		}


		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		// gMin.x = mesh->min.x;
		// gMin.y = mesh->min.y;

		stickfigure_running = new AssimpModel(resourceDirectory + "/stickfigure_anim.fbx");
		stickfigure_anim = new Animation(resourceDirectory + "/stickfigure_anim.fbx", stickfigure_running);
		stickfigure_animator = new Animator(stickfigure_anim);

	}

	unsigned int createSky(string dir, vector<string> faces) {
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for(GLuint i = 0; i < faces.size(); i++) {
		unsigned char *data =
		stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
		if (data) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
		0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else {
		cout << "failed to load: " << (dir+faces[i]).c_str() << endl;
		}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// cout << " creating cube map any errors : " << glGetError() << endl;
		return textureID;
	}

	void SetMaterial(shared_ptr<Program> curS, int i) {

    	switch (i) {
    		case 0: //
    			glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
    			glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
    			glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.23, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 120.0);
    		break;
    		case 1: //

				// neon green
				glUniform3f(curS->getUniform("MatAmb"), 0.6f, 0.0f, 0.6f); // Dark pink ambient
				glUniform3f(curS->getUniform("MatDif"), 0.0f, 1.0f, 0.0f); // Neon pink diffuse
				glUniform3f(curS->getUniform("MatSpec"), 0.8f, 0.8f, 0.8f); // White specular
				glUniform1f(curS->getUniform("MatShine"), 50.0f); // medium shine
    		break;
    		case 2:

				// Neon pink
				glUniform3f(curS->getUniform("MatAmb"), 0.6f, 0.0f, 0.6f); // Dark pink ambient
				glUniform3f(curS->getUniform("MatDif"), 1.0f, 0.0f, 1.0f); // Neon pink diffuse
				glUniform3f(curS->getUniform("MatSpec"), 0.8f, 0.8f, 0.8f); // White specular
				glUniform1f(curS->getUniform("MatShine"), 50.0f); // medium shine
            break;
			case 3:
				glUniform3f(curS->getUniform("MatAmb"), 0.05f, 0.05f, 0.05f); // black ambient
				glUniform3f(curS->getUniform("MatDif"), 0.07f, 0.07f, 0.07f); // black diffuse
				glUniform3f(curS->getUniform("MatSpec"), 0.1f, 0.7f, 1.0f); // blue specular
				glUniform1f(curS->getUniform("MatShine"), 300.0f); // high shine
				// Blue glow
				glUniform3f(curS->getUniform("MatEmitt"), 0.1f, 0.7f, 1.0f); // Blue emission matches specular
				glUniform1f(curS->getUniform("MatEmittIntensity"), 0.8f); // Bright glow for a neon effect
			break;
			case 4:
				glUniform3f(curS->getUniform("MatAmb"), 0.05f, 0.05f, 0.05f); // black ambient
				glUniform3f(curS->getUniform("MatDif"), 0.07f, 0.07f, 0.07f); // black diffuse
				glUniform3f(curS->getUniform("MatSpec"), 1.0f, 0.7f, 0.1f); // orange specular
				glUniform1f(curS->getUniform("MatShine"), 300.0f); // high shine
				// Orange glow
				glUniform3f(curS->getUniform("MatEmitt"), 1.0f, 0.7f, 0.1f); // Orange emission matches specular
				glUniform1f(curS->getUniform("MatEmittIntensity"), 0.8f); // Consistent bright glow for orange
				break;
    		break;
  		}
	}

	void SetMaterialMonitor(shared_ptr<Program> curS) {
		glUniform3f(curS->getUniform("MatAmb"), 0.7f, 0.7f, 0.7f); // silver ambient
		glUniform3f(curS->getUniform("MatDif"), 0.85f, 0.85f, 0.85f); // dark white diffuse
		glUniform3f(curS->getUniform("MatSpec"), 1.0f, 1.0f, 1.0f); // white specular
		glUniform1f(curS->getUniform("MatShine"), 60.0f); // medium shine
	}

	void SetMaterialGamingChair(shared_ptr<Program> curS) {
		glUniform3f(curS->getUniform("MatAmb"), 0.0f, 0.0f, 0.0f); // black ambient
		glUniform3f(curS->getUniform("MatDif"), 1.0f, 0.0f, 0.0f); // black diffuse
		glUniform3f(curS->getUniform("MatSpec"), 0.5f, 0.5f, 0.5f); // grey specular
		glUniform1f(curS->getUniform("MatShine"), 60.0f); // low shine
	}

	void SetMaterialGamingRoom(shared_ptr<Program> curS) {
		glUniform3f(curS->getUniform("MatAmb"), 0.0f, 0.0f, 0.0f); // black ambient
		glUniform3f(curS->getUniform("MatDif"), 0.05f, 0.05f, 0.05f); // black diffuse
		glUniform3f(curS->getUniform("MatSpec"), 1.0f, 1.0f, 1.0f); // grey specular
		glUniform1f(curS->getUniform("MatShine"), 60.0f); // low shine
	}

	void SetMaterialMan(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
			// gold
				glUniform3f(curS->getUniform("MatAmb"), 0.24725f, 0.1995f, 0.0745f);
				glUniform3f(curS->getUniform("MatDif"), 0.75164f, 0.60648f, 0.22648f);
				glUniform3f(curS->getUniform("MatSpec"), 0.628281f, 0.555802f, 0.366065f);
				glUniform1f(curS->getUniform("MatShine"), 51.2f);
			break;
			case 1:
			// silver
				glUniform3f(curS->getUniform("MatAmb"), 0.19225f, 0.19225f, 0.19225f);
				glUniform3f(curS->getUniform("MatDif"), 0.50754f, 0.50754f, 0.50754f);
				glUniform3f(curS->getUniform("MatSpec"), 0.508273f, 0.508273f, 0.508273f);
				glUniform1f(curS->getUniform("MatShine"), 51.2f);
			break;
			case 2:
			// bronze
				glUniform3f(curS->getUniform("MatAmb"), 0.2125f, 0.1275f, 0.054f);
				glUniform3f(curS->getUniform("MatDif"), 0.714f, 0.4284f, 0.18144f);
				glUniform3f(curS->getUniform("MatSpec"), 0.393548f, 0.271906f, 0.166721f);
				glUniform1f(curS->getUniform("MatShine"), 25.6f);
			break;
			case 3:
			// black
				glUniform3f(curS->getUniform("MatAmb"), 0.01f, 0.01f, 0.01f);
				glUniform3f(curS->getUniform("MatDif"), 0.07f, 0.07f, 0.07f);
				glUniform3f(curS->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f);
			break;
			case 4:
			// dark white
				glUniform3f(curS->getUniform("MatAmb"), 0.05f, 0.05f, 0.05f);
				glUniform3f(curS->getUniform("MatDif"), 0.5f, 0.5f, 0.5f);
				glUniform3f(curS->getUniform("MatSpec"), 0.7f, 0.7f, 0.7f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f);
			break;
		}
	}

	void SetMaterialCity(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
				// Concrete
				glUniform3f(curS->getUniform("MatAmb"), 0.2f, 0.2f, 0.2f);
				glUniform3f(curS->getUniform("MatDif"), 0.5f, 0.5f, 0.5f);
				glUniform3f(curS->getUniform("MatSpec"), 0.3f, 0.3f, 0.3f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f);
				break;
			case 1:
				// Brick
				glUniform3f(curS->getUniform("MatAmb"), 0.3f, 0.15f, 0.1f);
				glUniform3f(curS->getUniform("MatDif"), 0.6f, 0.3f, 0.2f);
				glUniform3f(curS->getUniform("MatSpec"), 0.2f, 0.1f, 0.1f);
				glUniform1f(curS->getUniform("MatShine"), 5.0f);
				break;
			case 2:
				// Asphalt
				glUniform3f(curS->getUniform("MatAmb"), 0.1f, 0.1f, 0.1f);
				glUniform3f(curS->getUniform("MatDif"), 0.2f, 0.2f, 0.2f);
				glUniform3f(curS->getUniform("MatSpec"), 0.3f, 0.3f, 0.3f);
				glUniform1f(curS->getUniform("MatShine"), 20.0f);
				break;
			case 3:
				// Metal
				glUniform3f(curS->getUniform("MatAmb"), 0.25f, 0.25f, 0.25f);
				glUniform3f(curS->getUniform("MatDif"), 0.4f, 0.4f, 0.4f);
				glUniform3f(curS->getUniform("MatSpec"), 0.774597f, 0.774597f, 0.774597f);
				glUniform1f(curS->getUniform("MatShine"), 76.8f);
				break;
			case 4:
				// Glass
				glUniform3f(curS->getUniform("MatAmb"), 0.02f, 0.02f, 0.02f);
				glUniform3f(curS->getUniform("MatDif"), 0.1f, 0.1f, 0.1f);
				glUniform3f(curS->getUniform("MatSpec"), 0.7f, 0.7f, 0.7f);
				glUniform1f(curS->getUniform("MatShine"), 100.0f);
				break;
			case 5:
				// Wood
				glUniform3f(curS->getUniform("MatAmb"), 0.1f, 0.05f, 0.02f);
				glUniform3f(curS->getUniform("MatDif"), 0.6f, 0.3f, 0.1f);
				glUniform3f(curS->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
				glUniform1f(curS->getUniform("MatShine"), 5.0f);
				break;
			case 6:
				// Painted Surface
				glUniform3f(curS->getUniform("MatAmb"), 0.1f, 0.1f, 0.1f);
				glUniform3f(curS->getUniform("MatDif"), 0.9f, 0.5f, 0.5f);
				glUniform3f(curS->getUniform("MatSpec"), 0.3f, 0.3f, 0.3f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f);
				break;
			case 7:
				// Plastic
				glUniform3f(curS->getUniform("MatAmb"), 0.2f, 0.2f, 0.2f);
				glUniform3f(curS->getUniform("MatDif"), 0.7f, 0.7f, 0.7f);
				glUniform3f(curS->getUniform("MatSpec"), 0.4f, 0.4f, 0.4f);
				glUniform1f(curS->getUniform("MatShine"), 30.0f);
				break;
		}
	}

	void SetMaterialBuildings(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
				// Concrete (Light Gray)
				glUniform3f(curS->getUniform("MatAmb"), 0.3f, 0.3f, 0.3f);
				glUniform3f(curS->getUniform("MatDif"), 0.6f, 0.6f, 0.6f);
				glUniform3f(curS->getUniform("MatSpec"), 0.2f, 0.2f, 0.2f);
				glUniform1f(curS->getUniform("MatShine"), 15.0f);
				break;
			case 1:
				// Dark Metal (Steel)
				glUniform3f(curS->getUniform("MatAmb"), 0.1f, 0.1f, 0.1f);
				glUniform3f(curS->getUniform("MatDif"), 0.4f, 0.4f, 0.4f);
				glUniform3f(curS->getUniform("MatSpec"), 0.8f, 0.8f, 0.8f);
				glUniform1f(curS->getUniform("MatShine"), 50.0f);
				break;
			case 2:
				// Weathered Concrete (Dark Gray)
				glUniform3f(curS->getUniform("MatAmb"), 0.2f, 0.2f, 0.2f);
				glUniform3f(curS->getUniform("MatDif"), 0.5f, 0.5f, 0.5f);
				glUniform3f(curS->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
				glUniform1f(curS->getUniform("MatShine"), 5.0f);
				break;
			case 3:
				// Concrete (Gray)
				glUniform3f(curS->getUniform("MatAmb"), 0.25f, 0.25f, 0.25f);
				glUniform3f(curS->getUniform("MatDif"), 0.55f, 0.55f, 0.55f);
				glUniform3f(curS->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
				glUniform1f(curS->getUniform("MatShine"), 5.0f);
		}
	}


	void SetMaterialCityRoad(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
				// Concrete
				glUniform3f(curS->getUniform("MatAmb"), 0.4f, 0.4f, 0.4f);
				glUniform3f(curS->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f); // some shine
				break;
		}
	}

	void SetMaterialCitySky(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
				// Reset values
				glUniform3f(curS->getUniform("MatAmb"), 1.0f, 1.0f, 1.0f);
				glUniform3f(curS->getUniform("MatSpec"), 1.0f, 1.0f, 1.0f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f); // some shine
				break;
		}
	}

	void SetMaterialStreetLights(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
				// Metal
				glUniform3f(curS->getUniform("MatAmb"), 0.25f, 0.25f, 0.25f);
				glUniform3f(curS->getUniform("MatDif"), 0.4f, 0.4f, 0.4f);
				glUniform3f(curS->getUniform("MatSpec"), 0.774597f, 0.774597f, 0.774597f);
				glUniform1f(curS->getUniform("MatShine"), 76.8f);
				break;
		}
	}

	void SetMaterialTrail(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
				// shiny blue
				glUniform3f(curS->getUniform("MatAmb"), 0.0f, 0.8f, 1.0f);
				glUniform3f(curS->getUniform("MatDif"), 0.0f, 0.8f, 1.0f);
				glUniform3f(curS->getUniform("MatSpec"), 0.0f, 0.8f, 1.0f);
				glUniform1f(curS->getUniform("MatShine"), 100.0f);
				glUniform3f(curS->getUniform("MatEmitt"), 0.0f, 0.8f, 1.0f);
				glUniform1f(curS->getUniform("MatEmittIntensity"), 0.5f);
				break;
			case 1:
				// shiny orange
				glUniform3f(curS->getUniform("MatAmb"), 1.0f, 0.8f, 0.0f);
				glUniform3f(curS->getUniform("MatDif"), 1.0f, 0.8f, 0.0f);
				glUniform3f(curS->getUniform("MatSpec"), 1.0f, 0.8f, 0.0f);
				glUniform1f(curS->getUniform("MatShine"), 100.0f);
				glUniform3f(curS->getUniform("MatEmitt"), 1.0f, 0.8f, 0.0f);
				glUniform1f(curS->getUniform("MatEmittIntensity"), 0.5f);
				break;
		}
	}

	void SetMaterialSigns(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
				// Electric Pink
				glUniform3f(curS->getUniform("MatAmb"), 1.0f, 0.2f, 0.6f);
				glUniform3f(curS->getUniform("MatDif"), 1.0f, 0.2f, 0.6f);
				glUniform3f(curS->getUniform("MatSpec"), 1.0f, 0.2f, 0.6f);
				glUniform1f(curS->getUniform("MatShine"), 100.0f);
				glUniform3f(curS->getUniform("MatEmitt"), 1.0f, 0.2f, 0.6f);
				glUniform1f(curS->getUniform("MatEmittIntensity"), 0.7f);
				break;
			case 1:
				// Neon Green
				glUniform3f(curS->getUniform("MatAmb"), 0.0f, 1.0f, 0.3f);
				glUniform3f(curS->getUniform("MatDif"), 0.0f, 1.0f, 0.3f);
				glUniform3f(curS->getUniform("MatSpec"), 0.0f, 1.0f, 0.3f);
				glUniform1f(curS->getUniform("MatShine"), 100.0f);
				glUniform3f(curS->getUniform("MatEmitt"), 0.0f, 1.0f, 0.3f);
				glUniform1f(curS->getUniform("MatEmittIntensity"), 0.6f);
				break;
			case 2:
				// Vibrant Violet
				glUniform3f(curS->getUniform("MatAmb"), 0.6f, 0.2f, 1.0f);
				glUniform3f(curS->getUniform("MatDif"), 0.6f, 0.2f, 1.0f);
				glUniform3f(curS->getUniform("MatSpec"), 0.6f, 0.2f, 1.0f);
				glUniform1f(curS->getUniform("MatShine"), 100.0f);
				glUniform3f(curS->getUniform("MatEmitt"), 0.6f, 0.2f, 1.0f);
				glUniform1f(curS->getUniform("MatEmittIntensity"), 0.8f);
				break;
			case 3:
				// Bright Cyan
				glUniform3f(curS->getUniform("MatAmb"), 0.0f, 1.0f, 1.0f);
				glUniform3f(curS->getUniform("MatDif"), 0.0f, 1.0f, 1.0f);
				glUniform3f(curS->getUniform("MatSpec"), 0.0f, 1.0f, 1.0f);
				glUniform1f(curS->getUniform("MatShine"), 100.0f);
				glUniform3f(curS->getUniform("MatEmitt"), 0.0f, 1.0f, 1.0f);
				glUniform1f(curS->getUniform("MatEmittIntensity"), 0.7f);
				break;
		}
	}

	void SetMaterialBench(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
				// Wooden Bench
				glUniform3f(curS->getUniform("MatAmb"), 0.4f, 0.2f, 0.1f); // Warm, earthy tone
				glUniform3f(curS->getUniform("MatDif"), 0.6f, 0.3f, 0.2f); // Slightly lighter for diffuse
				glUniform3f(curS->getUniform("MatSpec"), 0.2f, 0.1f, 0.05f); // Low specular for less shine
				glUniform1f(curS->getUniform("MatShine"), 20.0f); // Subtle shine for a semi-polished look
				break;
		}
	}
	/* helper for sending top of the matrix strack to GPU */
	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }

	/* helper function to set model trasnforms */
  	void setModel(shared_ptr<Program> curS, vec3 trans, float rotY, float rotX, float sc) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void initBboxpos() {
		// lightcycle1_bbox_max += start_lightcycle1_pos;
		// lightcycle1_bbox_min += start_lightcycle1_pos;
	}

	float randFloat(float l, float h)
	{
		float r = rand() / (float) RAND_MAX;
		return (1.0f - r) * l + r * h;
	}


	bool isCollision(vec3 p1, vec3 p2) {
		return (p2.x - p1.x <= 0.001) && (p2.z - p1.z <= 0.001);
	}


	void render(float frametime, float animTime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		// Projection->perspective(45.0f, aspect, 0.01f, 200.0f);
		Projection->perspective(45.0f, aspect, 0.01f, 400.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
			View->loadIdentity();
			// View->translate(vec3(0, 0, -5));
			if ((currState == START && currViewState == START_VIEW) || (currState == START && currViewState == START_ANIM_VIEW)) {
				// rotate around the computer screen
				// View->translate(vec3(vTransx, vTransy, vTransz));
				// View->rotate(radians(vThetay), vec3(0.0f, 1.0f, 0.0f));
				// View->translate(vec3(-1.6, .2, -1.5));
				// lookAt = manTrans;
				View->lookAt(eye, lookAt, vec3(0, 1, 0));

			}
			// else if (currState == IN_DESKTOP && currViewState == IN_DESKTOP_VIEW) {
			// 	// for rotating camera around light cycle

			// 	View->lookAt(eye, lookAt, up);


			// }
		// Draw a solid colored sphere
		solidColorProg->bind();
		//send the projetion and view for solid shader
		glUniformMatrix4fv(solidColorProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(solidColorProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		//send in the color to use


		// different color for different objects


		prog2->bind();
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));


		if ((currState == START && currViewState == START_VIEW) || (currState == START && currViewState == START_ANIM_VIEW)) {
			glUniform3f(prog2->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // white light
			glUniform1f(prog2->getUniform("lightIntensity[0]"), 1.0); // light intensity
			glUniform3f(prog2->getUniform("lightPos[0]"), 1.51, .2, 1); // light position at the computer screen

			glUniform3f(prog2->getUniform("lightColor[1]"), 1.0, 1.0, 1.0); // white light
			glUniform1f(prog2->getUniform("lightIntensity[1]"), 1.0); // light intensity
			glUniform3f(prog2->getUniform("lightPos[1]"), 1.51, .2, 2.5); // light position at the computer screen

			glUniform1i(prog2->getUniform("numLights"), 2); // light position at the computer screen

			for (size_t i = 0; i < gaming_rooms.size(); ++i) {
				SetMaterialGamingRoom(prog2);
				setModel(prog2, vec3(1.5, -1.3, -1), 0, 0, 1);
				gaming_rooms[i]->draw(prog2);
			}

			for (size_t i = 0; i < gaming_chairs.size(); ++i) {
				SetMaterialGamingChair(prog2);
				setModel(prog2, vec3(1.5, -1.3, -1), 0, 0, 1);
				gaming_chairs[i]->draw(prog2);
			}

			for (size_t i = 0; i < room_lamps.size(); ++i) {
				SetMaterialGamingRoom(prog2);
				setModel(prog2, vec3(1.5, -1.3, -1), 0, 0, 1);
				room_lamps[i]->draw(prog2);
			}

			// for (size_t i = 0; i < gaming_monitors.size(); ++i) {
			// 	if ((gaming_monitors_names[i].find("Object_296") == std::string::npos) && (gaming_monitors_names[i].find("Object_256") == std::string::npos) && (gaming_monitors_names[i].find("Object_265") == std::string::npos)) {
			// 		SetMaterialMonitor(prog2);
			// 		setModel(prog2, vec3(1.5, -1.3, -1), 0, 0, 1);
			// 		gaming_monitors[i]->draw(prog2);
			// 	}
			// }
			for (size_t i = 0; i < gaming_monitors.size(); ++i) {
				SetMaterialMonitor(prog2);
				setModel(prog2, vec3(1.5, -1.3, -1), 0, 0, 1);
				gaming_monitors[i]->draw(prog2);
			}


		}



		prog2->unbind();

		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		if ((currState == START && currViewState == START_VIEW) || (currState == START && currViewState == START_ANIM_VIEW)) {

			glUniform3f(texProg->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // white light
			glUniform1f(texProg->getUniform("lightIntensity[0]"), 1.0); // light intensity
			glUniform3f(texProg->getUniform("lightPos[0]"), 1.51, .2, 1); // light position at the computer screen

			glUniform3f(texProg->getUniform("lightColor[1]"), 1.0, 1.0, 1.0); // white light
			glUniform1f(texProg->getUniform("lightIntensity[1]"), 1.0); // light intensity
			glUniform3f(texProg->getUniform("lightPos[1]"), 1.51, .2, 2.5); // light position at the computer screen

			glUniform1i(texProg->getUniform("numLights"), 2); // light position at the computer screen

			gamingroomwallTexture->bind(texProg->getUniform("Texture0"));
			for (size_t i = 0; i < gaming_room_walls.size(); ++i) {
				setModel(texProg, vec3(1.5, -1.3, -1), 0, 0, 1);
				gaming_room_walls[i]->draw(texProg);
			}
			gamingroomwallTexture->unbind();

			gamingroomfloorTexture->bind(texProg->getUniform("Texture0"));
			setModel(texProg, vec3(1.5, -1.3, -1), 0, 0, 1);
			gaming_room_floor->draw(texProg);
			gamingroomfloorTexture->unbind();

		}

		texProg->unbind();

		skyProg->bind();
		glUniformMatrix4fv(skyProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(skyProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		skyProg->unbind();

		assimptexProg->bind();
			glUniformMatrix4fv(assimptexProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glUniformMatrix4fv(assimptexProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));


			if ((currState == START && currViewState == START_VIEW) || (currState == START && currViewState == START_ANIM_VIEW)) {

				glUniform3f(assimptexProg->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // white light
				glUniform1f(assimptexProg->getUniform("lightIntensity[0]"), 1.0); // light intensity
				glUniform3f(assimptexProg->getUniform("lightPos[0]"), 1.51, .2, 10); // light position at the computer screen

				glUniform3f(assimptexProg->getUniform("lightColor[1]"), 1.0, 1.0, 1.0); // white light
				glUniform1f(assimptexProg->getUniform("lightIntensity[1]"), 1.0); // light intensity
				glUniform3f(assimptexProg->getUniform("lightPos[1]"), 1.51, .2, 10); // light position at the computer screen

				glUniform1i(assimptexProg->getUniform("numLights"), 2); // light position at the computer screen

				// if (startEverything) {
				// 	stickfigure_animator->UpdateAnimation(1.5 * animTime);
				// }
				// if (!startEverything) {
				// 	stickfigure_animator->UpdateAnimation(0);
				// }
				if (manState == WALKING) {
					stickfigure_animator->UpdateAnimation(1.5 * animTime);
				} else if (manState == STANDING) {
					stickfigure_animator->UpdateAnimation(0);
				}

				vector<glm::mat4> transforms = stickfigure_animator->GetFinalBoneMatrices();
				for (int i = 0; i < transforms.size(); ++i) {
					glUniformMatrix4fv(assimptexProg->getUniform("finalBonesMatrices[" + std::to_string(i) + "]"), 1, GL_FALSE, value_ptr(transforms[i]));
				}
				if (currState == START && sTransx < 1.8) {
						Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(manTrans);
							Model->scale(vec3(0.00055));
							// Model->rotate(radians(90.0f), vec3(0, 1, 0));
							Model->rotate(manRot.y, vec3(0, 1, 0));
							// Model->rotate(manRot.x, vec3(1, 0, 0));
							Model->rotate(manRot.z, vec3(0, 0, 1));
							glUniform1i(assimptexProg->getUniform("hasTexture"), 0);
							SetMaterialMan(assimptexProg, 0);
							setModel(assimptexProg, Model);
							stickfigure_running->Draw(assimptexProg);
						Model->popMatrix();
				}


		}

		assimptexProg->unbind();

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initBboxpos();

	auto lastTime = chrono::high_resolution_clock::now();

	glfwSetInputMode(windowManager->getHandle(), GLFW_STICKY_KEYS, GLFW_TRUE);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		auto nextLastTIme = chrono::high_resolution_clock::now();

		float deltaTime = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - lastTime).count();

		deltaTime *= 0.000001f; // convert to seconds

		lastTime = nextLastTIme;

		float AnimcurrFrame = glfwGetTime();
		application->AnimDeltaTime = AnimcurrFrame - application->AnimLastFrame;
		application->AnimLastFrame = AnimcurrFrame;
		// Render scene.
		application->render(deltaTime, application->AnimDeltaTime);

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <STB_IMAGE/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.h>
#include <camera.h>
#include <model.h>
#include <Primitives.h>
#include <Light.h>
#include <FrameBufferObject.h>
#include <VBOQuad.h>

#include <iostream>
#include <vector>

enum ShaderType
{
	Raymarch,
	RaymarchCell,
	RaymarchOutput,
	AbstractMarch,
	AbstractCell,
	AbstractOutput,
	Illuminance,
	END
};

class RenderEngine
{
private:
	

	Camera* m_sceneCamera;

	//Projection and View
	glm::mat4 m_captureProjectionMatrix;
	glm::mat4 m_projectionMatrix;
	//Different view matrices for each face of the cube
	glm::mat4 m_cubemapViewMatrices[6];

	//Primitives object renderer
	Primitives* m_PrimitiveObject;

	//Load Models here
	Model* m_model;
	VBOQuad* m_screenQuad;

	float m_screenWidth, m_screenHeight;

	//Runtime adjustments
	float m_displacement;
	float m_brightness;
	float m_contrast;
	float m_saturation;

	//Vector of lights
	std::vector<Light> m_Lights;

	//Shader Switcher
	ShaderType m_ShadingState;
	//Create Shaders
	Shader* m_Shader;
	Shader* m_borderlandShader;
	Shader* m_abstractShader;
	Shader* m_illuminanceShader;
	Shader* m_sobelShader;

	FrameBufferObject* m_screenBuffer;

	//Set Parameters for shaders
	void SetStaticParams(Shader* p_shader);
	void SetDynamicParams(Shader* p_shader);

	//Generate Textures
	unsigned int LoadTextures(const char * p_path);

public:
	RenderEngine(Camera* p_sceneCamera, float p_screenWidth, float p_screenHeight);

	void Render();
	void Update(float p_time, float p_frameTime, glm::vec2 p_mousePos);
	void CreateLights(glm::vec3 p_position, glm::vec3 p_colour);
	void increaseDisplacement(float p_floatValue);
	void increaseBrightness(float p_floatValue);
	void increaseContrast(float p_floatValue);
	void increaseSaturation(float p_floatValue);
	void resetWeights();
	void switchShader();


};
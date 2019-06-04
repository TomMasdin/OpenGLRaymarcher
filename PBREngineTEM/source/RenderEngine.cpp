#include "RenderEngine.h"


RenderEngine::RenderEngine(Camera * p_sceneCamera, float p_screenWidth, float p_screenHeight)
{
	//Global openGL states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.

	//Set the camera, width and height for framebuffer switching
	m_sceneCamera = p_sceneCamera;
	m_screenWidth = p_screenWidth;
	m_screenHeight = p_screenHeight;

	//Initialise the primitive renderer
	m_PrimitiveObject = new Primitives();
	m_screenQuad = new VBOQuad();

	//Init the Shader Type
	m_ShadingState = ShaderType::Raymarch;

	//Create Shaders
	m_Shader = new Shader("resources/shaders/raymarching.vs", "resources/shaders/raymarching.fs");
	m_borderlandShader = new Shader("resources/shaders/sobelDetection.vs", "resources/shaders/sobelDetection.fs");
	m_abstractShader = new Shader("resources/shaders/raymarching.vs", "resources/shaders/abstractRay.fs");
	m_illuminanceShader = new Shader("resources/shaders/contrastShader.vs", "resources/shaders/contrastShader.fs");
	m_sobelShader = new Shader("resources/shaders/sobelDetection.vs", "resources/shaders/sobelTest.fs");
	//Set Static Params here
	m_borderlandShader->Use();
	m_borderlandShader->SetInt("screenTexture", 0);
	m_borderlandShader->SetFloat("width", m_screenWidth);
	m_borderlandShader->SetFloat("height", m_screenHeight);

	m_sobelShader->Use();
	m_sobelShader->SetInt("screenTexture", 0);
	m_sobelShader->SetFloat("width", m_screenWidth);
	m_sobelShader->SetFloat("height", m_screenHeight);

	m_illuminanceShader->Use();
	m_illuminanceShader->SetInt("screenTexture", 0);

	//Sort the framebuffer
	m_screenBuffer = new FrameBufferObject(m_screenWidth, m_screenHeight, m_screenWidth, m_screenHeight);
	m_displacement = 1.0;
	m_brightness = 1.2;
	m_contrast = 0.1;
	m_saturation = 1.6;

	//Reset the viewport to the window
	glViewport(0, 0, m_screenWidth, m_screenHeight);
}

void RenderEngine::SetStaticParams(Shader* p_shader)
{
	m_projectionMatrix = glm::perspective(glm::radians(m_sceneCamera->Zoom), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);
	p_shader->Use();
	p_shader->SetMat4("projection", m_projectionMatrix);
}

void RenderEngine::SetDynamicParams(Shader* p_shader)
{
	glm::mat4 view = m_sceneCamera->GetViewMatrix();
	p_shader->Use();
	p_shader->SetMat4("view", view);
	p_shader->SetVec3("camPos", m_sceneCamera->Position);
}

unsigned int RenderEngine::LoadTextures(const char * p_path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(p_path, &width, &height, &nrComponents, 0);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << p_path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void RenderEngine::Render()
{
	glClearColor(0.9f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	switch (m_ShadingState)
	{
	case Raymarch:
		m_Shader->Use();
		m_Shader->SetVec2("iResolution", glm::vec2(m_screenWidth, m_screenHeight));
		m_Shader->SetVec3("cam_pos", m_sceneCamera->Position);
		m_Shader->SetFloat("displacementMultiplier", m_displacement);
		m_screenQuad->Render();
		break;
	case RaymarchCell:
		m_screenBuffer->BindFrameBuffer();
		glClear(GL_COLOR_BUFFER_BIT);
		m_Shader->Use();
		m_Shader->SetVec2("iResolution", glm::vec2(m_screenWidth, m_screenHeight));
		m_Shader->SetVec3("cam_pos", m_sceneCamera->Position);
		m_Shader->SetFloat("displacementMultiplier", m_displacement);
		m_screenQuad->Render();
		m_screenBuffer->UnbindFrameBuffer();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_screenBuffer->GetColourTexture());

		m_borderlandShader->Use();
		m_screenQuad->Render();
		break;
	case RaymarchOutput:
		m_screenBuffer->BindFrameBuffer();
		glClear(GL_COLOR_BUFFER_BIT);
		m_Shader->Use();
		m_Shader->SetVec2("iResolution", glm::vec2(m_screenWidth, m_screenHeight));
		m_Shader->SetVec3("cam_pos", m_sceneCamera->Position);
		m_Shader->SetFloat("displacementMultiplier", m_displacement);
		m_screenQuad->Render();
		m_screenBuffer->UnbindFrameBuffer();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_screenBuffer->GetColourTexture());

		m_sobelShader->Use();
		m_screenQuad->Render();
		break;
	case AbstractMarch:
		m_abstractShader->Use();
		m_abstractShader->SetVec2("iResolution", glm::vec2(m_screenWidth, m_screenHeight));
		m_abstractShader->SetVec3("cam_pos", m_sceneCamera->Position);
		m_abstractShader->SetFloat("displacementMultiplier", m_displacement);
		m_screenQuad->Render();
		break;
	case AbstractCell:
		m_screenBuffer->BindFrameBuffer();
		m_abstractShader->Use();
		m_abstractShader->SetVec2("iResolution", glm::vec2(m_screenWidth, m_screenHeight));
		m_abstractShader->SetVec3("cam_pos", m_sceneCamera->Position);
		m_abstractShader->SetFloat("displacementMultiplier", m_displacement);
		m_screenQuad->Render();
		m_screenBuffer->UnbindFrameBuffer();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_screenBuffer->GetColourTexture());

		m_borderlandShader->Use();
		m_screenQuad->Render();
		break;
	case AbstractOutput:
		m_screenBuffer->BindFrameBuffer();
		m_abstractShader->Use();
		m_abstractShader->SetVec2("iResolution", glm::vec2(m_screenWidth, m_screenHeight));
		m_abstractShader->SetVec3("cam_pos", m_sceneCamera->Position);
		m_abstractShader->SetFloat("displacementMultiplier", m_displacement);
		m_screenQuad->Render();
		m_screenBuffer->UnbindFrameBuffer();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_screenBuffer->GetColourTexture());

		m_sobelShader->Use();
		m_screenQuad->Render();
		break;
	case Illuminance:
		m_screenBuffer->BindFrameBuffer();
		m_abstractShader->Use();
		m_abstractShader->SetVec2("iResolution", glm::vec2(m_screenWidth, m_screenHeight));
		m_abstractShader->SetVec3("cam_pos", m_sceneCamera->Position);
		m_abstractShader->SetFloat("displacementMultiplier", m_displacement);
		m_screenQuad->Render();
		m_screenBuffer->UnbindFrameBuffer();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_screenBuffer->GetColourTexture());

		m_illuminanceShader->Use();
		m_illuminanceShader->SetFloat("brightness", m_brightness);
		m_illuminanceShader->SetFloat("contrast", m_contrast);
		m_illuminanceShader->SetFloat("saturation", m_saturation);
		m_screenQuad->Render();
		break;
	}
	
	glEnable(GL_DEPTH_TEST);
}

void RenderEngine::Update(float p_time, float p_frameTime, glm::vec2 p_mousePos)
{
	m_Shader->Use();
	m_Shader->SetFloat("iTime", p_time);
	m_Shader->SetInt("iFrame", (int)p_time);
	m_Shader->SetVec2("iMouse", p_mousePos);

	m_abstractShader->Use();
	m_abstractShader->SetFloat("iTime", p_time);
	m_abstractShader->SetInt("iFrame", (int)p_time);
	m_abstractShader->SetVec2("iMouse", p_mousePos);
}

void RenderEngine::CreateLights(glm::vec3 p_position, glm::vec3 p_colour)
{
	m_Lights.push_back(Light(p_position, p_colour));
}

void RenderEngine::increaseDisplacement(float p_floatValue)
{
	m_displacement += p_floatValue;
	std::cout << "Displacement Value: "<< m_displacement << std::endl;
}

void RenderEngine::increaseBrightness(float p_floatValue)
{
	m_brightness += p_floatValue;
	std::cout << "Brightness Value: " << m_brightness << std::endl;
}

void RenderEngine::increaseContrast(float p_floatValue)
{
	m_contrast += p_floatValue;
	std::cout << "Contrast Value: " << m_contrast << std::endl;
}

void RenderEngine::increaseSaturation(float p_floatValue)
{
	m_saturation += p_floatValue;
	std::cout << "Saturation Value: " << m_saturation << std::endl;
}

void RenderEngine::resetWeights()
{
	m_displacement = 1.0;
	m_brightness = 1.2;
	m_contrast = 0.1;
	m_saturation = 1.6;
}

void RenderEngine::switchShader()
{
	m_ShadingState = static_cast<ShaderType>(static_cast<int>(m_ShadingState) + 1);
	if (m_ShadingState == ShaderType::END)
	{
		m_ShadingState = ShaderType::Raymarch;
	}
}

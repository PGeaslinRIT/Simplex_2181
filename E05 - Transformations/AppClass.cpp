#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	m_pMesh = new MyMesh();
	m_pMesh->GenerateCube(1.0f, C_BLACK);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	//Calculate the model, view and projection matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	matrix4 m4View = m_pCameraMngr->GetViewMatrix();

	static float horizontal = -15.0f;
	static float vertical = 6.0f;
	matrix4 m4Scale = IDENTITY_M4;
	matrix4 m4Translate;
	matrix4 m4Model;

	bool invaderTemplate[8][11] = {
		{ 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
		{ 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
		{ 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1 },
		{ 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1 },
		{ 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0 } };

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 11; j++)
		{
			if (invaderTemplate[i][j])
			{
				m4Translate = glm::translate(m4Scale, vector3(horizontal + j, vertical - i, 0.0f));
				m4Model = m4Translate;

				m_pMesh->Render(m4Projection, m4View, m4Model);
			}
		}
	}

	/*m4Translate = glm::translate(m4Scale, vector3(value, 0.0f, 0.0f));
	m4Model = m4Translate;

	m_pMesh->Render(m4Projection, m4View, m4Model);*/


	horizontal += 0.01f;
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}
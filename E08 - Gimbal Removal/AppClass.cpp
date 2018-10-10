#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	m_pMesh = new MyMesh();
	//m_pMesh->GenerateCube(1.0f, C_WHITE);
	m_pMesh->GenerateCone(2.0f, 5.0f, 3, C_WHITE);
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

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	m_qOrientation = vector3(0.0f), 0.0f;

	static quaternion curOrientation = glm::angleAxis(0.0f, m_v3Rotation);
	quaternion oldOr = curOrientation;
	
	m_m4Model = glm::rotate(IDENTITY_M4, glm::radians(m_v3Rotation.x), vector3(1.0f, 0.0f, 0.0f));
	m_m4Model = glm::rotate(m_m4Model, glm::radians(m_v3Rotation.y), vector3(0.0f, 1.0f, 0.0f));
	m_m4Model = glm::rotate(m_m4Model, glm::radians(m_v3Rotation.z), vector3(0.0f, 0.0f, 1.0f));
	//m_pMesh->Render(m4Projection, m4View, ToMatrix4(m_m4Model));
	float angle = acos(glm::radians((m_v3Rotation.x + m_v3Rotation.y + m_v3Rotation.z)-1 / 2));
	float xr, yr, zr;
	if (sin(angle) == 0)
	{
		xr = 0.0f;
		yr = 0.0f;
		zr = 0.0f;
	}
	else {
		xr = (m_m4Model[2].z - m_m4Model[3].y) / (2 * sin(angle));
		yr = (m_m4Model[3].x - m_m4Model[1].z) / (2 * sin(angle));
		zr = (m_m4Model[1].y - m_m4Model[2].x) / (2 * sin(angle));
	}
	quaternion q1 = glm::angleAxis(angle, vector3(xr,yr,zr));
	//quaternion q1 = glm::angleAxis(glm::radians(m_v3Rotation.z), AXIS_Z) * glm::angleAxis(glm::radians(m_v3Rotation.x), AXIS_X) * glm::angleAxis(glm::radians(m_v3Rotation.y), AXIS_Y);
	//quaternion q2 = glm::angleAxis(glm::radians(m_v3Rotation.x), AXIS_X);
	//quaternion q3 = glm::angleAxis(glm::radians(m_v3Rotation.y), AXIS_Y);

	curOrientation = oldOr * q1;

	float total = (curOrientation.w * curOrientation.w) + (curOrientation.x * curOrientation.x) + (curOrientation.y * curOrientation.y) + (curOrientation.z * curOrientation.z);
	curOrientation.w = (curOrientation.w / total);
	curOrientation.x = (curOrientation.x / total);
	curOrientation.y = (curOrientation.y / total);
	curOrientation.z = (curOrientation.z / total);
	m_pMesh->Render(m4Projection, m4View, ToMatrix4(curOrientation));
	
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
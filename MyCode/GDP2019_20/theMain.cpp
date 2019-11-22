#include "GLCommon.h"
#include <Windows.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>		// c libs
#include <stdio.h>		// c libs

#include <iostream>		// C++ IO standard stuff
#include <map>			// Map aka "dictonary" 

#include "cModelLoader.h"			
#include "cVAOManager.h"		// NEW
//#include "cGameObject.h"

#include "cShaderManager.h"

#include <sstream>
#include <fstream>

#include <limits.h>
#include <float.h>

// The Physics function
#include "PhysicsStuff.h"
#include "cPhysics.h"

#include "DebugRenderer/cDebugRenderer.h"
#include <pugixml/pugixml.hpp>
#include <pugixml/pugixml.cpp>
#include "cLight.h"
#include "cMediator.h"
#include "cObjectFactory.h"

// Used to visualize the attenuation of the lights...
#include "LightManager/cLightHelper.h"

using namespace pugi;

xml_document document;
std::string gameDataLocation = "gameData.xml";
xml_parse_result result = document.load_file(gameDataLocation.c_str());
std::ofstream file;
xml_node root_node = document.child("GameData");
xml_node lightData = root_node.child("LightData");
xml_node rampData = root_node.child("RampData");
xml_node ballData = root_node.child("BallData");
xml_node ballLightData = root_node.child("BallLightData");

bool fileChanged = false;

void DrawObject(glm::mat4 m, iObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);

//glm::vec3 borderLight3 = glm::vec3(0.0f, -149.0f, 0.0f);
//glm::vec3 borderLight4 = glm::vec3(0.0f, 200.0f, 0.0f);
//glm::vec3 borderLight5 = glm::vec3(0.0f, 0.0f, -199.0f);
//glm::vec3 borderLight6 = glm::vec3(0.0f, 0.0f, 199.0f);

cMediator* pMediator = cMediator::createMediator();

unsigned int currentRamp = 0;

cLight* pMainLight = new cLight();
unsigned int currentLight = 0;
cLight* pCorner1Light = new cLight();
cLight* pCorner2Light = new cLight();
cLight* pCorner3Light = new cLight();
cLight* pCorner4Light = new cLight();

float cameraLeftRight = 0.0f;

glm::vec3 cameraEye = glm::vec3(0.0, 120.0, -75.0);
glm::vec3 cameraTarget = glm::vec3(pMainLight->getPositionX(), pMainLight->getPositionY(), pMainLight->getPositionZ());
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

float SpotInnerAngle1 = 5.0f;
float cornerLightSpotOuterAngle1 = 7.5f;

//mainLight
// This is a "normalized" direction
// (i.e. the length is 1.0f)

bool bLightDebugSheresOn = false;


bool onGround = false;
bool onPlatform = false;

glm::mat4 calculateWorldMatrix(iObject* pCurrentObject);


// Load up my "scene"  (now global)
std::vector<iObject*> g_vec_pGameObjects;
std::vector<iObject*> g_vec_pWallObjects;
std::vector<iObject*> g_vec_pRampObjects;
std::vector<iObject*> g_vec_pBallObjects;
std::map<std::string /*FriendlyName*/, iObject*> g_map_GameObjectsByFriendlyName;


// returns NULL (0) if we didn't find it.
iObject* pFindObjectByFriendlyName(std::string name);
iObject* pFindObjectByFriendlyNameMap(std::string name);

//bool g_BallCollided = false;

bool isShiftKeyDownByAlone(int mods)
{
	if (mods == GLFW_MOD_SHIFT)
	{
		// Shift key is down all by itself
		return true;
	}
	return false;
}

bool isCtrlKeyDownByAlone(int mods)
{
	if (mods == GLFW_MOD_CONTROL)
	{
		return true;
	}
	return false;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	const float CAMERASPEED = 1.0f;
	const float MOVESPEED = 5.0f;

	if (!isShiftKeyDownByAlone(mods) && !isCtrlKeyDownByAlone(mods))
	{
		//Reset positions
		if (key == GLFW_KEY_R)
		{
			pMainLight->setPositionX(0.0f);
			pMainLight->setPositionY(10.0f);
			pMainLight->setPositionZ(0.0f);
			cameraEye = glm::vec3(0.0, 30.0, -100.0);
		}

		// Move the camera (A & D for left and right, along the x axis)
		if (key == GLFW_KEY_A)
		{
			cameraEye.x -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_D)
		{
			cameraEye.x += CAMERASPEED;		// Move the camera +0.01f units
		}

		// Move the camera (Q & E for up and down, along the y axis)
		if (key == GLFW_KEY_Q)
		{
			cameraEye.y -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_E)
		{
			cameraEye.y += CAMERASPEED;		// Move the camera +0.01f units
		}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_W)
		{
			cameraEye.z -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_S)
		{
			cameraEye.z += CAMERASPEED;		// Move the camera +0.01f units
		}

		if (key == GLFW_KEY_B)
		{
//			// Shoot a bullet from the pirate ship
//			// Find the pirate ship...
//			// returns NULL (0) if we didn't find it.
////			cGameObject* pShip = pFindObjectByFriendlyName("PirateShip");
//			iObject* pShip = pFindObjectByFriendlyNameMap("PirateShip");
//			// Maybe check to see if it returned something... 
//
//			// Find the sphere#2
////			cGameObject* pBall = pFindObjectByFriendlyName("Sphere#2");
//			iObject* pBall = pFindObjectByFriendlyNameMap("Sphere#2");
//
//			// Set the location velocity for sphere#2
//			pBall->positionXYZ = pShip->positionXYZ;
//			pBall->inverseMass = 1.0f;		// So it's updated
//			// 20.0 units "to the right"
//			// 30.0 units "up"
//			pBall->velocity = glm::vec3(15.0f, 20.0f, 0.0f);
//			pBall->accel = glm::vec3(0.0f, 0.0f, 0.0f);
//			pBall->diffuseColour = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		}//if ( key == GLFW_KEY_B )

	}

	if (isShiftKeyDownByAlone(mods))
	{
		// switch lights to control
		//if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		//{
		//	if (currentRamp == g_vec_pRampObjects.size() - 1)
		//	{
		//		currentRamp = 0;
		//	}
		//	else
		//	{
		//		currentRamp++;
		//	}
		//}
		//if (key == GLFW_KEY_BACKSLASH && action == GLFW_PRESS)
		//{
		//	if (currentRamp == 0)
		//	{
		//		currentRamp = g_vec_pRampObjects.size() - 1;
		//	}
		//	else
		//	{
		//		currentRamp--;
		//	}
		//}
		//// move the ramp
		//if (key == GLFW_KEY_A)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->positionXYZ.x += CAMERASPEED;
		//}
		//if (key == GLFW_KEY_D)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->positionXYZ.x -= CAMERASPEED;
		//}

		//// Move the camera (Q & E for up and down, along the y axis)
		//if (key == GLFW_KEY_Q)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->positionXYZ.y += CAMERASPEED;
		//}
		//if (key == GLFW_KEY_E)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->positionXYZ.y -= CAMERASPEED;
		//}

		//// Move the camera (W & S for towards and away, along the z axis)
		//if (key == GLFW_KEY_W)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->positionXYZ.z += CAMERASPEED;
		//}
		//if (key == GLFW_KEY_S)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->positionXYZ.z -= CAMERASPEED;
		//}

		////Rotate the ramp
		//if (key == GLFW_KEY_F)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->rotationXYZ.x += CAMERASPEED / 5.0f;
		//}
		//if (key == GLFW_KEY_H)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->rotationXYZ.x -= CAMERASPEED / 5.0f;
		//}

		//// Move the camera (Q & E for up and down, along the y axis)
		//if (key == GLFW_KEY_R)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->rotationXYZ.y += CAMERASPEED / 5.0f;
		//}
		//if (key == GLFW_KEY_Y)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->rotationXYZ.y -= CAMERASPEED / 5.0f;
		//}

		//// Move the camera (W & S for towards and away, along the z axis)
		//if (key == GLFW_KEY_T)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->rotationXYZ.z += CAMERASPEED / 5.0f;
		//}
		//if (key == GLFW_KEY_G)
		//{
		//	g_vec_pRampObjects.at(currentRamp)->rotationXYZ.z -= CAMERASPEED / 5.0f;
		//}

		//if (key == GLFW_KEY_K)
		//{
		//	int index = 0;
		//	for (xml_node ramp = rampData.first_child(); ramp; ramp = ramp.next_sibling())
		//	{
		//		xml_node changeData = ramp.first_child();
		//		changeData.last_child().set_value(std::to_string(g_vec_pRampObjects.at(index)->positionXYZ.x).c_str());
		//		changeData = changeData.next_sibling();
		//		changeData.last_child().set_value(std::to_string(g_vec_pRampObjects.at(index)->positionXYZ.y).c_str());
		//		changeData = changeData.next_sibling();
		//		changeData.last_child().set_value(std::to_string(g_vec_pRampObjects.at(index)->positionXYZ.z).c_str());
		//		changeData = changeData.next_sibling();
		//		changeData.last_child().set_value(std::to_string(g_vec_pRampObjects.at(index)->rotationXYZ.x).c_str());
		//		changeData = changeData.next_sibling();
		//		changeData.last_child().set_value(std::to_string(g_vec_pRampObjects.at(index)->rotationXYZ.y).c_str());
		//		changeData = changeData.next_sibling();
		//		changeData.last_child().set_value(std::to_string(g_vec_pRampObjects.at(index)->rotationXYZ.z).c_str());
		//		index++;
		//	}
		//	fileChanged = true;
		//}

		//if (key == GLFW_KEY_V)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition -= 0.1f;
		//}
		//if (key == GLFW_KEY_B)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition += 0.1f;
		//}
		//if (key == GLFW_KEY_N)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition -= 0.1f;
		//}
		//if (key == GLFW_KEY_M)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition += 0.1f;
		//}


		if (key == GLFW_KEY_9)
		{
			bLightDebugSheresOn = false;
		}
		if (key == GLFW_KEY_0)
		{
			bLightDebugSheresOn = true;
		}
		// switch lights to control
		if (key == GLFW_KEY_M)
		{
			currentLight = 0;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_1)
		{
			currentLight = 1;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_2)
		{
			currentLight = 2;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_3)
		{
			currentLight = 3;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_4)
		{
			currentLight = 4;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_5)
		{
			currentLight = 5;		// Move the camera -0.01f units
		}
		// move the light
		if (key == GLFW_KEY_A)
		{
			sNVPair message;
			message.name = "Position X Down";
			message.fValue = CAMERASPEED;
			pMediator->m_vec_pLights.at(currentLight)->RecieveMessage(message);		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_D)
		{
			sNVPair message;
			message.name = "Position X Up";
			message.fValue = CAMERASPEED;
			pMediator->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		}

		// Move the camera (Q & E for up and down, along the y axis)
		if (key == GLFW_KEY_Q)
		{
			sNVPair message;
			message.name = "Position Y Down";
			message.fValue = CAMERASPEED;
			pMediator->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		}
		if (key == GLFW_KEY_E)
		{
			sNVPair message;
			message.name = "Position Y Up";
			message.fValue = CAMERASPEED;
			pMediator->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_W)
		{
			sNVPair message;
			message.name = "Position Z Down";
			message.fValue = CAMERASPEED;
			pMediator->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		}
		if (key == GLFW_KEY_S)
		{
			sNVPair message;
			message.name = "Position Z Up";
			message.fValue = CAMERASPEED;
			pMediator->m_vec_pLights.at(currentLight)->RecieveMessage(message);
		}
		if (key == GLFW_KEY_I && action == GLFW_PRESS)
		{
			for (int i = 0; i < pMediator->m_vec_pLights.size(); i++)
			{
				pMediator->m_vec_pLights.at(i)->setLinearAtten(1.0f);
			}
		}
		if (key == GLFW_KEY_N && action == GLFW_PRESS)
		{
			for (int i = 0; i < pMediator->m_vec_pLights.size(); i++)
			{
				pMediator->m_vec_pLights.at(i)->setLinearAtten(0.003517f);
			}
		}

		if (key == GLFW_KEY_K)
		{
			for (int i = 0; i < pMediator->m_vec_pLights.size(); i++)
			{
				std::string currentNodeName = pMediator->m_vec_pLights.at(i)->getNodeName();
				xml_node LightToChange = lightData.child(currentNodeName.c_str());
				std::vector<std::string> changeData = pMediator->m_vec_pLights.at(i)->getAllDataStrings();
				//int index = 0;
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionX()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionY()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionZ()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getConstAtten()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getLinearAtten()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getQuadraticAtten()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getInnerSpot()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getOuterSpot()));
				//Set data to xml to set positions

				int index = 0;
				for (xml_node dataNode = LightToChange.child("PositionX"); dataNode; dataNode = dataNode.next_sibling())
				{
					//LightToChange->first_node("PositionX")->value(changeData.at(i).c_str());
					//LightToChange->first_node("PositionY")->value(changeData.at(i).c_str());
					//LightToChange->first_node("PositionZ")->value(changeData.at(i).c_str());
					//LightToChange->first_node("ConstAtten")->value(changeData.at(i).c_str());
					//LightToChange->first_node("LinearAtten")->value(changeData.at(i).c_str());
					//LightToChange->first_node("QuadraticAtten")->value(changeData.at(i).c_str());
					//LightToChange->first_node("SpotInnerAngle")->value(changeData.at(i).c_str());
					//LightToChange->first_node("SpotOuterAngle")->value(changeData.at(i).c_str());

					//std::string changeString = changeData.at(index);
					//std::cout << changeString << std::endl;
					dataNode.last_child().set_value(changeData.at(index).c_str());
					//std::cout << dataNode->value() << std::endl;
					//dataNode = dataNode->next_sibling();
					index++;
				}
				//for (xml_node<>* dataNode = LightToChange->first_node(); dataNode; dataNode = dataNode->next_sibling())
				//{
				//	//assert(index < changeData.size());
				//	const char * stringToChange = changeData.at(index).c_str();
				//	dataNode->value(stringToChange);
				//	file.open(gameDataLocation);
				//	file << "<?xml version='1.0' encoding='utf-8'?>\n";
				//	file << document;
				//	file.close();
				//	index++;
				//}
			}
			fileChanged = true;
		}
		//if (key == GLFW_KEY_V)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition -= 0.1f;
		//}
		//if (key == GLFW_KEY_B)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition += 0.1f;
		//}
		//if (key == GLFW_KEY_N)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition -= 0.1f;
		//}
		//if (key == GLFW_KEY_M)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition += 0.1f;
		//}


		if (key == GLFW_KEY_9)
		{
			bLightDebugSheresOn = false;
		}
		if (key == GLFW_KEY_0)
		{
			bLightDebugSheresOn = true;
		}

	}//if (isShiftKeyDownByAlone(mods))

	if (isCtrlKeyDownByAlone(mods))
	{
		// move the shpere
		iObject* pSphere = pFindObjectByFriendlyName("Sphere#1");
		if (key == GLFW_KEY_D)
		{
			//pSphere->rotationXYZ -= glm::vec3(CAMERASPEED, 0.0f, 0.0f);
			pSphere->setVelocity(pSphere->getVelocity() - glm::vec3(MOVESPEED, 0.0f, 0.0f));		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_A)
		{
			//pSphere->rotationXYZ += glm::vec3(CAMERASPEED, 0.0f, 0.0f);
			pSphere->setVelocity(pSphere->getVelocity() + glm::vec3(MOVESPEED, 0.0f, 0.0f));		// Move the camera +0.01f units
		}

		if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			if (pSphere->getIsWireframe())
			{
				pSphere->setIsWireframe(false);
			}
			else
			{
				pSphere->setIsWireframe(true);
			}
		}

		// Move the camera (Q & E for up and down, along the y axis)
		//if (key == GLFW_KEY_Q)
		//{
		//	pSphere->velocity -= glm::vec3(0.0f, CAMERASPEED, 0.0f);		// Move the camera -0.01f units
		//}
		//if (key == GLFW_KEY_E)
		//{
		//	pSphere->velocity += glm::vec3(0.0f, CAMERASPEED, 0.0f);		// Move the camera +0.01f units
		//	
		//}
		//if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		//{
		//	if (onGround)
		//	{
		//		pSphere->velocity.y = 10.0f;
		//	}
		//}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_S)
		{
			pSphere->setVelocity(pSphere->getVelocity() - glm::vec3(0.0f, 0.0f, MOVESPEED));		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_W)
		{
			pSphere->setVelocity(pSphere->getVelocity() + glm::vec3(0.0f, 0.0f, MOVESPEED));		// Move the camera +0.01f units
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			pSphere->setVelocity(pSphere->getVelocity() + glm::vec3(MOVESPEED, 0.0f, 0.0f));
			pSphere->setVelocity(pSphere->getVelocity() + glm::vec3(0.0f, 0.0f, MOVESPEED));
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			pSphere->setVelocity(pSphere->getVelocity() - glm::vec3(MOVESPEED, 0.0f, 0.0f));
			pSphere->setVelocity(pSphere->getVelocity() + glm::vec3(0.0f, 0.0f, MOVESPEED));
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			pSphere->setVelocity(pSphere->getVelocity() + glm::vec3(MOVESPEED, 0.0f, 0.0f));
			pSphere->setVelocity(pSphere->getVelocity() - glm::vec3(0.0f, 0.0f, MOVESPEED));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			pSphere->setVelocity(pSphere->getVelocity() - glm::vec3(0.0f, 0.0f, MOVESPEED));
			pSphere->setVelocity(pSphere->getVelocity() - glm::vec3(MOVESPEED, 0.0f, 0.0f));
		}

		if (key == GLFW_KEY_1)
		{
			pMediator->m_vec_pLights.at(currentLight)->setConstAtten(pMediator->m_vec_pLights.at(currentLight)->getConstAtten() * 0.99f);			// 99% of what it was
		}
		if (key == GLFW_KEY_2)
		{
			pMediator->m_vec_pLights.at(currentLight)->setConstAtten(pMediator->m_vec_pLights.at(currentLight)->getConstAtten() * 1.01f);
		}
		if (key == GLFW_KEY_3)
		{
			pMediator->m_vec_pLights.at(currentLight)->setLinearAtten(pMediator->m_vec_pLights.at(currentLight)->getLinearAtten() * 0.99f);			// 99% of what it was
		}
		if (key == GLFW_KEY_4)
		{
			pMediator->m_vec_pLights.at(currentLight)->setLinearAtten(pMediator->m_vec_pLights.at(currentLight)->getLinearAtten() * 1.01f);			// 1% more of what it was
		}
		if (key == GLFW_KEY_5)
		{
			pMediator->m_vec_pLights.at(currentLight)->setQuadraticAtten(pMediator->m_vec_pLights.at(currentLight)->getQuadraticAtten() * 0.99f);
		}
		if (key == GLFW_KEY_6)
		{
			pMediator->m_vec_pLights.at(currentLight)->setQuadraticAtten(pMediator->m_vec_pLights.at(currentLight)->getQuadraticAtten() * 1.01f);
		}

		//cGameObject* pShip = pFindObjectByFriendlyName("PirateShip");
		//// Turn the ship around
		//if (key == GLFW_KEY_A)
		//{	// Left
		//	pShip->HACK_AngleAroundYAxis -= 0.01f;
		//	pShip->rotationXYZ.y = pShip->HACK_AngleAroundYAxis;
		//}
		//if (key == GLFW_KEY_D)
		//{	// Right
		//	pShip->HACK_AngleAroundYAxis += 0.01f;
		//	pShip->rotationXYZ.y = pShip->HACK_AngleAroundYAxis;
		//}
		//if (key == GLFW_KEY_W)
		//{	// Faster
		//	pShip->HACK_speed += 0.1f;
		//}
		//if (key == GLFW_KEY_S)
		//{	// Slower
		//	pShip->HACK_speed -= 0.1f;
		//}
	}

	if (isCtrlKeyDownByAlone(mods) && isShiftKeyDownByAlone(mods))
	{

	}


	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}




int main(void)
{
	//std::ifstream gameData(gameDataLocation);


	//std::vector<char> buffer((std::istreambuf_iterator<char>(gameData)), std::istreambuf_iterator<char>());
	//buffer.push_back('\0');

	//document.parse<0>(&buffer[0]);
	//root_node = document.first_node("GameData");
	//lightData = root_node->first_node("LightData");

	//root_node = document.child("GameData");
	//lightData = root_node.child("LightData");

	cModelLoader* pTheModelLoader = new cModelLoader();	// Heap

	cObjectFactory* pFactory = new cObjectFactory();

	std::string objectPath;
	std::ifstream objectLocation("objectPaths.txt");
	std::vector<cMesh> meshVec;
	if (objectLocation.is_open())
	{
		while (getline(objectLocation, objectPath))
		{
			cMesh theMesh;
			pTheModelLoader->LoadPlyModel(objectPath, theMesh);
			meshVec.push_back(theMesh);
		}
		objectLocation.close();
	}

	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);



	cDebugRenderer* pDebugRenderer = new cDebugRenderer();
	pDebugRenderer->initialize();

	//	cMesh bunnyMesh;		// This is stack based
	////	if ( ! pTheModelLoader->LoadPlyModel("assets/models/Sky_Pirate_Combined_xyz.ply", bunnyMesh) )
	////	if ( ! pTheModelLoader->LoadPlyModel("assets/models/bun_zipper_res4_XYZ_N.ply", bunnyMesh) )
	//	if ( ! pTheModelLoader->LoadPlyModel("assets/models/bun_zipper_XYZ_n.ply", bunnyMesh) )
	//	{
	//		std::cout << "Didn't find the file" << std::endl;
	//	}
	//
	//	cMesh pirateMesh;
	//	pTheModelLoader->LoadPlyModel("assets/models/Sky_Pirate_Combined_xyz_n.ply", pirateMesh);
	//
	//	cMesh terrainMesh;
	//	pTheModelLoader->LoadPlyModel("assets/models/Terrain_XYZ_n.ply", terrainMesh);
	////	pTheModelLoader->LoadPlyModel("assets/models/BigFlatTerrain_XYZ_n.ply", terrainMesh);
	//
	//	cMesh cubeMesh;
	////	pTheModelLoader->LoadPlyModel("assets/models/Cube_1_Unit_from_origin_XYZ_n.ply", cubeMesh);
	////
	////	cMesh sphereMesh;
	////	pTheModelLoader->LoadPlyModel("assets/models/Sphere_Radius_1_XYZ_n.ply", sphereMesh);
	////
	////	cMesh borderMesh;
	////	pTheModelLoader->LoadPlyModel("assets/models/plain_box_XYZ_n.ply", borderMesh);
	//
	//	//cMesh borderMeshBottom;
	//	//pTheModelLoader->LoadPlyModel("assets/models/plain_box_XYZ_n.ply", borderMeshBottom);
	//
	//	//cMesh borderMeshLeft;
	//	//pTheModelLoader->LoadPlyModel("assets/models/plain_box_XYZ_n.ply", borderMeshLeft);
	//
	//	//cMesh borderMeshRight;
	//	//pTheModelLoader->LoadPlyModel("assets/models/plain_box_XYZ_n.ply", borderMeshRight);
	//
	//	//cMesh borderMeshFront;
	//	//pTheModelLoader->LoadPlyModel("assets/models/plain_box_XYZ_n.ply", borderMeshFront);
	//
	//	//cMesh borderMeshBack;
	//	//pTheModelLoader->LoadPlyModel("assets/models/plain_box_XYZ_n.ply", borderMeshBack);

	cShaderManager* pTheShaderManager = new cShaderManager();

	cShaderManager::cShader vertexShad;
	vertexShad.fileName = "assets/shaders/vertexShader01.glsl";

	cShaderManager::cShader fragShader;
	fragShader.fileName = "assets/shaders/fragmentShader01.glsl";

	if (!pTheShaderManager->createProgramFromFile("SimpleShader", vertexShad, fragShader))
	{
		std::cout << "Error: didn't compile the shader" << std::endl;
		std::cout << pTheShaderManager->getLastError();
		return -1;
	}

	GLuint shaderProgID = pTheShaderManager->getIDFromFriendlyName("SimpleShader");


	// Create a VAO Manager...
	// #include "cVAOManager.h"  (at the top of your file)
	cVAOManager* pTheVAOManager = new cVAOManager();

	// Note, the "filename" here is really the "model name" 
	//  that we can look up later (i.e. it doesn't have to be the file name)
	std::string meshName;
	std::ifstream meshNameFile("meshNames.txt");
	std::vector<std::string> nameVec;
	if (meshNameFile.is_open())
	{
		while (getline(meshNameFile, meshName))
		{
			nameVec.push_back(meshName);
		}
		meshNameFile.close();
	}
	for (int i = 0; i < nameVec.size(); ++i)
	{
		sModelDrawInfo drawInfo;
		pTheVAOManager->LoadModelIntoVAO(nameVec.at(i), meshVec.at(i), drawInfo, shaderProgID);
	}
	//sModelDrawInfo drawInfo;
	//pTheVAOManager->LoadModelIntoVAO("bunny", 
	//								 bunnyMesh, 
	//								 drawInfo, 
	//								 shaderProgID);
	//
	//sModelDrawInfo drawInfoPirate;
	//pTheVAOManager->LoadModelIntoVAO("pirate", 
	//								 pirateMesh,
	//								 drawInfoPirate, 
	//								 shaderProgID);
	//
	//sModelDrawInfo drawInfoTerrain;
	//pTheVAOManager->LoadModelIntoVAO("terrain", 
	//								 terrainMesh,
	//								 drawInfoTerrain,
	//								 shaderProgID);
	//
	//sModelDrawInfo cubeMeshInfo;
	//pTheVAOManager->LoadModelIntoVAO("cube", 
	//								 cubeMesh,			// Cube mesh info
	//								 cubeMeshInfo,
	//								 shaderProgID);
	//
	//sModelDrawInfo sphereMeshInfo;
	//pTheVAOManager->LoadModelIntoVAO("sphere", 
	//								 sphereMesh,		// Sphere mesh info
	//								 sphereMeshInfo,
	//								 shaderProgID);
	//
	//sModelDrawInfo boxBorderMeshInfo;
	//pTheVAOManager->LoadModelIntoVAO("border",
	//	borderMesh,
	//	boxBorderMeshInfo,
	//	shaderProgID);
	//
	//sModelDrawInfo boxBorderMeshInfo2;
	//pTheVAOManager->LoadModelIntoVAO("border2",
	//	borderMeshRight,
	//	boxBorderMeshInfo2,
	//	shaderProgID);
	//
	//sModelDrawInfo boxBorderMeshInfo3;
	//pTheVAOManager->LoadModelIntoVAO("border3",
	//	borderMeshTop,
	//	boxBorderMeshInfo3,
	//	shaderProgID);
	//
	//sModelDrawInfo boxBorderMeshInfo4;
	//pTheVAOManager->LoadModelIntoVAO("border4",
	//	borderMeshBottom,
	//	boxBorderMeshInfo4,
	//	shaderProgID);
	//
	//sModelDrawInfo boxBorderMeshInfo5;
	//pTheVAOManager->LoadModelIntoVAO("border5",
	//	borderMeshFront,
	//	boxBorderMeshInfo5,
	//	shaderProgID);
	//
	//sModelDrawInfo boxBorderMeshInfo6;
	//pTheVAOManager->LoadModelIntoVAO("border6",
	//	borderMeshBack,
	//	boxBorderMeshInfo6,
	//	shaderProgID);


	// At this point, the model is loaded into the GPU


	//// Load up my "scene" 
	//std::vector<cGameObject*> vec_pGameObjects;

	// Sphere and cube
	iObject* pSphere = pFactory->CreateObject("sphere");
	pSphere->setMeshName("sphere");
	pSphere->setFriendlyName("Sphere#1");	// We use to search 
	pSphere->setPositionXYZ(glm::vec3(-25.0f, 110.0f, 1.0f));
	pSphere->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pSphere->setScale(1.0f);
	pSphere->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pSphere->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
	pSphere->setAccel(glm::vec3(0.0f, 0.0f, 0.0f));
	pSphere->set_SPHERE_radius(1.0f);
	pSphere->setInverseMass(1.0f);
	//	pSphere->inverseMass = 0.0f;			// Sphere won't move
	pSphere->setIsVisible(true);
	pSphere->setIsWireframe(false);
	::g_vec_pGameObjects.push_back(pSphere);

	iObject* pWallLeft = pFactory->CreateObject("mesh");
	pWallLeft->setMeshName("border");
	pWallLeft->setFriendlyName("LeftWall");
	pWallLeft->setPositionXYZ(glm::vec3(-200.0f, 150.0f, 0.0f));
	pWallLeft->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pWallLeft->setScale(1.0f);
	pWallLeft->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pWallLeft->setInverseMass(0.0f);	// Ignored during update
	pWallLeft->setIsVisible(true);
	pWallLeft->setIsWireframe(false);
	::g_vec_pWallObjects.push_back(pWallLeft);

	iObject* pWallRight = pFactory->CreateObject("mesh");			// HEAP
	pWallRight->setMeshName("border");
	pWallRight->setFriendlyName("RightWall");
	pWallRight->setPositionXYZ(glm::vec3(200.0f, 150.0f, 0.0f));
	pWallRight->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pWallRight->setScale(1.0f);
	pWallRight->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pWallRight->setInverseMass(0.0f);	// Ignored during update
	pWallRight->setIsVisible(true);
	pWallRight->setIsWireframe(false);
	::g_vec_pWallObjects.push_back(pWallRight);

	iObject* pWallTop = pFactory->CreateObject("mesh");			// HEAP
	pWallTop->setMeshName("border");
	pWallTop->setFriendlyName("TopWall");
	pWallTop->setPositionXYZ(glm::vec3(0.0f, 350.0f, 0.0f));
	pWallTop->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pWallTop->setScale(1.0f);
	pWallTop->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pWallTop->setInverseMass(0.0f);	// Ignored during update
	pWallTop->setIsVisible(true);
	pWallTop->setIsWireframe(false);
	::g_vec_pWallObjects.push_back(pWallTop);

	iObject* pWallBottom = pFactory->CreateObject("mesh");			// HEAP
	pWallBottom->setMeshName("border");
	pWallBottom->setFriendlyName("BottomWall");
	pWallBottom->setPositionXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pWallBottom->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pWallBottom->setScale(1.0f);
	pWallBottom->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pWallBottom->setInverseMass(0.0f);	// Ignored during update
	pWallBottom->setIsVisible(true);
	pWallBottom->setIsWireframe(false);
	::g_vec_pWallObjects.push_back(pWallBottom);

	iObject* pWallFront = pFactory->CreateObject("mesh");			// HEAP
	pWallFront->setMeshName("border");
	pWallFront->setFriendlyName("FrontWall");
	pWallFront->setPositionXYZ(glm::vec3(0.0f, 150.0f, 200.0f));
	pWallFront->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pWallFront->setScale(1.0f);
	pWallFront->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pWallFront->setInverseMass(0.0f);	// Ignored during update
	pWallFront->setIsVisible(true);
	pWallFront->setIsWireframe(false);
	::g_vec_pWallObjects.push_back(pWallFront);

	iObject* pWallBack = pFactory->CreateObject("mesh");			// HEAP
	pWallBack->setMeshName("border");
	pWallBack->setFriendlyName("BackWall");
	pWallBack->setPositionXYZ(glm::vec3(0.0f, 5.0f, -200.0f));
	pWallBack->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pWallBack->setScale(1.0f);
	pWallBack->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pWallBack->setInverseMass(0.0f);	// Ignored during update
	pWallBack->setIsVisible(true);
	pWallBack->setIsWireframe(false);
	::g_vec_pWallObjects.push_back(pWallBack);

	for (xml_node ramp = rampData.first_child(); ramp; ramp = ramp.next_sibling())
	{
		std::string objectType = ramp.child("ObjectType").child_value();
		iObject* pRamp = pFactory->CreateObject(objectType);
		std::string meshName = ramp.child("MeshName").child_value();
		pRamp->setMeshName(meshName);
		std::string meshFriendlyName = ramp.child("MeshFriendlyName").child_value();
		pRamp->setFriendlyName(meshFriendlyName);
		std::string positionXString = ramp.child("PositionX").child_value();
		std::string positionYString = ramp.child("PositionY").child_value();
		std::string positionZString = ramp.child("PositionZ").child_value();
		pRamp->setPositionXYZ(glm::vec3(std::stof(positionXString), std::stof(positionYString), std::stof(positionZString)));
		std::string rotationXString = ramp.child("RotationX").child_value();
		std::string rotationYString = ramp.child("RotationY").child_value();
		std::string rotationZString = ramp.child("RotationZ").child_value();
		pRamp->setRotationXYZ(glm::vec3(std::stof(rotationXString), std::stof(rotationYString), std::stof(rotationZString)));
		std::string scaleString = ramp.child("Scale").child_value();
		pRamp->setScale(std::stof(scaleString));
		std::string objectColourRString = ramp.child("ObjectColourR").child_value();
		std::string objectColourGString = ramp.child("ObjectColourG").child_value();
		std::string objectColourBString = ramp.child("ObjectColourB").child_value();
		pRamp->setObjectColourRGBA(glm::vec4(std::stof(objectColourRString), std::stof(objectColourGString), std::stof(objectColourBString), 1.0f));
		std::string inverseMass = ramp.child("InverseMass").child_value();
		pRamp->setInverseMass(std::stof(inverseMass));	// Ignored during update
		std::string isVisible = ramp.child("IsVisible").child_value();
		pRamp->setIsVisible(std::stoi(isVisible));
		std::string isWireFrame = ramp.child("IsWireFrame").child_value();
		pRamp->setIsWireframe(std::stoi(isWireFrame));

		::g_vec_pRampObjects.push_back(pRamp);
	}

	for (xml_node ball = ballData.first_child(); ball; ball = ball.next_sibling())
	{
		std::string objectType = ball.child("ObjectType").child_value();
		iObject* pBall = pFactory->CreateObject(objectType);
		std::string meshName = ball.child("MeshName").child_value();
		pBall->setMeshName(meshName);
		std::string meshFriendlyName = ball.child("MeshFriendlyName").child_value();
		pBall->setFriendlyName(meshFriendlyName);
		std::string positionXString = ball.child("PositionX").child_value();
		std::string positionYString = ball.child("PositionY").child_value();
		std::string positionZString = ball.child("PositionZ").child_value();
		pBall->setPositionXYZ(glm::vec3(std::stof(positionXString), std::stof(positionYString), std::stof(positionZString)));
		std::string scaleString = ball.child("Scale").child_value();
		pBall->setScale(std::stof(scaleString));
		std::string objectColourRString = ball.child("ObjectColourR").child_value();
		std::string objectColourGString = ball.child("ObjectColourG").child_value();
		std::string objectColourBString = ball.child("ObjectColourB").child_value();
		pBall->setObjectColourRGBA(glm::vec4(std::stof(objectColourRString), std::stof(objectColourGString), std::stof(objectColourBString), 1.0f));
		std::string sphereRadius = ball.child("SphereRadius").child_value();
		pBall->set_SPHERE_radius(std::stof(sphereRadius));
		std::string inverseMass = ball.child("InverseMass").child_value();
		pBall->setInverseMass(std::stof(inverseMass));
		std::string isVisible = ball.child("IsVisible").child_value();
		pBall->setIsVisible(std::stoi(isVisible));
		std::string isWireFrame = ball.child("IsWireFrame").child_value();
		pBall->setIsWireframe(std::stoi(isWireFrame));

		::g_vec_pBallObjects.push_back(pBall);
	}
	int test = g_vec_pRampObjects.size();
	int test2 = g_vec_pBallObjects.size();

	
	//::g_vec_pGameObjects.push_back(pSphere2);
	//::g_vec_pGameObjects.push_back(pCube);
	//::g_vec_pGameObjects.push_back(pTerrain);
	//::g_vec_pGameObjects.push_back(pPirate);
	//::g_vec_pGameObjects.push_back(pBunny);
	
	// Will be moved placed around the scene
	iObject* pDebugSphere = pFactory->CreateObject("sphere");
	pDebugSphere->setMeshName("sphere");
	pDebugSphere->setFriendlyName("debug_sphere");
	pDebugSphere->setPositionXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pDebugSphere->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pDebugSphere->setScale(0.1f);
	//	pDebugSphere->objectColourRGBA = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pDebugSphere->setDebugColour(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	pDebugSphere->setIsWireframe(true);
	pDebugSphere->setInverseMass(0.0f);			// Sphere won't move
	pDebugSphere->setIsVisible(false);

	glEnable(GL_DEPTH);			// Write to the depth buffer
	glEnable(GL_DEPTH_TEST);	// Test with buffer when drawing

	cPhysics* pPhsyics = new cPhysics();

	cLightHelper* pLightHelper = new cLightHelper();

	//Get data from xml to set positions of main light
	pMainLight->setNodeName("MainLight");
	xml_node mainLightNode = lightData.child("MainLight");
	pMainLight->setPositionX(std::stof(mainLightNode.child("PositionX").child_value()));
	pMainLight->setPositionY(std::stof(mainLightNode.child("PositionY").child_value()));
	pMainLight->setPositionZ(std::stof(mainLightNode.child("PositionZ").child_value()));
	pMainLight->setPositionXYZ(glm::vec3(std::stof(mainLightNode.child("PositionX").child_value()), std::stof(mainLightNode.child("PositionY").child_value()), std::stof(mainLightNode.child("PositionZ").child_value())));
	pMainLight->setConstAtten(std::stof(mainLightNode.child("ConstAtten").child_value()));
	pMainLight->setLinearAtten(std::stof(mainLightNode.child("LinearAtten").child_value()));
	pMainLight->setQuadraticAtten(std::stof(mainLightNode.child("QuadraticAtten").child_value()));
	pMainLight->setInnerSpot(std::stof(mainLightNode.child("SpotInnerAngle").child_value()));
	pMainLight->setOuterSpot(std::stof(mainLightNode.child("SpotOuterAngle").child_value()));

	//Get data from xml to set positions of corner light 1
	pCorner1Light->setNodeName("CornerLight1");
	xml_node cornerLight1Node = lightData.child("CornerLight1");
	pCorner1Light->setPositionX(std::stof(cornerLight1Node.child("PositionX").child_value()));
	pCorner1Light->setPositionY(std::stof(cornerLight1Node.child("PositionY").child_value()));
	pCorner1Light->setPositionZ(std::stof(cornerLight1Node.child("PositionZ").child_value()));
	pCorner1Light->setPositionXYZ(glm::vec3(std::stof(cornerLight1Node.child("PositionX").child_value()), std::stof(cornerLight1Node.child("PositionY").child_value()), std::stof(cornerLight1Node.child("PositionZ").child_value())));
	pCorner1Light->setConstAtten(std::stof(cornerLight1Node.child("ConstAtten").child_value()));
	pCorner1Light->setLinearAtten(std::stof(cornerLight1Node.child("LinearAtten").child_value()));
	pCorner1Light->setQuadraticAtten(std::stof(cornerLight1Node.child("QuadraticAtten").child_value()));
	pCorner1Light->setInnerSpot(std::stof(cornerLight1Node.child("SpotInnerAngle").child_value()));
	pCorner1Light->setOuterSpot(std::stof(cornerLight1Node.child("SpotOuterAngle").child_value()));
	//Get data from xml to set positions of corner light 2
	pCorner2Light->setNodeName("CornerLight2");
	xml_node cornerLight2Node = lightData.child("CornerLight2");
	pCorner2Light->setPositionX(std::stof(cornerLight2Node.child("PositionX").child_value()));
	pCorner2Light->setPositionY(std::stof(cornerLight2Node.child("PositionY").child_value()));
	pCorner2Light->setPositionZ(std::stof(cornerLight2Node.child("PositionZ").child_value()));
	pCorner2Light->setPositionXYZ(glm::vec3(std::stof(cornerLight2Node.child("PositionX").child_value()), std::stof(cornerLight2Node.child("PositionY").child_value()), std::stof(cornerLight2Node.child("PositionZ").child_value())));
	pCorner2Light->setConstAtten(std::stof(cornerLight2Node.child("ConstAtten").child_value()));
	pCorner2Light->setLinearAtten(std::stof(cornerLight2Node.child("LinearAtten").child_value()));
	pCorner2Light->setQuadraticAtten(std::stof(cornerLight2Node.child("QuadraticAtten").child_value()));
	pCorner2Light->setInnerSpot(std::stof(cornerLight2Node.child("SpotInnerAngle").child_value()));
	pCorner2Light->setOuterSpot(std::stof(cornerLight2Node.child("SpotOuterAngle").child_value()));
	//Get data from xml to set positions of corner light 3
	pCorner3Light->setNodeName("CornerLight3");
	xml_node cornerLight3Node = lightData.child("CornerLight3");
	pCorner3Light->setPositionX(std::stof(cornerLight3Node.child("PositionX").child_value()));
	pCorner3Light->setPositionY(std::stof(cornerLight3Node.child("PositionY").child_value()));
	pCorner3Light->setPositionZ(std::stof(cornerLight3Node.child("PositionZ").child_value()));
	pCorner3Light->setPositionXYZ(glm::vec3(std::stof(cornerLight3Node.child("PositionX").child_value()), std::stof(cornerLight3Node.child("PositionY").child_value()), std::stof(cornerLight3Node.child("PositionZ").child_value())));
	pCorner3Light->setConstAtten(std::stof(cornerLight3Node.child("ConstAtten").child_value()));
	pCorner3Light->setLinearAtten(std::stof(cornerLight3Node.child("LinearAtten").child_value()));
	pCorner3Light->setQuadraticAtten(std::stof(cornerLight3Node.child("QuadraticAtten").child_value()));
	pCorner3Light->setInnerSpot(std::stof(cornerLight3Node.child("SpotInnerAngle").child_value()));
	pCorner3Light->setOuterSpot(std::stof(cornerLight3Node.child("SpotOuterAngle").child_value()));
	//Get data from xml to set positions of corner light 4
	pCorner4Light->setNodeName("CornerLight4");
	xml_node cornerLight4Node = lightData.child("CornerLight4");
	pCorner4Light->setPositionX(std::stof(cornerLight4Node.child("PositionX").child_value()));
	pCorner4Light->setPositionY(std::stof(cornerLight4Node.child("PositionY").child_value()));
	pCorner4Light->setPositionZ(std::stof(cornerLight4Node.child("PositionZ").child_value()));
	pCorner4Light->setPositionXYZ(glm::vec3(std::stof(cornerLight4Node.child("PositionX").child_value()), std::stof(cornerLight4Node.child("PositionY").child_value()), std::stof(cornerLight4Node.child("PositionZ").child_value())));
	pCorner4Light->setConstAtten(std::stof(cornerLight4Node.child("ConstAtten").child_value()));
	pCorner4Light->setLinearAtten(std::stof(cornerLight4Node.child("LinearAtten").child_value()));
	pCorner4Light->setQuadraticAtten(std::stof(cornerLight4Node.child("QuadraticAtten").child_value()));
	pCorner4Light->setInnerSpot(std::stof(cornerLight4Node.child("SpotInnerAngle").child_value()));
	pCorner4Light->setOuterSpot(std::stof(cornerLight4Node.child("SpotOuterAngle").child_value()));

	int count = 1;
	for (xml_node ballLight = ballLightData.first_child(); ballLight; ballLight = ballLight.next_sibling())
	{
		cLight* pBallLight = new cLight();
		pBallLight->setNodeName("BallLight" + std::to_string(count));
		pBallLight->setPositionX(std::stof(ballLight.child("PositionX").child_value()));
		pBallLight->setPositionY(std::stof(ballLight.child("PositionY").child_value()));
		pBallLight->setPositionZ(std::stof(ballLight.child("PositionZ").child_value()));
		pBallLight->setPositionXYZ(glm::vec3(std::stof(ballLight.child("PositionX").child_value()), std::stof(ballLight.child("PositionY").child_value()), std::stof(ballLight.child("PositionZ").child_value())));
		pBallLight->setConstAtten(std::stof(ballLight.child("ConstAtten").child_value()));
		pBallLight->setLinearAtten(std::stof(ballLight.child("LinearAtten").child_value()));
		pBallLight->setQuadraticAtten(std::stof(ballLight.child("QuadraticAtten").child_value()));
		pBallLight->setInnerSpot(std::stof(ballLight.child("SpotInnerAngle").child_value()));
		pBallLight->setOuterSpot(std::stof(ballLight.child("SpotOuterAngle").child_value()));
		pMediator->m_vec_pBallLights.push_back(pBallLight);
		count++;
	}

	pMediator->m_vec_pLights.push_back(pMainLight);
	pMediator->m_vec_pLights.push_back(pCorner1Light);
	pMediator->m_vec_pLights.push_back(pCorner2Light);
	pMediator->m_vec_pLights.push_back(pCorner3Light);
	pMediator->m_vec_pLights.push_back(pCorner4Light);

	int setCount = 0;
	float properYPosition = 0.0f;
	float properPlatformYPosition = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		if (fileChanged)
		{

		}
		float ratio;
		int width, height;
		glm::mat4 p, v;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		// Projection matrix
		p = glm::perspective(0.6f,		// FOV
			ratio,			// Aspect ratio
			0.1f,			// Near clipping plane
			1000.0f);		// Far clipping plane

// View matrix
		v = glm::mat4(1.0f);

		glm::vec3 mainLightPosition = glm::vec3(pMainLight->getPositionX(), pMainLight->getPositionY(), pMainLight->getPositionZ());
		glm::vec3 corner1LightPosition = glm::vec3(pCorner1Light->getPositionX(), pCorner1Light->getPositionY(), pCorner1Light->getPositionZ());
		glm::vec3 corner2LightPosition = glm::vec3(pCorner2Light->getPositionX(), pCorner2Light->getPositionY(), pCorner2Light->getPositionZ());
		glm::vec3 corner3LightPosition = glm::vec3(pCorner3Light->getPositionX(), pCorner3Light->getPositionY(), pCorner3Light->getPositionZ());
		glm::vec3 corner4LightPosition = glm::vec3(pCorner4Light->getPositionX(), pCorner4Light->getPositionY(), pCorner4Light->getPositionZ());

		//glm::vec3 currentObjectPosition = glm::vec3(g_vec_pGameObjects.at(currentRamp)->positionXYZ);

		//Look at lights

		//if (pMediator->m_vec_pLights.at(currentLight)->getNodeName() == "MainLight")
		//{
		//	v = glm::lookAt(cameraEye,
		//		mainLightPosition,
		//		upVector);
		//}
		//if (pMediator->m_vec_pLights.at(currentLight)->getNodeName() == "CornerLight1")
		//{
		//	v = glm::lookAt(cameraEye,
		//		corner1LightPosition,
		//		upVector);
		//}
		//if (pMediator->m_vec_pLights.at(currentLight)->getNodeName() == "CornerLight2")
		//{
		//	v = glm::lookAt(cameraEye,
		//		corner2LightPosition,
		//		upVector);
		//}
		//if (pMediator->m_vec_pLights.at(currentLight)->getNodeName() == "CornerLight3")
		//{
		//	v = glm::lookAt(cameraEye,
		//		corner3LightPosition,
		//		upVector);
		//}
		//if (pMediator->m_vec_pLights.at(currentLight)->getNodeName() == "CornerLight4")
		//{
		//	v = glm::lookAt(cameraEye,
		//		corner4LightPosition,
		//		upVector);
		//}

		//Look at current object
		/*v = glm::lookAt(cameraEye,
			currentObjectPosition,
			upVector);*/

			//Look at sphere

		glm::vec3 cameraEye = glm::vec3(g_vec_pGameObjects.at(0)->getPositionXYZ().x + cameraLeftRight, g_vec_pGameObjects.at(0)->getPositionXYZ().y + 20.0f, g_vec_pGameObjects.at(0)->getPositionXYZ().z - 50.0f);
		v = glm::lookAt(cameraEye,
			pSphere->getPositionXYZ(),
			upVector);

		glViewport(0, 0, width, height);

		// Clear both the colour buffer (what we see) and the 
		//  depth (or z) buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int lightIndex = 0;
		for (lightIndex; lightIndex < pMediator->m_vec_pLights.size(); ++lightIndex)
		{
			std::string positionString = "theLights[" + std::to_string(lightIndex) + "].position";
			std::string diffuseString = "theLights[" + std::to_string(lightIndex) + "].diffuse";
			std::string specularString = "theLights[" + std::to_string(lightIndex) + "].specular";
			std::string attenString = "theLights[" + std::to_string(lightIndex) + "].atten";
			std::string directionString = "theLights[" + std::to_string(lightIndex) + "].direction";
			std::string param1String = "theLights[" + std::to_string(lightIndex) + "].param1";
			std::string param2String = "theLights[" + std::to_string(lightIndex) + "].param2";

			GLint position = glGetUniformLocation(shaderProgID, positionString.c_str());
			GLint diffuse = glGetUniformLocation(shaderProgID, diffuseString.c_str());
			GLint specular = glGetUniformLocation(shaderProgID, specularString.c_str());
			GLint atten = glGetUniformLocation(shaderProgID, attenString.c_str());
			GLint direction = glGetUniformLocation(shaderProgID, directionString.c_str());
			GLint param1 = glGetUniformLocation(shaderProgID, param1String.c_str());
			GLint param2 = glGetUniformLocation(shaderProgID, param2String.c_str());

			glUniform4f(position, pMediator->m_vec_pLights.at(lightIndex)->getPositionX(), pMediator->m_vec_pLights.at(lightIndex)->getPositionY(), pMediator->m_vec_pLights.at(lightIndex)->getPositionZ(), 1.0f);
			glUniform4f(diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
			glUniform4f(specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
			glUniform4f(atten, pMediator->m_vec_pLights.at(lightIndex)->getConstAtten(),  /* constant attenuation */	pMediator->m_vec_pLights.at(lightIndex)->getLinearAtten(),  /* Linear */ pMediator->m_vec_pLights.at(lightIndex)->getQuadraticAtten(),	/* Quadratic */  1000000.0f);	// Distance cut off

			glUniform4f(param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 2.0f);
			glUniform4f(param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
		}

		for (int i = 0; i < pMediator->m_vec_pBallLights.size(); ++i)
		{
			std::string positionString = "theLights[" + std::to_string(lightIndex) + "].position";
			std::string diffuseString = "theLights[" + std::to_string(lightIndex) + "].diffuse";
			std::string specularString = "theLights[" + std::to_string(lightIndex) + "].specular";
			std::string attenString = "theLights[" + std::to_string(lightIndex) + "].atten";
			std::string directionString = "theLights[" + std::to_string(lightIndex) + "].direction";
			std::string param1String = "theLights[" + std::to_string(lightIndex) + "].param1";
			std::string param2String = "theLights[" + std::to_string(lightIndex) + "].param2";

			GLint position = glGetUniformLocation(shaderProgID, positionString.c_str());
			GLint diffuse = glGetUniformLocation(shaderProgID, diffuseString.c_str());
			GLint specular = glGetUniformLocation(shaderProgID, specularString.c_str());
			GLint atten = glGetUniformLocation(shaderProgID, attenString.c_str());
			GLint direction = glGetUniformLocation(shaderProgID, directionString.c_str());
			GLint param1 = glGetUniformLocation(shaderProgID, param1String.c_str());
			GLint param2 = glGetUniformLocation(shaderProgID, param2String.c_str());

			glUniform4f(position, pMediator->m_vec_pBallLights.at(i)->getPositionX(), pMediator->m_vec_pBallLights.at(i)->getPositionY(), pMediator->m_vec_pBallLights.at(i)->getPositionZ(), 1.0f);
			glUniform4f(diffuse, 1.0f, 0.0f, 1.0f, 1.0f);	// White
			glUniform4f(specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
			glUniform4f(atten, pMediator->m_vec_pBallLights.at(i)->getConstAtten(),  /* constant attenuation */	pMediator->m_vec_pBallLights.at(i)->getLinearAtten(),  /* Linear */ pMediator->m_vec_pBallLights.at(i)->getQuadraticAtten(),	/* Quadratic */  1.0f);	// Distance cut off
			glUniform4f(direction, 0.0f, -1.0f, 0.0f, 1.0f);
			glUniform4f(param1, 0.0f /*POINT light*/, 1.0f, 0.0f, 2.0f);
			glUniform4f(param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);

			lightIndex++;
		}
		int vecSize = pMediator->m_vec_pBallLights.size();
		//GLint L_0_position = glGetUniformLocation( shaderProgID, "theLights[0].position");
		//GLint L_0_diffuse = glGetUniformLocation( shaderProgID, "theLights[0].diffuse");
		//GLint L_0_specular = glGetUniformLocation( shaderProgID, "theLights[0].specular");
		//GLint L_0_atten = glGetUniformLocation( shaderProgID, "theLights[0].atten");
		//GLint L_0_direction = glGetUniformLocation( shaderProgID, "theLights[0].direction");
		//GLint L_0_param1 = glGetUniformLocation( shaderProgID, "theLights[0].param1");
		//GLint L_0_param2 = glGetUniformLocation( shaderProgID, "theLights[0].param2");
		//
		//glUniform4f(L_0_position, pMainLight->getPositionX(), pMainLight->getPositionY(), pMainLight->getPositionZ(),	1.0f);
		//glUniform4f(L_0_diffuse, 1.0f, 1.0f, 1.0f, 1.0f );	// White
		//glUniform4f(L_0_specular, 1.0f, 1.0f, 1.0f, 1.0f );	// White
		//glUniform4f(L_0_atten, pMainLight->getConstAtten(),  /* constant attenuation */	pMainLight->getLinearAtten(),  /* Linear */ pMainLight->getQuadraticAtten(),	/* Quadratic */  1000000.0f );	// Distance cut off
		//
		//glUniform4f(L_0_param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 2.0f);
		//glUniform4f(L_0_param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
		//
		//GLint L_1_position = glGetUniformLocation(shaderProgID, "theLights[1].position");
		//GLint L_1_diffuse = glGetUniformLocation(shaderProgID, "theLights[1].diffuse");
		//GLint L_1_specular = glGetUniformLocation(shaderProgID, "theLights[1].specular");
		//GLint L_1_atten = glGetUniformLocation(shaderProgID, "theLights[1].atten");
		//GLint L_1_direction = glGetUniformLocation(shaderProgID, "theLights[1].direction");
		//GLint L_1_param1 = glGetUniformLocation(shaderProgID, "theLights[1].param1");
		//GLint L_1_param2 = glGetUniformLocation(shaderProgID, "theLights[1].param2");
		//
		//glUniform4f(L_1_position,
		//	pCorner1Light->getPositionX(),
		//	pCorner1Light->getPositionY(),
		//	pCorner1Light->getPositionZ(),
		//	1.0f);
		//glUniform4f(L_1_diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_1_specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_1_atten, pCorner1Light->getConstAtten(),  // constant attenuation
		//	pCorner1Light->getLinearAtten(),  // Linear 
		//	pCorner1Light->getQuadraticAtten(),	// Quadratic 
		//	1000000.0f);	// Distance cut off
		//
		//glUniform4f(L_1_param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f);
		//glUniform4f(L_1_param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
		//
		//GLint L_2_position = glGetUniformLocation(shaderProgID, "theLights[1].position");
		//GLint L_2_diffuse = glGetUniformLocation(shaderProgID, "theLights[1].diffuse");
		//GLint L_2_specular = glGetUniformLocation(shaderProgID, "theLights[1].specular");
		//GLint L_2_atten = glGetUniformLocation(shaderProgID, "theLights[1].atten");
		//GLint L_2_direction = glGetUniformLocation(shaderProgID, "theLights[1].direction");
		//GLint L_2_param1 = glGetUniformLocation(shaderProgID, "theLights[1].param1");
		//GLint L_2_param2 = glGetUniformLocation(shaderProgID, "theLights[1].param2");
		//
		//glUniform4f(L_2_position,
		//	pCorner2Light->getPositionX(),
		//	pCorner2Light->getPositionY(),
		//	pCorner2Light->getPositionZ(),
		//	1.0f);
		//glUniform4f(L_2_diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_2_specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_2_atten, pCorner2Light->getConstAtten(),  // constant attenuation
		//	pCorner2Light->getLinearAtten(),  // Linear 
		//	pCorner2Light->getQuadraticAtten(),	// Quadratic 
		//	1000000.0f);	// Distance cut off
		//
		//glUniform4f(L_2_param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f);
		//glUniform4f(L_2_param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
		//
		//GLint L_2_position = glGetUniformLocation(shaderProgID, "theLights[2].position");
		//GLint L_2_diffuse = glGetUniformLocation(shaderProgID, "theLights[2].diffuse");
		//GLint L_2_specular = glGetUniformLocation(shaderProgID, "theLights[2].specular");
		//GLint L_2_atten = glGetUniformLocation(shaderProgID, "theLights[2].atten");
		//GLint L_2_direction = glGetUniformLocation(shaderProgID, "theLights[2].direction");
		//GLint L_2_param1 = glGetUniformLocation(shaderProgID, "theLights[2].param1");
		//GLint L_2_param2 = glGetUniformLocation(shaderProgID, "theLights[2].param2");
		//
		//glUniform4f(L_2_position,
		//	borderLight2.x,
		//	borderLight2.y,
		//	borderLight2.z,
		//	1.0f);
		//glUniform4f(L_2_diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_2_specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_2_atten, 0.0f,  // constant attenuation
		//	borderLightLinearAtten2,  // Linear 
		//	borderLightQuadraticAtten2,	// Quadratic 
		//	1000000.0f);	// Distance cut off
		//
		//glUniform4f(L_2_param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f);
		//glUniform4f(L_2_param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
		//
		//GLint L_3_position = glGetUniformLocation(shaderProgID, "theLights[3].position");
		//GLint L_3_diffuse = glGetUniformLocation(shaderProgID, "theLights[3].diffuse");
		//GLint L_3_specular = glGetUniformLocation(shaderProgID, "theLights[3].specular");
		//GLint L_3_atten = glGetUniformLocation(shaderProgID, "theLights[3].atten");
		//GLint L_3_direction = glGetUniformLocation(shaderProgID, "theLights[3].direction");
		//GLint L_3_param1 = glGetUniformLocation(shaderProgID, "theLights[3].param1");
		//GLint L_3_param2 = glGetUniformLocation(shaderProgID, "theLights[3].param2");
		//
		//glUniform4f(L_3_position,
		//	borderLight3.x,
		//	borderLight3.y,
		//	borderLight3.z,
		//	1.0f);
		//glUniform4f(L_3_diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_3_specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_3_atten, 0.0f,  // constant attenuation
		//	borderLightLinearAtten3,  // Linear 
		//	borderLightQuadraticAtten3,	// Quadratic 
		//	1000000.0f);	// Distance cut off
//
		//glUniform4f(L_0_param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f);
		//glUniform4f(L_0_param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
//
		//GLint L_4_position = glGetUniformLocation(shaderProgID, "theLights[4].position");
		//GLint L_4_diffuse = glGetUniformLocation(shaderProgID, "theLights[4].diffuse");
		//GLint L_4_specular = glGetUniformLocation(shaderProgID, "theLights[4].specular");
		//GLint L_4_atten = glGetUniformLocation(shaderProgID, "theLights[4].atten");
		//GLint L_4_direction = glGetUniformLocation(shaderProgID, "theLights[4].direction");
		//GLint L_4_param1 = glGetUniformLocation(shaderProgID, "theLights[4].param1");
		//GLint L_4_param2 = glGetUniformLocation(shaderProgID, "theLights[4].param2");
//
		//glUniform4f(L_4_position,
		//	borderLight4.x,
		//	borderLight4.y,
		//	borderLight4.z,
		//	1.0f);
		//glUniform4f(L_4_diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_4_specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_4_atten, 0.0f,  // constant attenuation
		//	borderLightLinearAtten4,  // Linear 
		//	borderLightQuadraticAtten4,	// Quadratic 
		//	1000000.0f);	// Distance cut off
//
		//glUniform4f(L_4_param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f);
		//glUniform4f(L_4_param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
//
		//GLint L_5_position = glGetUniformLocation(shaderProgID, "theLights[5].position");
		//GLint L_5_diffuse = glGetUniformLocation(shaderProgID, "theLights[5].diffuse");
		//GLint L_5_specular = glGetUniformLocation(shaderProgID, "theLights[5].specular");
		//GLint L_5_atten = glGetUniformLocation(shaderProgID, "theLights[5].atten");
		//GLint L_5_direction = glGetUniformLocation(shaderProgID, "theLights[5].direction");
		//GLint L_5_param1 = glGetUniformLocation(shaderProgID, "theLights[5].param1");
		//GLint L_5_param2 = glGetUniformLocation(shaderProgID, "theLights[5].param2");
//
		//glUniform4f(L_5_position,
		//	borderLight5.x,
		//	borderLight5.y,
		//	borderLight5.z,
		//	1.0f);
		//glUniform4f(L_5_diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_5_specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_5_atten, 0.0f,  // constant attenuation
		//	borderLightLinearAtten5,  // Linear 
		//	borderLightQuadraticAtten5,	// Quadratic 
		//	1000000.0f);	// Distance cut off
//
		//glUniform4f(L_5_param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f);
		//glUniform4f(L_5_param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
//
		//GLint L_6_position = glGetUniformLocation(shaderProgID, "theLights[6].position");
		//GLint L_6_diffuse = glGetUniformLocation(shaderProgID, "theLights[6].diffuse");
		//GLint L_6_specular = glGetUniformLocation(shaderProgID, "theLights[6].specular");
		//GLint L_6_atten = glGetUniformLocation(shaderProgID, "theLights[6].atten");
		//GLint L_6_direction = glGetUniformLocation(shaderProgID, "theLights[6].direction");
		//GLint L_6_param1 = glGetUniformLocation(shaderProgID, "theLights[6].param1");
		//GLint L_6_param2 = glGetUniformLocation(shaderProgID, "theLights[6].param2");
//
		//glUniform4f(L_6_position,
		//	borderLight6.x,
		//	borderLight6.y,
		//	borderLight6.z,
		//	1.0f);
		//glUniform4f(L_6_diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_6_specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
		//glUniform4f(L_6_atten, 0.0f,  // constant attenuation
		//	borderLightLinearAtten6,  // Linear 
		//	borderLightQuadraticAtten6,	// Quadratic 
		//	1000000.0f);	// Distance cut off
//
		//// Point light:
		//glUniform4f(L_6_param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f );
		//glUniform4f(L_6_param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);


		// ********************************************************
		// Move the pirate in the direction it's pointing in...

		// Vec4 = mat4x4 * vec4				vertFinal = matModel * vertStart;

		//cGameObject* pPirate = pFindObjectByFriendlyName("PirateShip");

		//glm::mat4 matRotY = glm::rotate(glm::mat4(1.0f),
		//								pPirate->HACK_AngleAroundYAxis,	//(float)glfwGetTime(),					// Angle 
		//								glm::vec3(0.0f, 1.0f, 0.0f));

		//// Assume the ship is at 0,0,0
		//glm::vec4 frontOfShip = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);	// pointing to the "front" of the ship
		//
		//// Vec4 = mat4x4 * vec4				vertFinal = matModel * vertStart;
		//glm::vec4 frontOfShipInWorld = matRotY * frontOfShip;

		//// this value would be the velocity, ifyou wanted the phsyics update do to it
		//glm::vec3 newSpeedOfShipIN_THE_DIRECTION_WE_WANT_TO_GO
		//	= frontOfShipInWorld * pPirate->HACK_speed;

		//// Update the pirate ship
		//pPirate->positionXYZ += newSpeedOfShipIN_THE_DIRECTION_WE_WANT_TO_GO;

		// ********************************************************

		// Also set the position of my "eye" (the camera)
		//uniform vec4 eyeLocation;
		GLint eyeLocation_UL = glGetUniformLocation(shaderProgID, "eyeLocation");

		glUniform4f(eyeLocation_UL,
			cameraEye.x, cameraEye.y, cameraEye.z, 1.0f);


		std::stringstream ssTitle;
		ssTitle
			<< "X: "
			<< g_vec_pGameObjects.at(0)->getPositionXYZ().x << ", "
			<< "Y: "
			<< g_vec_pGameObjects.at(0)->getPositionXYZ().y << ", "
			<< "Z: "
			<< g_vec_pGameObjects.at(0)->getPositionXYZ().z;
		//<< "Const Atten: "
		//<< pLightsVec.at(currentLight)->getConstAtten() << " , "
		//<< "Linear Atten: "
		//<< pLightsVec.at(currentLight)->getLinearAtten() << " , "
		//<< "Quadratic Atten: "
		//<< pLightsVec.at(currentLight)->getQuadraticAtten();
		glfwSetWindowTitle(window, ssTitle.str().c_str());


		GLint matView_UL = glGetUniformLocation(shaderProgID, "matView");
		GLint matProj_UL = glGetUniformLocation(shaderProgID, "matProj");

		glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(matProj_UL, 1, GL_FALSE, glm::value_ptr(p));


		// **************************************************
		// **************************************************
		// Loop to draw everything in the scene


		for (int index = 0; index != ::g_vec_pGameObjects.size(); index++)
		{
			glm::mat4 matModel = glm::mat4(1.0f);

			iObject* pCurrentObject = ::g_vec_pGameObjects[index];

			DrawObject(matModel, pCurrentObject,
				shaderProgID, pTheVAOManager);

		}//for (int index...
		for (int index = 0; index != ::g_vec_pWallObjects.size(); index++)
		{
			glm::mat4 matModel = glm::mat4(1.0f);

			iObject* pCurrentObject = ::g_vec_pWallObjects[index];

			DrawObject(matModel, pCurrentObject,
				shaderProgID, pTheVAOManager);

		}//for (int index...
		for (int index = 0; index != ::g_vec_pRampObjects.size(); index++)
		{
			glm::mat4 matModel = glm::mat4(1.0f);

			iObject* pCurrentObject = ::g_vec_pRampObjects[index];

			DrawObject(matModel, pCurrentObject,
				shaderProgID, pTheVAOManager);

		}//for (int index...
		for (int index = 0; index != ::g_vec_pBallObjects.size(); index++)
		{
			glm::mat4 matModel = glm::mat4(1.0f);

			iObject* pCurrentObject = ::g_vec_pBallObjects[index];

			DrawObject(matModel, pCurrentObject,
				shaderProgID, pTheVAOManager);

		}//for (int index...

		// Let's draw all the closest points to the sphere
		// on the terrain mesh....
		// 
		// For each triangle in the terrain mesh:
		// - Run ClosestPointToTriangle
		// - Place the debug sphere "there"
		// - Draw it.
		pPhsyics->IntegrationStep(::g_vec_pGameObjects, 0.03f);
		pPhsyics->IntegrationStep(::g_vec_pBallObjects, 0.03f);

		if (pSphere->getVelocity().x < 0)
		{
			pSphere->setVelocity(pSphere->getVelocity() * glm::vec3(-1.0f, -1.0f, -1.0f));
			pSphere->setVelocity(pSphere->getVelocity() * glm::vec3(0.95f, 1.0f, 1.0f));
			pSphere->setVelocity(pSphere->getVelocity()* glm::vec3(-1.0f, -1.0f, -1.0f));
		}
		if (pSphere->getVelocity().z < 0)
		{
			pSphere->setVelocity(pSphere->getVelocity()* glm::vec3(-1.0f, -1.0f, -1.0f));
			pSphere->setVelocity(pSphere->getVelocity() * glm::vec3(1.0f, 1.0f, 0.95f));
			pSphere->setVelocity(pSphere->getVelocity()* glm::vec3(-1.0f, -1.0f, -1.0f));
		}
		glm::vec3 slowX = glm::vec3(0.95, 1.0f, 1.0f);
		glm::vec3 slowZ = glm::vec3(1.0f, 1.0f, 0.95);
		if (pSphere->getVelocity().x > 0)
		{
			pSphere->setVelocity(pSphere->getVelocity() * slowX);
		}
		if (pSphere->getVelocity().z > 0)
		{
			pSphere->setVelocity(pSphere->getVelocity()* slowZ);
		}

		//*************************************
		//	Walls
		//*************************************

		for (int k = 0; k < ::g_vec_pWallObjects.size(); k++)
		{
			glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
			cPhysics::sPhysicsTriangle closestTriangle;

			glm::mat4 matWorld = calculateWorldMatrix(g_vec_pWallObjects.at(k));

			cMesh transformedMesh;
			pPhsyics->CalculateTransformedMesh(meshVec.at(5), matWorld, transformedMesh);

			pPhsyics->GetClosestTriangleToPoint(pSphere->getPositionXYZ(), transformedMesh, closestPoint, closestTriangle);

			// Highlight the triangle that I'm closest to
			pDebugRenderer->addTriangle(closestTriangle.verts[0],
				closestTriangle.verts[1],
				closestTriangle.verts[2],
				glm::vec3(1.0f, 0.0f, 0.0f));

			// Highlight the triangle that I'm closest to
			// To draw the normal, calculate the average of the 3 vertices, 
			// then draw that average + the normal (the normal starts at the 0,0,0 OF THE TRIANGLE)
			glm::vec3 centreOfTriangle = (closestTriangle.verts[0] +
				closestTriangle.verts[1] +
				closestTriangle.verts[2]) / 3.0f;		// Average

			glm::vec3 normalInWorld = centreOfTriangle + (closestTriangle.normal * 20.0f);	// Normal x 10 length

			pDebugRenderer->addLine(centreOfTriangle,
				normalInWorld,
				glm::vec3(1.0f, 1.0f, 0.0f));

			// Are we hitting the triangle? 
			float distance = glm::length(pSphere->getPositionXYZ() - closestPoint);

			if (distance <= pSphere->get_SPHERE_radius())
			{
				//if (k == 0)
				//{
				//	pSphere->inverseMass = 0.0f;
				//	pSphere->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
				//}

				// ************************************************************************

				// If you want, move the sphere back to where it just penetrated...
				// So that it will collide exactly where it's supposed to. 
				// But, that's not a big problem.

				// 1. Calculate vector from centre of sphere to closest point
				glm::vec3 vecSphereToClosestPoint = closestPoint - pSphere->getPositionXYZ();

				// 2. Get the length of this vector
				float centreToContractDistance = glm::length(vecSphereToClosestPoint);

				// 3. Create a vector from closest point to radius
				float lengthPositionAdjustment = pSphere->get_SPHERE_radius() - centreToContractDistance;

				// 4. Sphere is moving in the direction of the velocity, so 
				//    we want to move the sphere BACK along this velocity vector
				glm::vec3 vecDirection = glm::normalize(pSphere->getVelocity());

				glm::vec3 vecPositionAdjust = (-vecDirection) * lengthPositionAdjustment;

				// 5. Reposition sphere 
				pSphere->setPositionXYZ(pSphere->getPositionXYZ() + vecPositionAdjust);
				//			pSphere->inverseMass = 0.0f;

							// ************************************************************************


							// Is in contact with the triangle... 
							// Calculate the response vector off the triangle. 
				glm::vec3 velocityVector = glm::normalize(pSphere->getVelocity());
				float gravY = (-pSphere->getVelocity().y) * 0.45f;
				glm::vec3 gravity = glm::vec3(0.0f, gravY, 0.0f);

				// closestTriangle.normal
				glm::vec3 reflectionVec = glm::reflect(velocityVector, closestTriangle.normal);
				reflectionVec = glm::normalize(reflectionVec);

				// Stop the sphere and draw the two vectors...
	//			pSphere->inverseMass = 0.0f;	// Stopped

				glm::vec3 velVecX20 = velocityVector * 10.0f;
				pDebugRenderer->addLine(closestPoint, velVecX20,
					glm::vec3(1.0f, 0.0f, 0.0f), 30.0f /*seconds*/);

				glm::vec3 reflectionVecX20 = reflectionVec * 10.0f;
				pDebugRenderer->addLine(closestPoint, reflectionVecX20,
					glm::vec3(0.0f, 1.0f, 1.0f), 30.0f /*seconds*/);

				// Change the direction of the ball (the bounce off the triangle)

				// Get lenght of the velocity vector
				float speed = glm::length(pSphere->getVelocity());



				if (pSphere->getVelocity().y * -1 < 1 && !onGround)
				{
					onGround = true;
					properYPosition = pSphere->getPositionXYZ().y;
				}
				if (!onGround)
				{
					pSphere->setVelocity((reflectionVec * speed) - gravity);
				}
				else if (onGround && properYPosition != 0.0f)
				{
					pSphere->setVelocity(reflectionVec * speed);
					glm::vec3 properPositionXYZ = glm::vec3(pSphere->getPositionXYZ().x, properYPosition, pSphere->getPositionXYZ().z);
					pSphere->setPositionXYZ(properPositionXYZ);
				}
				//std::cout << pSphere->velocity.b << ", " << pSphere->velocity.g << ", " << pSphere->velocity.p << ", " << pSphere->velocity.r << ", " << pSphere->velocity.s << ", " << pSphere->velocity.t << ", " << pSphere->velocity.x << ", " << pSphere->velocity.y << ", " << pSphere->velocity.z;
			}

			/*bool DidBallCollideWithGround = false;
			HACK_BounceOffSomePlanes(pSphere, DidBallCollideWithGround );*/

			// A more general 
			pPhsyics->TestForCollisions(::g_vec_pGameObjects);
			pPhsyics->TestForCollisions(::g_vec_pWallObjects);
			pPhsyics->TestForCollisions(::g_vec_pRampObjects);
			pPhsyics->TestForCollisions(::g_vec_pBallObjects);

			//{// Draw closest point
			//	glm::mat4 matModel = glm::mat4(1.0f);
			//	pDebugSphere->positionXYZ = closestPoint;
			//	pDebugSphere->scale = 1.0f;
			//	pDebugSphere->debugColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
			//	pDebugSphere->isWireframe = true;
			//	DrawObject(matModel, pDebugSphere,
			//		shaderProgID, pTheVAOManager);
			//}


			// How far did we penetrate the surface?
			glm::vec3 CentreToClosestPoint = pSphere->getPositionXYZ() - closestPoint;

			// Direction that ball is going is normalized velocity
			glm::vec3 directionBall = glm::normalize(pSphere->getVelocity());	// 1.0f

			// Calcualte direction to move it back the way it came from
			glm::vec3 oppositeDirection = -directionBall;				// 1.0f

			float distanceToClosestPoint = glm::length(CentreToClosestPoint);

			pDebugRenderer->addLine(pSphere->getPositionXYZ(),
				closestPoint,
				glm::vec3(0.0f, 1.0f, 0.0f),
				1.0f);

		}// end for

		//*************************************
		//	Ramps
		//*************************************

		for (int k = 0; k < ::g_vec_pRampObjects.size(); k++)
		{
			glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
			cPhysics::sPhysicsTriangle closestTriangle;

			glm::mat4 matWorld = calculateWorldMatrix(g_vec_pRampObjects.at(k));

			cMesh transformedMesh;
			pPhsyics->CalculateTransformedMesh(meshVec.at(6), matWorld, transformedMesh);

			pPhsyics->GetClosestTriangleToPoint(pSphere->getPositionXYZ(), transformedMesh, closestPoint, closestTriangle);

			// Highlight the triangle that I'm closest to
			pDebugRenderer->addTriangle(closestTriangle.verts[0],
				closestTriangle.verts[1],
				closestTriangle.verts[2],
				glm::vec3(1.0f, 0.0f, 0.0f));

			// Highlight the triangle that I'm closest to
			// To draw the normal, calculate the average of the 3 vertices, 
			// then draw that average + the normal (the normal starts at the 0,0,0 OF THE TRIANGLE)
			glm::vec3 centreOfTriangle = (closestTriangle.verts[0] +
				closestTriangle.verts[1] +
				closestTriangle.verts[2]) / 3.0f;		// Average

			glm::vec3 normalInWorld = centreOfTriangle + (closestTriangle.normal * 20.0f);	// Normal x 10 length

			pDebugRenderer->addLine(centreOfTriangle,
				normalInWorld,
				glm::vec3(1.0f, 1.0f, 0.0f));

			// Are we hitting the triangle? 
			float distance = glm::length(pSphere->getPositionXYZ() - closestPoint);

			if (distance <= pSphere->get_SPHERE_radius())
			{
				onGround = false;
				//if (k == 0)
				//{
				//	pSphere->inverseMass = 0.0f;
				//	pSphere->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
				//}

				// ************************************************************************

				// If you want, move the sphere back to where it just penetrated...
				// So that it will collide exactly where it's supposed to. 
				// But, that's not a big problem.

				// 1. Calculate vector from centre of sphere to closest point
				glm::vec3 vecSphereToClosestPoint = closestPoint - pSphere->getPositionXYZ();

				// 2. Get the length of this vector
				float centreToContractDistance = glm::length(vecSphereToClosestPoint);

				// 3. Create a vector from closest point to radius
				float lengthPositionAdjustment = pSphere->get_SPHERE_radius() - centreToContractDistance;

				// 4. Sphere is moving in the direction of the velocity, so 
				//    we want to move the sphere BACK along this velocity vector
				glm::vec3 vecDirection = glm::normalize(pSphere->getVelocity());

				glm::vec3 vecPositionAdjust = (-vecDirection) * lengthPositionAdjustment;

				// 5. Reposition sphere 
				pSphere->setPositionXYZ(pSphere->getPositionXYZ() + vecPositionAdjust);

				glm::vec3 velocityVector = glm::normalize(pSphere->getVelocity());
				float gravY = (-pSphere->getVelocity().y) * 0.25f;
				glm::vec3 gravity = glm::vec3(0.0f, gravY, 0.0f);

				// closestTriangle.normal
				glm::vec3 reflectionVec = glm::reflect(velocityVector, closestTriangle.normal);
				reflectionVec = glm::normalize(reflectionVec);

				// Stop the sphere and draw the two vectors...
	//			pSphere->inverseMass = 0.0f;	// Stopped

				glm::vec3 velVecX20 = velocityVector * 10.0f;
				pDebugRenderer->addLine(closestPoint, velVecX20,
					glm::vec3(1.0f, 0.0f, 0.0f), 30.0f /*seconds*/);

				glm::vec3 reflectionVecX20 = reflectionVec * 10.0f;
				pDebugRenderer->addLine(closestPoint, reflectionVecX20,
					glm::vec3(0.0f, 1.0f, 1.0f), 30.0f /*seconds*/);

				// Change the direction of the ball (the bounce off the triangle)

				// Get lenght of the velocity vector
				float speed = glm::length(pSphere->getVelocity());

				/*if (pSphere->velocity.y * -1 < 1 && !onPlatform)
				{
					onPlatform = true;
					properPlatformYPosition = pSphere->positionXYZ.y;
				}
				if (!onPlatform)
				{*/
				pSphere->setVelocity((reflectionVec * speed) - gravity);
				//}
				/*else if (onPlatform && properPlatformYPosition != 0.0f)
				{
					pSphere->velocity = reflectionVec * speed;
					pSphere->positionXYZ.y = properPlatformYPosition;
				}*/
			}
			// A more general 
			pPhsyics->TestForCollisions(::g_vec_pGameObjects);
			pPhsyics->TestForCollisions(::g_vec_pWallObjects);
			pPhsyics->TestForCollisions(::g_vec_pRampObjects);
			pPhsyics->TestForCollisions(::g_vec_pBallObjects);

			//{// Draw closest point
			//	glm::mat4 matModel = glm::mat4(1.0f);
			//	pDebugSphere->positionXYZ = closestPoint;
			//	pDebugSphere->scale = 1.0f;
			//	pDebugSphere->debugColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
			//	pDebugSphere->isWireframe = true;
			//	DrawObject(matModel, pDebugSphere,
			//		shaderProgID, pTheVAOManager);
			//}


			// How far did we penetrate the surface?
			glm::vec3 CentreToClosestPoint = pSphere->getPositionXYZ() - closestPoint;

			// Direction that ball is going is normalized velocity
			glm::vec3 directionBall = glm::normalize(pSphere->getVelocity());	// 1.0f

			// Calcualte direction to move it back the way it came from
			glm::vec3 oppositeDirection = -directionBall;				// 1.0f

			float distanceToClosestPoint = glm::length(CentreToClosestPoint);

			pDebugRenderer->addLine(pSphere->getPositionXYZ(),
				closestPoint,
				glm::vec3(0.0f, 1.0f, 0.0f),
				1.0f);
		}// end ramp for

		//*************************************
		//	Balls
		//*************************************

		for (int k = 0; k < ::g_vec_pBallObjects.size(); k++)
		{
			glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
			cPhysics::sPhysicsTriangle closestTriangle;

			glm::mat4 matWorld = calculateWorldMatrix(g_vec_pBallObjects.at(k));

			cMesh transformedMesh;
			pPhsyics->CalculateTransformedMesh(meshVec.at(6), matWorld, transformedMesh);

			pPhsyics->GetClosestTriangleToPoint(pSphere->getPositionXYZ(), transformedMesh, closestPoint, closestTriangle);

			// Highlight the triangle that I'm closest to
			pDebugRenderer->addTriangle(closestTriangle.verts[0],
				closestTriangle.verts[1],
				closestTriangle.verts[2],
				glm::vec3(1.0f, 0.0f, 0.0f));

			// Highlight the triangle that I'm closest to
			// To draw the normal, calculate the average of the 3 vertices, 
			// then draw that average + the normal (the normal starts at the 0,0,0 OF THE TRIANGLE)
			glm::vec3 centreOfTriangle = (closestTriangle.verts[0] +
				closestTriangle.verts[1] +
				closestTriangle.verts[2]) / 3.0f;		// Average

			glm::vec3 normalInWorld = centreOfTriangle + (closestTriangle.normal * 20.0f);	// Normal x 10 length

			pDebugRenderer->addLine(centreOfTriangle,
				normalInWorld,
				glm::vec3(1.0f, 1.0f, 0.0f));

			// Are we hitting the triangle? 
			float distance = glm::length(pSphere->getPositionXYZ() - closestPoint);

			if (distance <= pSphere->get_SPHERE_radius())
			{
				onGround = false;
				g_vec_pBallObjects.at(k)->setObjectColourRGBA(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));


				//if (k == 0)
				//{
				//	pSphere->inverseMass = 0.0f;
				//	pSphere->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
				//}

				// ************************************************************************

				// If you want, move the sphere back to where it just penetrated...
				// So that it will collide exactly where it's supposed to. 
				// But, that's not a big problem.

				// 1. Calculate vector from centre of sphere to closest point
				glm::vec3 vecSphereToClosestPoint = closestPoint - pSphere->getPositionXYZ();

				// 2. Get the length of this vector
				float centreToContractDistance = glm::length(vecSphereToClosestPoint);

				// 3. Create a vector from closest point to radius
				float lengthPositionAdjustment = pSphere->get_SPHERE_radius() - centreToContractDistance;

				// 4. Sphere is moving in the direction of the velocity, so 
				//    we want to move the sphere BACK along this velocity vector
				glm::vec3 vecDirection = glm::normalize(pSphere->getVelocity());

				glm::vec3 vecPositionAdjust = (-vecDirection) * lengthPositionAdjustment;

				// 5. Reposition sphere 
				pSphere->setPositionXYZ(pSphere->getPositionXYZ() + (vecPositionAdjust));

				glm::vec3 velocityVector = glm::normalize(pSphere->getVelocity());
				float gravY = (-pSphere->getVelocity().y) * 0.25f;
				glm::vec3 gravity = glm::vec3(0.0f, gravY, 0.0f);

				// closestTriangle.normal
				glm::vec3 reflectionVec = glm::reflect(velocityVector, closestTriangle.normal);
				reflectionVec = glm::normalize(reflectionVec);

				// Stop the sphere and draw the two vectors...
	//			pSphere->inverseMass = 0.0f;	// Stopped

				glm::vec3 velVecX20 = velocityVector * 10.0f;
				pDebugRenderer->addLine(closestPoint, velVecX20,
					glm::vec3(1.0f, 0.0f, 0.0f), 30.0f /*seconds*/);

				glm::vec3 reflectionVecX20 = reflectionVec * 10.0f;
				pDebugRenderer->addLine(closestPoint, reflectionVecX20,
					glm::vec3(0.0f, 1.0f, 1.0f), 30.0f /*seconds*/);

				// Change the direction of the ball (the bounce off the triangle)

				// Get lenght of the velocity vector
				float speed = glm::length(pSphere->getVelocity());

				/*if (pSphere->velocity.y * -1 < 1 && !onPlatform)
				{
					onPlatform = true;
					properPlatformYPosition = pSphere->positionXYZ.y;
				}
				if (!onPlatform)
				{*/
				g_vec_pBallObjects.at(k)->setVelocity(pSphere->getVelocity() - gravity);
				pSphere->setVelocity((reflectionVec * speed) - gravity);

				//}
				/*else if (onPlatform && properPlatformYPosition != 0.0f)
				{
					pSphere->velocity = reflectionVec * speed;
					pSphere->positionXYZ.y = properPlatformYPosition;
				}*/
			}
			// A more general 
			pPhsyics->TestForCollisions(::g_vec_pGameObjects);
			pPhsyics->TestForCollisions(::g_vec_pWallObjects);
			pPhsyics->TestForCollisions(::g_vec_pRampObjects);
			pPhsyics->TestForCollisions(::g_vec_pBallObjects);

			//{// Draw closest point
			//	glm::mat4 matModel = glm::mat4(1.0f);
			//	pDebugSphere->positionXYZ = closestPoint;
			//	pDebugSphere->scale = 1.0f;
			//	pDebugSphere->debugColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
			//	pDebugSphere->isWireframe = true;
			//	DrawObject(matModel, pDebugSphere,
			//		shaderProgID, pTheVAOManager);
			//}


			// How far did we penetrate the surface?
			glm::vec3 CentreToClosestPoint = pSphere->getPositionXYZ() - closestPoint;

			// Direction that ball is going is normalized velocity
			glm::vec3 directionBall = glm::normalize(pSphere->getVelocity());	// 1.0f

			// Calcualte direction to move it back the way it came from
			glm::vec3 oppositeDirection = -directionBall;				// 1.0f

			float distanceToClosestPoint = glm::length(CentreToClosestPoint);

			pDebugRenderer->addLine(pSphere->getPositionXYZ(),
				closestPoint,
				glm::vec3(0.0f, 1.0f, 0.0f),
				1.0f);
		}// end ball for

		for (int j = 0; j < ::g_vec_pBallObjects.size(); j++)
		{
			/*if (g_vec_pBallObjects.at(j)->positionXYZ.y < 101)
			{
				g_vec_pBallObjects.at(j)->inverseMass = 0.0f;
				g_vec_pBallObjects.at(j)->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
			}*/

			//*************************************
			//	Ramps (for balls)
			//*************************************

			for (int k = 0; k < ::g_vec_pRampObjects.size(); k++)
			{
				glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
				cPhysics::sPhysicsTriangle closestTriangle;

				glm::mat4 matWorld = calculateWorldMatrix(g_vec_pRampObjects.at(k));

				cMesh transformedMesh;
				pPhsyics->CalculateTransformedMesh(meshVec.at(6), matWorld, transformedMesh);

				pPhsyics->GetClosestTriangleToPoint(g_vec_pBallObjects.at(j)->getPositionXYZ(), transformedMesh, closestPoint, closestTriangle);

				// Highlight the triangle that I'm closest to
				pDebugRenderer->addTriangle(closestTriangle.verts[0],
					closestTriangle.verts[1],
					closestTriangle.verts[2],
					glm::vec3(1.0f, 0.0f, 0.0f));

				// Highlight the triangle that I'm closest to
				// To draw the normal, calculate the average of the 3 vertices, 
				// then draw that average + the normal (the normal starts at the 0,0,0 OF THE TRIANGLE)
				glm::vec3 centreOfTriangle = (closestTriangle.verts[0] +
					closestTriangle.verts[1] +
					closestTriangle.verts[2]) / 3.0f;		// Average

				glm::vec3 normalInWorld = centreOfTriangle + (closestTriangle.normal * 20.0f);	// Normal x 10 length

				pDebugRenderer->addLine(centreOfTriangle,
					normalInWorld,
					glm::vec3(1.0f, 1.0f, 0.0f));

				// Are we hitting the triangle? 
				float distance = glm::length(g_vec_pBallObjects.at(j)->getPositionXYZ() - closestPoint);

				if (distance <= g_vec_pBallObjects.at(j)->get_SPHERE_radius())
				{
					//if (k == 0)
					//{
					//	g_vec_pBallObjects.at(j)->inverseMass = 0.0f;
					//	g_vec_pBallObjects.at(j)->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
					//}

					// ************************************************************************

					// If you want, move the sphere back to where it just penetrated...
					// So that it will collide exactly where it's supposed to. 
					// But, that's not a big problem.

					// 1. Calculate vector from centre of sphere to closest point
					glm::vec3 vecSphereToClosestPoint = closestPoint - g_vec_pBallObjects.at(j)->getPositionXYZ();

					// 2. Get the length of this vector
					float centreToContractDistance = glm::length(vecSphereToClosestPoint);

					// 3. Create a vector from closest point to radius
					float lengthPositionAdjustment = g_vec_pBallObjects.at(j)->get_SPHERE_radius() - centreToContractDistance;

					// 4. Sphere is moving in the direction of the velocity, so 
					//    we want to move the sphere BACK along this velocity vector
					glm::vec3 vecDirection = glm::normalize(g_vec_pBallObjects.at(j)->getVelocity());

					glm::vec3 vecPositionAdjust = (-vecDirection) * lengthPositionAdjustment;

					// 5. Reposition sphere 
					g_vec_pBallObjects.at(j)->setPositionXYZ(g_vec_pBallObjects.at(j)->getPositionXYZ() + (vecPositionAdjust));

					glm::vec3 velocityVector = glm::normalize(g_vec_pBallObjects.at(j)->getVelocity());
					float gravY = (-g_vec_pBallObjects.at(j)->getVelocity().y) * 0.25f;
					glm::vec3 gravity = glm::vec3(0.0f, gravY, 0.0f);

					// closestTriangle.normal
					glm::vec3 reflectionVec = glm::reflect(velocityVector, closestTriangle.normal);
					reflectionVec = glm::normalize(reflectionVec);

					// Stop the sphere and draw the two vectors...
		//			g_vec_pBallObjects.at(j)->inverseMass = 0.0f;	// Stopped

					glm::vec3 velVecX20 = velocityVector * 10.0f;
					pDebugRenderer->addLine(closestPoint, velVecX20,
						glm::vec3(1.0f, 0.0f, 0.0f), 30.0f /*seconds*/);

					glm::vec3 reflectionVecX20 = reflectionVec * 10.0f;
					pDebugRenderer->addLine(closestPoint, reflectionVecX20,
						glm::vec3(0.0f, 1.0f, 1.0f), 30.0f /*seconds*/);

					// Change the direction of the ball (the bounce off the triangle)

					// Get lenght of the velocity vector
					float speed = glm::length(g_vec_pBallObjects.at(j)->getVelocity());

					/*if (g_vec_pBallObjects.at(j)->velocity.y * -1 < 1 && !onPlatform)
					{
						onPlatform = true;
						properPlatformYPosition = g_vec_pBallObjects.at(j)->positionXYZ.y;
					}
					if (!onPlatform)
					{*/
					g_vec_pBallObjects.at(j)->setVelocity((reflectionVec * speed) - gravity);
					//}
					/*else if (onPlatform && properPlatformYPosition != 0.0f)
					{
						g_vec_pBallObjects.at(j)->velocity = reflectionVec * speed;
						g_vec_pBallObjects.at(j)->positionXYZ.y = properPlatformYPosition;
					}*/
				}
				// A more general 
				pPhsyics->TestForCollisions(::g_vec_pGameObjects);
				pPhsyics->TestForCollisions(::g_vec_pWallObjects);
				pPhsyics->TestForCollisions(::g_vec_pRampObjects);
				pPhsyics->TestForCollisions(::g_vec_pBallObjects);

				//{// Draw closest point
				//	glm::mat4 matModel = glm::mat4(1.0f);
				//	pDebugSphere->positionXYZ = closestPoint;
				//	pDebugSphere->scale = 1.0f;
				//	pDebugSphere->debugColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
				//	pDebugSphere->isWireframe = true;
				//	DrawObject(matModel, pDebugSphere,
				//		shaderProgID, pTheVAOManager);
				//}


				// How far did we penetrate the surface?
				glm::vec3 CentreToClosestPoint = g_vec_pBallObjects.at(j)->getPositionXYZ() - closestPoint;

				// Direction that ball is going is normalized velocity
				glm::vec3 directionBall = glm::normalize(g_vec_pBallObjects.at(j)->getVelocity());	// 1.0f

				// Calcualte direction to move it back the way it came from
				glm::vec3 oppositeDirection = -directionBall;				// 1.0f

				float distanceToClosestPoint = glm::length(CentreToClosestPoint);

				pDebugRenderer->addLine(g_vec_pBallObjects.at(j)->getPositionXYZ(),
					closestPoint,
					glm::vec3(0.0f, 1.0f, 0.0f),
					1.0f);
			}// end ramp for
			//*************************************
			//	Walls (for balls)
			//*************************************

			for (int k = 0; k < ::g_vec_pWallObjects.size(); k++)
			{
				glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
				cPhysics::sPhysicsTriangle closestTriangle;

				glm::mat4 matWorld = calculateWorldMatrix(g_vec_pWallObjects.at(k));

				cMesh transformedMesh;
				pPhsyics->CalculateTransformedMesh(meshVec.at(5), matWorld, transformedMesh);

				pPhsyics->GetClosestTriangleToPoint(g_vec_pBallObjects.at(j)->getPositionXYZ(), transformedMesh, closestPoint, closestTriangle);

				// Highlight the triangle that I'm closest to
				pDebugRenderer->addTriangle(closestTriangle.verts[0],
					closestTriangle.verts[1],
					closestTriangle.verts[2],
					glm::vec3(1.0f, 0.0f, 0.0f));

				// Highlight the triangle that I'm closest to
				// To draw the normal, calculate the average of the 3 vertices, 
				// then draw that average + the normal (the normal starts at the 0,0,0 OF THE TRIANGLE)
				glm::vec3 centreOfTriangle = (closestTriangle.verts[0] +
					closestTriangle.verts[1] +
					closestTriangle.verts[2]) / 3.0f;		// Average

				glm::vec3 normalInWorld = centreOfTriangle + (closestTriangle.normal * 20.0f);	// Normal x 10 length

				pDebugRenderer->addLine(centreOfTriangle,
					normalInWorld,
					glm::vec3(1.0f, 1.0f, 0.0f));

				// Are we hitting the triangle? 
				float distance = glm::length(g_vec_pBallObjects.at(j)->getPositionXYZ() - closestPoint);

				if (distance <= g_vec_pBallObjects.at(j)->get_SPHERE_radius())
				{
					if (k == 3)
					{
						g_vec_pBallObjects.at(j)->setInverseMass(0.0f);
						g_vec_pBallObjects.at(j)->setPositionXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
					}

					// ************************************************************************

					// If you want, move the sphere back to where it just penetrated...
					// So that it will collide exactly where it's supposed to. 
					// But, that's not a big problem.

					// 1. Calculate vector from centre of sphere to closest point
					glm::vec3 vecSphereToClosestPoint = closestPoint - g_vec_pBallObjects.at(j)->getPositionXYZ();

					// 2. Get the length of this vector
					float centreToContractDistance = glm::length(vecSphereToClosestPoint);

					// 3. Create a vector from closest point to radius
					float lengthPositionAdjustment = g_vec_pBallObjects.at(j)->get_SPHERE_radius() - centreToContractDistance;

					// 4. Sphere is moving in the direction of the velocity, so 
					//    we want to move the sphere BACK along this velocity vector
					glm::vec3 vecDirection = glm::normalize(g_vec_pBallObjects.at(j)->getVelocity());

					glm::vec3 vecPositionAdjust = (-vecDirection) * lengthPositionAdjustment;

					// 5. Reposition sphere 
					g_vec_pBallObjects.at(j)->setPositionXYZ(g_vec_pBallObjects.at(j)->getPositionXYZ() + (vecPositionAdjust));

					glm::vec3 velocityVector = glm::normalize(g_vec_pBallObjects.at(j)->getVelocity());
					float gravY = (-g_vec_pBallObjects.at(j)->getVelocity().y) * 0.25f;
					glm::vec3 gravity = glm::vec3(0.0f, gravY, 0.0f);

					// closestTriangle.normal
					glm::vec3 reflectionVec = glm::reflect(velocityVector, closestTriangle.normal);
					reflectionVec = glm::normalize(reflectionVec);

					// Stop the sphere and draw the two vectors...
		//			g_vec_pBallObjects.at(j)->inverseMass = 0.0f;	// Stopped

					glm::vec3 velVecX20 = velocityVector * 10.0f;
					pDebugRenderer->addLine(closestPoint, velVecX20,
						glm::vec3(1.0f, 0.0f, 0.0f), 30.0f /*seconds*/);

					glm::vec3 reflectionVecX20 = reflectionVec * 10.0f;
					pDebugRenderer->addLine(closestPoint, reflectionVecX20,
						glm::vec3(0.0f, 1.0f, 1.0f), 30.0f /*seconds*/);

					// Change the direction of the ball (the bounce off the triangle)

					// Get lenght of the velocity vector
					float speed = glm::length(g_vec_pBallObjects.at(j)->getVelocity());

					/*if (g_vec_pBallObjects.at(j)->velocity.y * -1 < 1 && !onPlatform)
					{
						onPlatform = true;
						properPlatformYPosition = g_vec_pBallObjects.at(j)->positionXYZ.y;
					}
					if (!onPlatform)
					{*/
					g_vec_pBallObjects.at(j)->setVelocity((reflectionVec * speed) - gravity);
					//}
					/*else if (onPlatform && properPlatformYPosition != 0.0f)
					{
						g_vec_pBallObjects.at(j)->velocity = reflectionVec * speed;
						g_vec_pBallObjects.at(j)->positionXYZ.y = properPlatformYPosition;
					}*/
				}
				// A more general 
				pPhsyics->TestForCollisions(::g_vec_pGameObjects);
				pPhsyics->TestForCollisions(::g_vec_pWallObjects);
				pPhsyics->TestForCollisions(::g_vec_pRampObjects);
				pPhsyics->TestForCollisions(::g_vec_pBallObjects);

				//{// Draw closest point
				//	glm::mat4 matModel = glm::mat4(1.0f);
				//	pDebugSphere->positionXYZ = closestPoint;
				//	pDebugSphere->scale = 1.0f;
				//	pDebugSphere->debugColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
				//	pDebugSphere->isWireframe = true;
				//	DrawObject(matModel, pDebugSphere,
				//		shaderProgID, pTheVAOManager);
				//}


				// How far did we penetrate the surface?
				glm::vec3 CentreToClosestPoint = g_vec_pBallObjects.at(j)->getPositionXYZ() - closestPoint;

				// Direction that ball is going is normalized velocity
				glm::vec3 directionBall = glm::normalize(g_vec_pBallObjects.at(j)->getVelocity());	// 1.0f

				// Calcualte direction to move it back the way it came from
				glm::vec3 oppositeDirection = -directionBall;				// 1.0f

				float distanceToClosestPoint = glm::length(CentreToClosestPoint);

				pDebugRenderer->addLine(g_vec_pBallObjects.at(j)->getPositionXYZ(),
					closestPoint,
					glm::vec3(0.0f, 1.0f, 0.0f),
					1.0f);
			}// end wall for
		}// end ball for

		// HACK
		//if (DidBallCollideWithGround)
		//{ 
		//	float sphereRadius = 1.0f;
		//	float distanceToMoveBack = sphereRadius - distanceToClosestPoint;

		//	glm::vec3 adjustmentVector = oppositeDirection * distanceToMoveBack;

		//	// Let's move the sphere that amount...
		//	pSphere->positionXYZ += adjustmentVector;
		//}


		//std::cout 
		//	<< pSphere->velocity.x << ", "
		//	<< pSphere->velocity.y << ", "
		//	<< pSphere->velocity.z << "   dist = "
		//	<< distanceToClosestPoint << std::endl;

		//howMuchToMoveItBack = 1.0 - lenthOfThatVector


		if (bLightDebugSheresOn)
		{
			{// Draw where the light is at
				for (int i = 0; i < pMediator->m_vec_pLights.size(); ++i)
				{
					glm::mat4 matModel = glm::mat4(1.0f);
					pDebugSphere->setPositionXYZ(pMediator->m_vec_pLights.at(i)->getPositionXYZ());
					pDebugSphere->setScale(0.5f);
					pDebugSphere->setDebugColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					pDebugSphere->setIsWireframe(true);
					DrawObject(matModel, pDebugSphere,
						shaderProgID, pTheVAOManager);
					pDebugSphere->setIsVisible(true);
				}
				/*
				//glm::mat4 matModel1 = glm::mat4(1.0f);
				//pDebugSphere->positionXYZ = corner1LightPosition;
				//pDebugSphere->scale = 0.5f;
				//pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				//pDebugSphere->isWireframe = true;
				//DrawObject(matModel1, pDebugSphere,
				//	shaderProgID, pTheVAOManager);
				//
				glm::mat4 matModel2 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight2;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel2, pDebugSphere,
					shaderProgID, pTheVAOManager);
					//
				glm::mat4 matModel3 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight3;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel3, pDebugSphere,
					shaderProgID, pTheVAOManager);

				glm::mat4 matModel4 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight4;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel4, pDebugSphere,
					shaderProgID, pTheVAOManager);

				glm::mat4 matModel5 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight5;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel5, pDebugSphere,
					shaderProgID, pTheVAOManager);

				glm::mat4 matModel6 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight6;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel6, pDebugSphere,
					shaderProgID, pTheVAOManager);*/
			}

			// Draw spheres to represent the attenuation...
			{   // Draw a sphere at 1% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pMediator->m_vec_pLights.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.01f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pMediator->m_vec_pLights.at(currentLight)->getConstAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 25% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pMediator->m_vec_pLights.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.25f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pMediator->m_vec_pLights.at(currentLight)->getConstAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 50% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pMediator->m_vec_pLights.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.50f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pMediator->m_vec_pLights.at(currentLight)->getConstAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 75% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pMediator->m_vec_pLights.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.75f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pMediator->m_vec_pLights.at(currentLight)->getConstAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 95% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pMediator->m_vec_pLights.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.95f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pMediator->m_vec_pLights.at(currentLight)->getConstAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getLinearAtten(),
					pMediator->m_vec_pLights.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
		}// if (bLightDebugSheresOn) 

		 // **************************************************
		// *************************************************
		if (fileChanged)
		{
			//file.open(gameDataLocation);
			file << "<?xml version='1.0' encoding='utf-8'?>\n";
			document.save_file(gameDataLocation.c_str());
			//file.close();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}// main loop

	glfwDestroyWindow(window);
	glfwTerminate();

	// Delete everything
	delete pTheModelLoader;
	//	delete pTheVAOManager;

		// Watch out!!
		// sVertex* pVertices = new sVertex[numberOfVertsOnGPU];
	//	delete [] pVertices;		// If it's an array, also use [] bracket

	exit(EXIT_SUCCESS);
}


void DrawObject(glm::mat4 m, iObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager)
{
	// 
				//         mat4x4_identity(m);
	m = glm::mat4(1.0f);



	// ******* TRANSLATION TRANSFORM *********
	glm::mat4 matTrans
		= glm::translate(glm::mat4(1.0f),
			glm::vec3(pCurrentObject->getPositionXYZ().x,
				pCurrentObject->getPositionXYZ().y,
				pCurrentObject->getPositionXYZ().z));
	m = m * matTrans;
	// ******* TRANSLATION TRANSFORM *********



	// ******* ROTATION TRANSFORM *********
	//mat4x4_rotate_Z(m, m, (float) glfwGetTime());
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().z,					// Angle 
		glm::vec3(0.0f, 0.0f, 1.0f));
	m = m * rotateZ;

	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().y,	//(float)glfwGetTime(),					// Angle 
		glm::vec3(0.0f, 1.0f, 0.0f));
	m = m * rotateY;

	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().x,	// (float)glfwGetTime(),					// Angle 
		glm::vec3(1.0f, 0.0f, 0.0f));
	m = m * rotateX;
	// ******* ROTATION TRANSFORM *********



	// ******* SCALE TRANSFORM *********
	glm::mat4 scale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentObject->getScale(),
			pCurrentObject->getScale(),
			pCurrentObject->getScale()));
	m = m * scale;
	// ******* SCALE TRANSFORM *********



	//mat4x4_mul(mvp, p, m);
	//mvp = p * v * m;

	// Choose which shader to use
	//glUseProgram(program);
	glUseProgram(shaderProgID);


	//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
	//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

	//uniform mat4 matModel;		// Model or World 
	//uniform mat4 matView; 		// View or camera
	//uniform mat4 matProj;
	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");

	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(m));
	//glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(v));
	//glUniformMatrix4fv(matProj_UL, 1, GL_FALSE, glm::value_ptr(p));



	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");

	glUniform3f(newColour_location,
		pCurrentObject->getObjectColourRGBA().r,
		pCurrentObject->getObjectColourRGBA().g,
		pCurrentObject->getObjectColourRGBA().b);

	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	glUniform4f(diffuseColour_UL,
		pCurrentObject->getObjectColourRGBA().r,
		pCurrentObject->getObjectColourRGBA().g,
		pCurrentObject->getObjectColourRGBA().b,
		pCurrentObject->getObjectColourRGBA().a);	// 

	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");
	glUniform4f(specularColour_UL,
		1.0f,	// R
		1.0f,	// G
		1.0f,	// B
		1000.0f);	// Specular "power" (how shinny the object is)
					// 1.0 to really big (10000.0f)


//uniform vec4 debugColour;
//uniform bool bDoNotLight;
	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	if (pCurrentObject->getIsWireframe())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// LINES
		glUniform4f(debugColour_UL,
			pCurrentObject->getDebugColour().r,
			pCurrentObject->getDebugColour().g,
			pCurrentObject->getDebugColour().b,
			pCurrentObject->getDebugColour().a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}
	else
	{	// Regular object (lit and not wireframe)
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		// SOLID
	}
	//glPointSize(15.0f);

	if (pCurrentObject->getDisableDepthBufferTest())
	{
		glDisable(GL_DEPTH_TEST);					// DEPTH Test OFF
	}
	else
	{
		glEnable(GL_DEPTH_TEST);						// Turn ON depth test
	}

	if (pCurrentObject->getDisableDepthBufferWrite())
	{
		glDisable(GL_DEPTH);						// DON'T Write to depth buffer
	}
	else
	{
		glEnable(GL_DEPTH);								// Write to depth buffer
	}


	//		glDrawArrays(GL_TRIANGLES, 0, 2844);
	//		glDrawArrays(GL_TRIANGLES, 0, numberOfVertsOnGPU);

	sModelDrawInfo drawInfo;
	//if (pTheVAOManager->FindDrawInfoByModelName("bunny", drawInfo))
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->getMeshName(), drawInfo))
	{
		glBindVertexArray(drawInfo.VAO_ID);
		glDrawElements(GL_TRIANGLES,
			drawInfo.numberOfIndices,
			GL_UNSIGNED_INT,
			0);
		glBindVertexArray(0);
	}

	return;
} // DrawObject;
// 

// returns NULL (0) if we didn't find it.
iObject* pFindObjectByFriendlyName(std::string name)
{
	// Do a linear search 
	for (unsigned int index = 0;
		index != g_vec_pGameObjects.size(); index++)
	{
		if (::g_vec_pGameObjects[index]->getFriendlyName() == name)
		{
			// Found it!!
			return ::g_vec_pGameObjects[index];
		}
	}
	for (unsigned int index = 0;
		index != g_vec_pWallObjects.size(); index++)
	{
		if (::g_vec_pWallObjects[index]->getFriendlyName() == name)
		{
			// Found it!!
			return ::g_vec_pWallObjects[index];
		}
	}
	// Didn't find it
	return NULL;
}

// returns NULL (0) if we didn't find it.
iObject* pFindObjectByFriendlyNameMap(std::string name)
{
	//std::map<std::string, cGameObject*> g_map_GameObjectsByFriendlyName;
	return ::g_map_GameObjectsByFriendlyName[name];
}

glm::mat4 calculateWorldMatrix(iObject* pCurrentObject)
{

	glm::mat4 matWorld = glm::mat4(1.0f);


	// ******* TRANSLATION TRANSFORM *********
	glm::mat4 matTrans
		= glm::translate(glm::mat4(1.0f),
			glm::vec3(pCurrentObject->getPositionXYZ().x,
				pCurrentObject->getPositionXYZ().y,
				pCurrentObject->getPositionXYZ().z));
	matWorld = matWorld * matTrans;
	// ******* TRANSLATION TRANSFORM *********



	// ******* ROTATION TRANSFORM *********
	//mat4x4_rotate_Z(m, m, (float) glfwGetTime());
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().z,					// Angle 
		glm::vec3(0.0f, 0.0f, 1.0f));
	matWorld = matWorld * rotateZ;

	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().y,	//(float)glfwGetTime(),					// Angle 
		glm::vec3(0.0f, 1.0f, 0.0f));
	matWorld = matWorld * rotateY;

	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().x,	// (float)glfwGetTime(),					// Angle 
		glm::vec3(1.0f, 0.0f, 0.0f));
	matWorld = matWorld * rotateX;
	// ******* ROTATION TRANSFORM *********



	// ******* SCALE TRANSFORM *********
	glm::mat4 scale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentObject->getScale(),
			pCurrentObject->getScale(),
			pCurrentObject->getScale()));
	matWorld = matWorld * scale;
	// ******* SCALE TRANSFORM *********


	return matWorld;
}
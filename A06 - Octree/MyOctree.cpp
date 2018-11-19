#include "MyOctree.h"
using namespace Simplex;
//  MyOctree
uint MyOctree::numOctree = 0;
uint MyOctree::maxLevel = 0;
uint MyOctree::idealCount = 0;
void MyOctree::Init(void)
{
	m_nData = 0; //from base class

	//initialize values for this octant to 0 
	numChildren = 0;
	m_fSize = 0.0f;
	m_iID = numOctree;
	m_iLevel = 0;
	v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	//managers
	m_pMeshMngr = MeshManager::GetInstance();
	m_pMyEntityMngr = MyEntityManager::GetInstance();

	//set pointers to null 
	m_pRoot = nullptr;
	m_pParent = nullptr;

	for (size_t i = 0; i < 8; i++)
	{
		m_pChildren[i] = nullptr;
	}

}
//The big 3
Simplex::MyOctree::MyOctree(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	//set values for desired octree
	numOctree = 0;
	maxLevel = a_nMaxLevel;
	idealCount = a_nIdealEntityCount;
	m_iID = numOctree;

	m_pRoot = this;
	m_lChildren.clear();

	//hold the bois for this octant
	std::vector<vector3> lMinMax;

	uint numEntities = m_pMyEntityMngr->GetEntityCount();
	for (size_t i = 0; i < numEntities; i++)
	{
		MyEntity* pEntity = m_pMyEntityMngr->GetEntity(i);
		MyRigidBody* pRB = pEntity->GetRigidBody();

		lMinMax.push_back(pRB->GetMinGlobal());
		lMinMax.push_back(pRB->GetMaxGlobal());
	}

	MyRigidBody* pRigidBody = new MyRigidBody(lMinMax);

	vector3 halfWidth = pRigidBody->GetHalfWidth();

	float fMax = halfWidth.x;
	for (size_t i = 0; i < 3; i++)
	{
		if (fMax < halfWidth[i])
		{
			fMax = halfWidth[i];
		}
	}

	vector3 center = pRigidBody->GetCenterLocal();
	lMinMax.clear();
	SafeDelete(pRigidBody); //save the memory leaks lol


	m_fSize = fMax * 2.0f;
	v3Center = center;
	m_v3Min = v3Center - (vector3(fMax));
	m_v3Max = v3Center + (vector3(fMax));

	numOctree++;

	ConstructTree(maxLevel); //subdivide with a check
}
Simplex::MyOctree::MyOctree(vector3 a_v3Center, float a_fSize) //add a leaf
{
	Init();
	v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = v3Center + (vector3(m_fSize) / 2.0f);

	numOctree++;
}
MyOctree::MyOctree(MyOctree const& other)
{
	m_nData = other.m_nData;
	m_lData = other.m_lData;
}
MyOctree& MyOctree::operator=(MyOctree const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctree temp(other);
		Swap(temp);
	}
	return *this;
}
MyOctree::~MyOctree() { Release(); };
void Simplex::MyOctree::Subdivide()
{

	if (m_iLevel >= maxLevel) return; //make sure its not exceeding the max
	if (numChildren != 0) return; //if its not a leaf dont do it

	numChildren = 8;

	//calcuate the locations of the leafs and add them to the list
	float fSize = m_fSize / 4.0f;
	float fSizeD = m_fSize / 2.0f;
	vector3 fCenters;

	fCenters = v3Center;
	fCenters.x -= fSize;
	fCenters.y -= fSize;
	fCenters.z -= fSize;

	m_pChildren[0] = new MyOctree(fCenters, fSizeD);

	fCenters.x += fSizeD;
	m_pChildren[1] = new MyOctree(fCenters, fSizeD);

	fCenters.z += fSizeD;
	m_pChildren[2] = new MyOctree(fCenters, fSizeD);

	fCenters.x -= fSizeD;
	m_pChildren[3] = new MyOctree(fCenters, fSizeD);

	fCenters.y += fSizeD;
	m_pChildren[4] = new MyOctree(fCenters, fSizeD);

	fCenters.z -= fSizeD;
	m_pChildren[5] = new MyOctree(fCenters, fSizeD);

	fCenters.x += fSizeD;
	m_pChildren[6] = new MyOctree(fCenters, fSizeD);

	fCenters.z += fSizeD;
	m_pChildren[7] = new MyOctree(fCenters, fSizeD);

	for (size_t i = 0; i < 8; i++)
	{
		m_pChildren[i]->m_pRoot = m_pRoot;
		m_pChildren[i]->m_pParent = this;
		m_pChildren[i]->m_iLevel = m_iLevel + 1;
		m_pChildren[i]->idealCount = idealCount;
		if (m_pChildren[i]->ContainsMoreThan(idealCount))
		{
			m_pChildren[i]->Subdivide();
		}
	}

}

void Simplex::MyOctree::ConstructList() //keep a list of the children
{
	for (size_t i = 0; i < numChildren; i++)
	{
		m_pChildren[i]->ConstructList();
	}

	if (m_lEntities.size() > 0)
	{
		m_pRoot->m_lChildren.push_back(this);
	}
}
void MyOctree::Swap(MyOctree& other)
{
	std::swap(m_nData, other.m_nData);
	std::swap(m_lData, other.m_lData);
}
float Simplex::MyOctree::GetSize()
{
	return 0.0f;
}
vector3 Simplex::MyOctree::GetCenterGlobal()
{
	return v3Center;
}
vector3 Simplex::MyOctree::GetMinGlobal()
{
	return m_v3Min;
}
vector3 Simplex::MyOctree::GetMaxGlobal()
{
	return m_v3Max;
}
bool Simplex::MyOctree::IsColliding(uint a_iRBIndex) //check if the entity is in the octant
{
	int objectCount = m_pMyEntityMngr->GetEntityCount();
	if (a_iRBIndex >= objectCount) //not checking this boi 
	{
		return false;
	}

	MyEntity* entity = m_pMyEntityMngr->GetEntity(a_iRBIndex);
	MyRigidBody* rb = entity->GetRigidBody();
	vector3 min = rb->GetMinGlobal();
	vector3 max = rb->GetMaxGlobal();

	if (m_v3Max.x < min.x)
	{
		return false;
	}
	if (m_v3Min.x > max.x)
	{
		return false;
	}

	if (m_v3Max.y < min.y)
	{
		return false;
	}
	if (m_v3Min.y > max.y)
	{
		return false;
	}

	if (m_v3Max.z < min.z)
	{
		return false;
	}
	if (m_v3Min.z > max.z)
	{
		return false;
	}

	return true;
}
void Simplex::MyOctree::Display(uint a_iIndex, vector3 a_v3Color)
{
	if (m_iID == a_iIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}

	for (size_t i = 0; i < numChildren; i++)
	{
		m_pChildren[i]->Display(a_v3Color);
	}
}
void Simplex::MyOctree::Display(vector3 a_v3Color) //display based on size and location 
{
	for (size_t i = 0; i < numChildren; i++)
	{
		m_pChildren[i]->Display(a_v3Color);
	}

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void Simplex::MyOctree::ClearEntityList()
{
	for (size_t i = 0; i < numChildren; i++)
	{
		m_pChildren[i]->ClearEntityList();
	}
	m_lEntities.clear();
}


bool Simplex::MyOctree::ContainsMoreThan(uint a_nEntities) //check how many it contains against max
{
	int count = 0;
	int objectCount = m_pMyEntityMngr->GetEntityCount();
	for (size_t i = 0; i < objectCount; i++)
	{
		if (IsColliding(i)) //if there is an object in it, add to count
		{
			count++;
		}
		if (count > a_nEntities)
		{
			return true;
		}
	}
	return false;
}
void Simplex::MyOctree::DeleteChildren() //no memory leaks boi
{
	//loop through the children list
	for (size_t i = 0; i < numChildren; i++)
	{
		m_pChildren[i]->DeleteChildren();
		delete m_pChildren[i];
		m_pChildren[i] = nullptr;
	}
	numChildren = 0;
}
void Simplex::MyOctree::ConstructTree(uint a_iMaxLevel) //check if a subdivision needs to be done again or not
{
	if (m_iLevel != 0) return;

	maxLevel = a_iMaxLevel;
	numOctree = 1;
	m_lEntities.clear();
	DeleteChildren();
	m_lChildren.clear();

	if (ContainsMoreThan(idealCount)) //make sure its not past max
	{
		Subdivide();
	}

	AssignIDToEntity(); //put the entities into their octants (sort of)

	ConstructList();
}
void Simplex::MyOctree::AssignIDToEntity()
{
	//lop through children
	for (size_t i = 0; i < numChildren; i++)
	{
		m_pChildren[i]->AssignIDToEntity();
	}
	//no children -> leaf
	if (numChildren == 0)
	{
		int entities = m_pMyEntityMngr->GetEntityCount();
		for (size_t i = 0; i < entities; i++)
		{
			if (IsColliding(i)) //check if the entity is in the octant
			{
				m_lEntities.push_back(i);
				m_pMyEntityMngr->AddDimension(i, m_iID); //add the dimension
			}
		}
	}
}
uint Simplex::MyOctree::GetOctantCount()
{
	return numOctree;
}
void MyOctree::Release(void)
{
	if (m_iLevel == 0)
	{
		DeleteChildren();
	}
	numChildren = 0;
	m_fSize = 0.0f;
	m_lChildren.clear();

	
}

//Accessors
void MyOctree::SetData(int a_nData){ m_nData = a_nData; }
int MyOctree::GetData(void){ return m_nData; }
void MyOctree::SetDataOnVector(int a_nData){ m_lData.push_back(a_nData);}
int& MyOctree::GetDataOnVector(int a_nIndex)
{
	int nIndex = static_cast<int>(m_lData.size());
	assert(a_nIndex >= 0 && a_nIndex < nIndex);
	return m_lData[a_nIndex];
}
//--- Non Standard Singleton Methods


#include "MyOctant.h"
using namespace Simplex;
//  MyOctant
uint MyOctant::count = 0;
void MyOctant::Init(void)
{
	m_nData = 0;
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
	
	//IsColliding();
	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}

}

//The big 3
MyOctant::MyOctant() {
	Init();
	std::vector<MyEntity*> l_Entity_List = m_pEntityMngr->GetEntityList();
	uint iEntityCount = l_Entity_List.size();
	std::vector<vector3> v3MaxMin_list;

	//marker for root
	//constructor for non root subdivisions 
	for (uint i = 0; i < iEntityCount; i++)
	{
		MyRigidBody* pRG = l_Entity_List[i]->GetRigidBody();
		vector3 v3Min = pRG->GetMinGlobal();
		vector3 v3Max = pRG->GetMaxGlobal();
		v3MaxMin_list.push_back(v3Min);
		v3MaxMin_list.push_back(v3Max);


	}
	m_pRigidBody = new MyRigidBody(v3MaxMin_list);
	m_pRigidBody->MakeCubic();
	Subdivide();
}
Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();
	std::vector<vector3> v3MaxMin_list;
	v3MaxMin_list.push_back(vector3(a_v3Center - vector3(a_fSize)));
	v3MaxMin_list.push_back(vector3(a_v3Center + vector3(a_fSize)));
	m_pRigidBody = new MyRigidBody(v3MaxMin_list);
	count++;
	m_iID = count;
	
	//what level to go into
	//hoe many objects per cotant
}
MyOctant::MyOctant(MyOctant const& other)
{
	m_nData = other.m_nData;
	m_lData = other.m_lData;
}
MyOctant& MyOctant::operator=(MyOctant const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}
MyOctant::~MyOctant() { Release(); };

void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_nData, other.m_nData);
	std::swap(m_lData, other.m_lData);
}
void MyOctant::Release(void)
{
	//release children 
	delete[] m_pChild;
	m_lData.clear();
}
void Simplex::MyOctant::Display(void)
{
	m_pRigidBody->AddToRenderList();
	for(uint i = 0; i < 8; i++)
	{
		if (m_pChild[i])
		{
			m_pChild[i]->Display();
		}
	}
	//m_pMeshMngr->AddWireCubeToRenderList(glm::scale(vector3(70)), C_BLUE);
}
void Simplex::MyOctant::IsColliding(void)
{
	std::vector<MyEntity*> l_Entity_List = m_pEntityMngr->GetEntityList();
	uint iEntityCount = l_Entity_List.size();

	for (uint i = 0; i < l_Entity_List.size(); i++)
	{
		MyRigidBody* pRB = l_Entity_List[i]->GetRigidBody();
		if (pRB->IsColliding(m_pRigidBody))
		{
			l_Entity_List[i]->AddDimension(m_iID); //should have index for child subdivision
		}
	}
}
/*CHECK FOR NUMBER OF OBJECTS FOR IF IT SHOUDL SUBDIVIDE WHEN CALLED AGAIN*/
void Simplex::MyOctant::Subdivide(void)
{
	if (m_nlevel > 2)
	{
		return;
	}
	vector3 v3Center = m_pRigidBody->GetCenterLocal();
	vector3 v3HalfWidth = m_pRigidBody->GetHalfWidth();
	float fSize = v3HalfWidth.x / 2; 
	float fCenters = fSize;
	
	m_pChild[0] = new MyOctant(v3Center + vector3(fCenters, fCenters, fCenters), fSize);
	m_pChild[1] = new MyOctant(v3Center + vector3(-fCenters, fCenters, fCenters), fSize);
	m_pChild[2] = new MyOctant(v3Center + vector3(-fCenters, -fCenters, fCenters), fSize);
	m_pChild[3] = new MyOctant(v3Center + vector3(fCenters, -fCenters, fCenters), fSize);

	m_pChild[4] = new MyOctant(v3Center + vector3(fCenters, fCenters, -fCenters), fSize);
	m_pChild[5] = new MyOctant(v3Center + vector3(-fCenters, fCenters, -fCenters), fSize);
	m_pChild[6] = new MyOctant(v3Center + vector3(-fCenters, -fCenters, -fCenters), fSize);
	m_pChild[7] = new MyOctant(v3Center + vector3(fCenters, -fCenters, -fCenters), fSize);
	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i]->m_nlevel = m_nlevel + 1;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->Subdivide();
	}
}


//Accessors
void MyOctant::SetData(int a_nData){ m_nData = a_nData; }
int MyOctant::GetData(void){ return m_nData; }
void MyOctant::SetDataOnVector(int a_nData){ m_lData.push_back(a_nData);}
int& MyOctant::GetDataOnVector(int a_nIndex)
{
	int nIndex = static_cast<int>(m_lData.size());
	assert(a_nIndex >= 0 && a_nIndex < nIndex);
	return m_lData[a_nIndex];
}
//--- Non Standard Singleton Methods


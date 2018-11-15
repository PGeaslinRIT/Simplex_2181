#include "MyOctant.h"
using namespace Simplex;

uint MyOctant::m_uOctantCount; //will store the number of octants instantiated
uint MyOctant::m_uMaxLevel;//will store the maximum level an octant can go to
uint MyOctant::m_uIdealEntityCount; //will tell how many ideal Entities this object will contain

MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uOctantCount = 0;

	//get ID and increase octant count
	m_uID = m_uOctantCount;
	m_uOctantCount++;

	//set itself as root
	m_pRoot = this;

	//set default dimensions
	SetSizePos(vector3(0.0f), 72.0f);

	//get all entities this is colliding with
	for (uint i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
	{
		IsColliding(i);
	}

	//construct tree up to a certain level
	ConstructTree(a_nMaxLevel);
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();

	//get ID and increase octant count
	m_uID = m_uOctantCount;
	m_uOctantCount++;

	//set up position variables
	SetSizePos(a_v3Center, a_fSize);
}

MyOctant::MyOctant(MyOctant const& other)
{
	m_uOctantCount = other.m_uOctantCount;
	m_uMaxLevel = other.m_uMaxLevel;
	m_uIdealEntityCount = other.m_uIdealEntityCount;

	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_uChildren = other.m_uChildren;

	m_fSize = other.m_fSize;

	m_pMeshMngr = other.m_pMeshMngr;
	m_pEntityMngr = other.m_pEntityMngr;

	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pParent = other.m_pParent;

	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}

	m_EntityList = other.m_EntityList;

	m_pRoot = other.m_pRoot;

	m_lChild = other.m_lChild;

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

MyOctant::~MyOctant(void)
{
	Release();
}

void MyOctant::Release(void)
{
	m_pMeshMngr = nullptr;
	m_pEntityMngr = nullptr;
	m_pParent = nullptr;
	m_pRoot = nullptr;

	//kill children and their children
	KillBranches();
}

void MyOctant::Init(void)
{
	m_uID = 0;
	m_uLevel = 0;
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pParent = nullptr;

	m_pRoot = nullptr;
}

void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);

	std::swap(m_pMeshMngr, other.m_pMeshMngr);
	std::swap(m_pEntityMngr, other.m_pEntityMngr);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	std::swap(m_pParent, other.m_pParent);
	std::swap(m_pChild, other.m_pChild);

	std::swap(m_EntityList, other.m_EntityList);

	std::swap(m_pRoot, other.m_pRoot);

	std::swap(m_lChild, other.m_lChild);
}

void MyOctant::SetSizePos(vector3 a_v3Center, float a_fSize)
{
	//set up position variables
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	//find max and min vectors
	m_v3Max = m_v3Center + vector3(m_fSize / 2.0f);
	m_v3Min = m_v3Center - vector3(m_fSize / 2.0f);
}

float MyOctant::GetSize(void)
{
	return m_fSize;
}

vector3 MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

bool MyOctant::IsColliding(uint a_uRBIndex)
{
	//get rigidbody of given entity
	MyRigidBody* tempBody = m_pEntityMngr->GetEntity(a_uRBIndex)->GetRigidBody();

	//collision flag
	bool bColliding = true;

	//check ARBB collision
	if (this->m_v3Max.x < tempBody->GetMinGlobal().x) //this to the right of other
		bColliding = false;
	if (this->m_v3Min.x > tempBody->GetMaxGlobal().x) //this to the left of other
		bColliding = false;

	if (this->m_v3Max.y < tempBody->GetMinGlobal().y) //this below of other
		bColliding = false;
	if (this->m_v3Min.y > tempBody->GetMaxGlobal().y) //this above of other
		bColliding = false;

	if (this->m_v3Max.z < tempBody->GetMinGlobal().z) //this behind of other
		bColliding = false;
	if (this->m_v3Min.z > tempBody->GetMaxGlobal().z) //this in front of other
		bColliding = false;

	//if there is a collision, add entity to list of entities in octant
	if (bColliding)
	{
		m_EntityList.push_back(a_uRBIndex);
	}

	return bColliding;
}

void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	//display this octant if it's the one specified
	if (m_uID == a_nIndex)
	{
		//display this octant and all its descendents
		Display(a_v3Color);
	}
	else if (!IsLeaf())
	{
		//if this is not the correct octant, check it's children if it has any
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->Display(a_nIndex, a_v3Color);
		}
	}
}

void MyOctant::Display(vector3 a_v3Color)
{
	//draw wire mesh around this octant
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_v3Center) *glm::scale((m_v3Max - m_v3Min)), a_v3Color);

	//display children
	if (!IsLeaf())
	{
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->Display(a_v3Color);
		}
	}
}

void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	//display this octant if it has no children and does contain entities
	if (IsLeaf() && m_EntityList.size() > 0)
	{
		Display(a_v3Color);
	}
	else if (!IsLeaf())
	{
		//if this octant isn't a leaf, check its children
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->DisplayLeafs(a_v3Color);
		}
	}
}

void MyOctant::ClearEntityList(void)
{
	//do nothing if the list is empty
	if (m_EntityList.size() == 0)
		return;

	//clears the entity list
	m_EntityList.clear();
}

void MyOctant::Subdivide(void)
{
	//check that this is a leaf before dividing
	if (IsLeaf())
	{
		//set number of children
		m_uChildren = 8;

		//get new centers
		float quarterSize = m_fSize / 4.0f;

		vector3 newCenters[8];
		newCenters[0] = m_v3Center + vector3( quarterSize,  quarterSize,  quarterSize);
		newCenters[1] = m_v3Center + vector3(-quarterSize,  quarterSize,  quarterSize);
		newCenters[2] = m_v3Center + vector3( quarterSize, -quarterSize,  quarterSize);
		newCenters[3] = m_v3Center + vector3( quarterSize,  quarterSize, -quarterSize);
		newCenters[4] = m_v3Center + vector3( quarterSize, -quarterSize, -quarterSize);
		newCenters[5] = m_v3Center + vector3(-quarterSize,  quarterSize, -quarterSize);
		newCenters[6] = m_v3Center + vector3(-quarterSize, -quarterSize,  quarterSize);
		newCenters[7] = m_v3Center + vector3(-quarterSize, -quarterSize, -quarterSize);

		//create eight children
		for (uint i = 0; i < m_uChildren; i++)
		{
			//determine size and center of child
			float childSize = m_fSize / 2.0f;

			//create the child
			m_pChild[i] = new MyOctant(newCenters[i], childSize);

			//set this octant as the child's parent and pass on root octant
			m_pChild[i]->m_pParent = this;
			m_pChild[i]->m_pRoot = m_pRoot;

			//set child's level
			m_pChild[i]->m_uLevel = m_uLevel + 1;

			//check for entities in the parent that collide with the child
			for (uint j = 0; j < m_EntityList.size(); j++)
			{
				m_pChild[i]->IsColliding(m_EntityList[j]);
			}
		}
	}
	else
	{
		//if not a leaf, tell children to subdivide
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->Subdivide();
		}
	}
}

MyOctant* MyOctant::GetChild(uint a_nChild)
{
	//return nothing if there are no children
	if (IsLeaf())
		return nullptr;

	//check for index out of bounds
	if (a_nChild > 8 || a_nChild < 0)
		return nullptr;

	//return the specified child is possible
	return m_pChild[a_nChild];
}

MyOctant* MyOctant::GetParent(void)
{
	//return this octant's parent
	return m_pParent;
}

bool MyOctant::IsLeaf(void)
{
	//is leaf if it has no children
	return (m_uChildren == 0);
}

bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	//return whether or not this octant has more than the specified number of entities
	return (m_EntityList.size() > a_nEntities);
}

void MyOctant::KillBranches(void)
{
	//go through all children and their children to delete them
	if (!IsLeaf())
	{
		for (uint i = 0; i < m_uChildren; i++)
		{
			delete m_pChild[i];
		}
	}
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//set new max level
	m_uMaxLevel = a_nMaxLevel;

	//subdivide the prescribed number of times
	for (uint i = 0; i < m_uMaxLevel; i++)
	{
		//only need to call root's subdivide since subdivide is recursive
		m_pRoot->Subdivide();
	}

	//clear alll entities of dimensions
	m_pEntityMngr->ClearDimensionSetAll();

	m_pRoot->AssignIDtoEntity();
	m_pRoot->ConstructList();
}

void MyOctant::AssignIDtoEntity(void)
{
	//check if this is a leaf
	if (IsLeaf())
	{
		//add this octants id as a dimension to each entity it contains
		for (uint i = 0; i < m_EntityList.size(); i++)
		{
			std::cout << "Entity " << m_EntityList[i] << " is in octant " << m_uID << std::endl;
			m_pEntityMngr->AddDimension(m_EntityList[i], m_uID);
		}
	}
	else
	{
		//if this has children, check the children
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->AssignIDtoEntity();
		}
	}
}

uint MyOctant::GetOctantCount()
{
	return m_uOctantCount;
}

void MyOctant::ConstructList(void)
{
	//check that this is a leaf and that it has entities
	if (IsLeaf() && m_EntityList.size() > 0)
	{
		//if a leaf with entities, add itself to the list of leaves with entities
		m_pRoot->m_lChild.push_back(this);
	}
	else
	{
		//if not a leaf, check children
		for (uint i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->ConstructList();
		}
	}
}


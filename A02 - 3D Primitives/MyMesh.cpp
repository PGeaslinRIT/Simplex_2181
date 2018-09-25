#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//determine the interval between subdivisions
	float intervals = 2.0f * PI / a_nSubdivisions;

	//half height
	float halfHeight = a_fHeight / 2.0f;

	//store a vector pointing to the origin
	vector3 baseCenter = vector3(0.0, 0.0, -1.0f * halfHeight);
	vector3 peak = vector3(0.0, 0.0, halfHeight);

	//add a triangle for each subdivision
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//find the first point of the triangle
		vector3 thisVertex = vector3(a_fRadius * cosf(i * intervals), a_fRadius * sinf(i*intervals), -1.0f * halfHeight);

		//find the third point of the triangle
		vector3 nextVertex = vector3(a_fRadius * cosf((i + 1) * intervals), a_fRadius * sinf((i + 1) *intervals), -1.0f * halfHeight);

		//create a triangle and add its vertices to the vector
		AddTri(baseCenter, thisVertex, nextVertex);

		//add the exterior triangle pointing towards the peak
		AddTri(thisVertex, peak, nextVertex);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//determine the interval between subdivisions
	float intervals = 2.0f * PI / a_nSubdivisions;

	//half height
	float halfHeight = a_fHeight / 2.0f;

	//store a vector pointing to the origin
	vector3 baseBCenter = vector3(0.0, 0.0, -1.0f * halfHeight);
	vector3 baseTCenter = vector3(0.0, 0.0, halfHeight);

	//add a triangle for each subdivision
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//find the first point of the triangle
		vector3 thisBVertex = vector3(a_fRadius * cosf(i * intervals), a_fRadius * sinf(i*intervals), -1.0f * halfHeight);
		vector3 thisTVertex = vector3(a_fRadius * cosf(i * intervals), a_fRadius * sinf(i*intervals), halfHeight);

		//find the second point
		vector3 nextBVertex = vector3(a_fRadius * cosf((i + 1) * intervals), a_fRadius * sinf((i + 1) *intervals), -1.0f * halfHeight);
		vector3 nextTVertex = vector3(a_fRadius * cosf((i + 1) * intervals), a_fRadius * sinf((i + 1) *intervals), halfHeight);

		//bottom base
		AddTri(thisBVertex, baseBCenter, nextBVertex);

		//top base
		AddTri(baseTCenter, thisTVertex, nextTVertex);

		//walls
		AddQuad(thisBVertex, nextBVertex, thisTVertex, nextTVertex);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color) 
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	//determine the interval between subdivisions
	float intervals = 2.0f * PI / a_nSubdivisions;

	//half height
	float halfHeight = a_fHeight / 2.0f;

	//store a vector pointing to the origin
	vector3 baseBCenter = vector3(0.0, 0.0, -1.0f * halfHeight);
	vector3 baseTCenter = vector3(0.0, 0.0, halfHeight);

	//add a triangle for each subdivision
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//find the first point of the triangle
		vector3 thisOuterBVertex = vector3(a_fOuterRadius * cosf(i * intervals), a_fOuterRadius * sinf(i*intervals), -1.0f * halfHeight);
		vector3 thisOuterTVertex = vector3(a_fOuterRadius * cosf(i * intervals), a_fOuterRadius * sinf(i*intervals), halfHeight);
		vector3 thisInnerBVertex = vector3(a_fInnerRadius * cosf(i * intervals), a_fInnerRadius * sinf(i*intervals), -1.0f * halfHeight);
		vector3 thisInnerTVertex = vector3(a_fInnerRadius * cosf(i * intervals), a_fInnerRadius * sinf(i*intervals), halfHeight);

		//find the third point of the triangle, wrapping to the beginning of the subdivisions if necessary
		vector3 nextOuterBVertex;
		vector3 nextOuterTVertex;
		vector3 nextInnerBVertex;
		vector3 nextInnerTVertex;
		if (i + 1 == a_nSubdivisions)
		{
			nextOuterBVertex = vector3(a_fOuterRadius * cosf(0.0f), a_fOuterRadius * sinf(0.0f), -1.0f * halfHeight);
			nextOuterTVertex = vector3(a_fOuterRadius * cosf(0.0f), a_fOuterRadius * sinf(0.0f), halfHeight);
			nextInnerBVertex = vector3(a_fInnerRadius * cosf(0.0f), a_fInnerRadius * sinf(0.0f), -1.0f * halfHeight);
			nextInnerTVertex = vector3(a_fInnerRadius * cosf(0.0f), a_fInnerRadius * sinf(0.0f), halfHeight);
		}
		else
		{
			nextOuterBVertex = vector3(a_fOuterRadius * cosf((i + 1) * intervals), a_fOuterRadius * sinf((i + 1) * intervals), -1.0f * halfHeight);
			nextOuterTVertex = vector3(a_fOuterRadius * cosf((i + 1) * intervals), a_fOuterRadius * sinf((i + 1) * intervals), halfHeight);
			nextInnerBVertex = vector3(a_fInnerRadius * cosf((i + 1) * intervals), a_fInnerRadius * sinf((i + 1) * intervals), -1.0f * halfHeight);
			nextInnerTVertex = vector3(a_fInnerRadius * cosf((i + 1) * intervals), a_fInnerRadius * sinf((i + 1) * intervals), halfHeight);
		}

		//bottom base
		AddQuad(thisOuterBVertex, thisInnerBVertex, nextOuterBVertex, nextInnerBVertex);

		//top base
		AddQuad(nextOuterTVertex, nextInnerTVertex, thisOuterTVertex, thisInnerTVertex);

		//inner wall
		AddQuad(nextInnerTVertex, nextInnerBVertex, thisInnerTVertex, thisInnerBVertex);

		//outer wall
		AddQuad(thisOuterBVertex, nextOuterBVertex, thisOuterTVertex, nextOuterTVertex);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 12)
		a_nSubdivisions = 12;

	Release();
	Init();

	//determine the interval between subdivisions
	float interval = 2.0f * PI / a_nSubdivisions;
	float halfInt = interval / 2.0f;

	//store a vector pointing to the origin
	vector3 bottomPoint = vector3(0.0, 0.0, -1.0f * a_fRadius);
	vector3 topPoint = vector3(0.0, 0.0, a_fRadius);

	//add a triangle for each subdivision
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		//find the first point on the caps
		vector3 thisTVertex = a_fRadius * vector3(
			cosf(i * interval) * sinf(halfInt),
			sinf(i * interval) * sinf(halfInt),
			cos(halfInt));
		vector3 thisBVertex = a_fRadius * vector3(
			cosf(i * interval) * sinf(halfInt),
			sinf(i * interval) * sinf(halfInt),
			-1.0f * cos(halfInt));

		//find the second point on the caps
		vector3 nextTVertex = a_fRadius * vector3(
			cosf((i + 1) * interval) * sinf(halfInt),
			sinf((i + 1) * interval) * sinf(halfInt),
			cos(halfInt));
		vector3 nextBVertex = a_fRadius * vector3(
			cosf((i + 1) * interval) * sinf(halfInt),
			sinf((i + 1) * interval) * sinf(halfInt),
			-1.0f * cos(halfInt));

		//create the walls for the quad faces, moving along the subdivision from top to bottom
		for (int j = 1; j < a_nSubdivisions - 1; j++)
		{
			//find the first pair of points
			vector3 thisTFaceVertex = a_fRadius * vector3(
				cosf(i * interval) * sinf(j * halfInt),
				sinf(i * interval) * sinf(j * halfInt),
				cos(j * halfInt));
			vector3 thisBFaceVertex = a_fRadius * vector3(
				cosf(i * interval) * sinf((j + 1) * halfInt),
				sinf(i * interval) * sinf((j + 1) * halfInt),
				cos((j + 1) * halfInt));

			//find the second pair of points
			vector3 nextTFaceVertex = a_fRadius * vector3(
				cosf((i + 1) * interval) * sinf(j * halfInt),
				sinf((i + 1) * interval) * sinf(j * halfInt),
				cos(j * halfInt));
			vector3 nextBFaceVertex = a_fRadius * vector3(
				cosf((i + 1) * interval) * sinf((j + 1) * halfInt),
				sinf((i + 1) * interval) * sinf((j + 1) * halfInt),
				cos((j + 1) * halfInt));

			//add quad faces
			AddQuad(nextBFaceVertex, nextTFaceVertex, thisBFaceVertex, thisTFaceVertex);
		}

		//top ring
		AddTri(thisTVertex, nextTVertex, topPoint);

		//bottom ring
		AddTri(bottomPoint, nextBVertex, thisBVertex);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
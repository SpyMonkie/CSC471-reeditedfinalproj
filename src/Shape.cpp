
#include "Shape.h"
#include <iostream>
#include <cassert>

#include "GLSL.h"
#include "Program.h"

using namespace std;

Shape::Shape(bool textured) :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0),
	vaoID(0),
	texOff(!textured)
{
	min = glm::vec3(0);
	max = glm::vec3(0);
}

Shape::~Shape()
{
}

// copy the data from the shape to this object
void Shape::createShape(tinyobj::shape_t & shape)
{
	posBuf = shape.mesh.positions;
	norBuf = shape.mesh.normals;
	texBuf = shape.mesh.texcoords;
	eleBuf = shape.mesh.indices;
}

void Shape::measure()
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = std::numeric_limits<float>::max();
	maxX = maxY = maxZ = -std::numeric_limits<float>::max();

	//Go through all vertices to determine min and max of each dimension
	for (size_t v = 0; v < posBuf.size() / 3; v++)
	{
		if (posBuf[3*v+0] < minX) minX = posBuf[3 * v + 0];
		if (posBuf[3*v+0] > maxX) maxX = posBuf[3 * v + 0];

		if (posBuf[3*v+1] < minY) minY = posBuf[3 * v + 1];
		if (posBuf[3*v+1] > maxY) maxY = posBuf[3 * v + 1];

		if (posBuf[3*v+2] < minZ) minZ = posBuf[3 * v + 2];
		if (posBuf[3*v+2] > maxZ) maxZ = posBuf[3 * v + 2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
	max.x = maxX;
	max.y = maxY;
	max.z = maxZ;
}

void Shape::init()
{
	// Initialize the vertex array object
	CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Send the position array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));

	// Send the normal array to the GPU
	if (norBuf.empty())
	{
		// norBufID = 0;
		// norBufID = 0;
				// if no normals - compute normals for program 3
		// computeNormals(); -- on CPU with vertices and faces you have
		// then send to GPU
		// in norBuffer - initalize to {0, 0, 0} for each vertex
		// for every face (use eleBuf) compute the normal and add to the normal of each vertex in that face : compute two edges of the face and take the cross product - glm::cross(e1, e2)
		// add face normal to each adjacent vertices in the normal buffer
		// norBufID = 0;

		norBuf.resize(posBuf.size(), 0.0f); // initialize normals to zero

		for (size_t i = 0; i < eleBuf.size(); i += 3) {

			// Get the indices of the vertices of the face from element buffer (eleBuf)
			unsigned int idx0 = eleBuf[i];
			unsigned int idx1 = eleBuf[i + 1];
			unsigned int idx2 = eleBuf[i + 2];

			// Compute the edges of the triangle
			// get each position of the vertices from the position buffer (posBuf)
			glm::vec3 v0 = glm::vec3(posBuf[3 * idx0], posBuf[3 * idx0 + 1], posBuf[3 * idx0 + 2]); // multiply by 3 because each vertex has 3 floats
			glm::vec3 v1 = glm::vec3(posBuf[3 * idx1], posBuf[3 * idx1 + 1], posBuf[3 * idx1 + 2]); // add 0 1 2 to get the x y z of the vertex respectively
			glm::vec3 v2 = glm::vec3(posBuf[3 * idx2], posBuf[3 * idx2 + 1], posBuf[3 * idx2 + 2]);

			// subtract vertex 0 from vertex 1 and vertex 2 to get the edges
			// cross product of the edges gives the normal
			glm::vec3 e1 = v1 - v0;
			glm::vec3 e2 = v2 - v0;

			// compute the normal
			glm::vec3 faceNormal = glm::normalize(glm::cross(e1, e2));

			// add the face normal to the normals of the vertices
			// in the normal buffer (norBuf)
			// multiply by 3 again because each vertex has 3 floats
			// and add 0 1 2 to get the x y z of the vertex respectively
			norBuf[3 * idx0] += faceNormal.x;
			norBuf[3 * idx0 + 1] += faceNormal.y;
			norBuf[3 * idx0 + 2] += faceNormal.z;
			norBuf[3 * idx1] += faceNormal.x;
			norBuf[3 * idx1 + 1] += faceNormal.y;
			norBuf[3 * idx1 + 2] += faceNormal.z;
			norBuf[3 * idx2] += faceNormal.x;
			norBuf[3 * idx2 + 1] += faceNormal.y;
			norBuf[3 * idx2 + 2] += faceNormal.z;
		}
		// Normalize the vertex normals
		for (size_t i = 0; i < norBuf.size(); i += 3) {
			glm::vec3 normal(norBuf[i], norBuf[i + 1], norBuf[i + 2]); // get the x y z of the normal
			normal = glm::normalize(normal); // normalize the normal

			// set the normal back to the normal buffer
			norBuf[i] = normal.x;
			norBuf[i + 1] = normal.y;
			norBuf[i + 2] = normal.z;
		}

		// Send the normal array to the GPU
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW));
	}

	// Send the texture array to the GPU
	if (texBuf.empty() || texOff)
	{
		texBufID = 0;
	}
	else
	{
		CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
	}

	// Send the element array to the GPU
	CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
	CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));

	// Unbind the arrays
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

	CHECKED_GL_CALL(glBindVertexArray(vaoID));

	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
	CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if (h_nor != -1 && norBufID != 0)
	{
		GLSL::enableVertexAttribArray(h_nor);
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
		CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
	}

	if (texBufID != 0 && !texOff)
	{
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");

		if (h_tex != -1 && texBufID != 0)
		{
			GLSL::enableVertexAttribArray(h_tex);
			CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
			CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
		}
	}

	// Bind element buffer
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

	// Draw
	CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

	// Disable and unbind
	if (h_tex != -1)
	{
		GLSL::disableVertexAttribArray(h_tex);
	}
	if (h_nor != -1)
	{
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

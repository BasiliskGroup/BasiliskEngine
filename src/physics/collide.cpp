
// Copied and modified from box2D-lite: https://github.com/erincatto/box2d-lite

/*
MIT License

Copyright (c) 2019 Erin Catto

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <basilisk/physics/solver.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/physics/maths.h>

// Box vertex and edge numbering:
//
//        ^ y
//        |
//        e1
//   v2 ------ v1
//    |        |
// e2 |        | e4  --> x
//    |        |
//   v3 ------ v4
//        e3

namespace bsk::internal {

enum Axis
{
	FACE_A_X,
	FACE_A_Y,
	FACE_B_X,
	FACE_B_Y
};

enum EdgeNumbers
{
	NO_EDGE = 0,
	EDGE1,
	EDGE2,
	EDGE3,
	EDGE4
};

struct ClipVertex
{
	ClipVertex() { fp.value = 0; }
	glm::vec2 v;
	Manifold::FeaturePair fp;
};

static void Flip(Manifold::FeaturePair& fp)
{
	char temp = fp.e.inEdge1;
	fp.e.inEdge1 = fp.e.inEdge2;
	fp.e.inEdge2 = temp;

	temp = fp.e.outEdge1;
	fp.e.outEdge1 = fp.e.outEdge2;
	fp.e.outEdge2 = temp;
}

static int ClipSegmentToLine(ClipVertex vOut[2], ClipVertex vIn[2],
	const glm::vec2& normal, float offset, char clipEdge)
{
	// Start with no output points
	int numOut = 0;

	// Calculate the distance of end points to the line
	float distance0 = dot(normal, vIn[0].v) - offset;
	float distance1 = dot(normal, vIn[1].v) - offset;

	// If the points are behind the plane
	if (distance0 <= 0.0f) vOut[numOut++] = vIn[0];
	if (distance1 <= 0.0f) vOut[numOut++] = vIn[1];

	// If the points are on different sides of the plane
	if (distance0 * distance1 < 0.0f)
	{
		// Find intersection point of edge and plane
		float interp = distance0 / (distance0 - distance1);
		vOut[numOut].v = vIn[0].v + (vIn[1].v - vIn[0].v) * interp;
		if (distance0 > 0.0f)
		{
			vOut[numOut].fp = vIn[0].fp;
			vOut[numOut].fp.e.inEdge1 = clipEdge;
			vOut[numOut].fp.e.inEdge2 = NO_EDGE;
		}
		else
		{
			vOut[numOut].fp = vIn[1].fp;
			vOut[numOut].fp.e.outEdge1 = clipEdge;
			vOut[numOut].fp.e.outEdge2 = NO_EDGE;
		}
		++numOut;
	}

	return numOut;
}

static void ComputeIncidentEdge(ClipVertex c[2], const glm::vec2& h, const glm::vec2& pos,
	const glm::mat2& Rot, const glm::vec2& normal)
{
	// The normal is from the reference box. Convert it
	// to the incident boxe's frame and flip sign.
	glm::mat2 RotT = transpose(Rot);
	glm::vec2 n = -(RotT * normal);
	glm::vec2 nAbs = abs(n);

	if (nAbs.x > nAbs.y)
	{
		if (sign(n.x) > 0.0f)
		{
			c[0].v = glm::vec2{ h.x, -h.y };
			c[0].fp.e.inEdge2 = EDGE3;
			c[0].fp.e.outEdge2 = EDGE4;

			c[1].v = glm::vec2{ h.x, h.y };
			c[1].fp.e.inEdge2 = EDGE4;
			c[1].fp.e.outEdge2 = EDGE1;
		}
		else
		{
			c[0].v = glm::vec2{ -h.x, h.y };
			c[0].fp.e.inEdge2 = EDGE1;
			c[0].fp.e.outEdge2 = EDGE2;

			c[1].v = glm::vec2{ -h.x, -h.y };
			c[1].fp.e.inEdge2 = EDGE2;
			c[1].fp.e.outEdge2 = EDGE3;
		}
	}
	else
	{
		if (sign(n.y) > 0.0f)
		{
			c[0].v = glm::vec2{ h.x, h.y };
			c[0].fp.e.inEdge2 = EDGE4;
			c[0].fp.e.outEdge2 = EDGE1;

			c[1].v = glm::vec2{ -h.x, h.y };
			c[1].fp.e.inEdge2 = EDGE1;
			c[1].fp.e.outEdge2 = EDGE2;
		}
		else
		{
			c[0].v = glm::vec2{ -h.x, -h.y };
			c[0].fp.e.inEdge2 = EDGE2;
			c[0].fp.e.outEdge2 = EDGE3;

			c[1].v = glm::vec2{ h.x, -h.y };
			c[1].fp.e.inEdge2 = EDGE3;
			c[1].fp.e.outEdge2 = EDGE4;
		}
	}

	c[0].v = pos + Rot * c[0].v;
	c[1].v = pos + Rot * c[1].v;
}

// The normal points from A to B
int Manifold::collide(Rigid* bodyA, Rigid* bodyB, Contact* contacts)
{
	glm::vec2 normal;

	// Setup
	glm::vec2 hA = bodyA->getSize() * 0.5f;
	glm::vec2 hB = bodyB->getSize() * 0.5f;

	glm::vec2 posA = glm::vec2(bodyA->getPosition().x, bodyA->getPosition().y);
	glm::vec2 posB = glm::vec2(bodyB->getPosition().x, bodyB->getPosition().y);

	glm::mat2 RotA = rotation(bodyA->getPosition().z), RotB = rotation(bodyB->getPosition().z);

	glm::mat2 RotAT = transpose(RotA);
	glm::mat2 RotBT = transpose(RotB);

	glm::vec2 dp = posB - posA;
	glm::vec2 dA = RotAT * dp;
	glm::vec2 dB = RotBT * dp;

	glm::mat2 C = RotAT * RotB;
	glm::mat2 absC = abs(C);
	glm::mat2 absCT = transpose(absC);

	// Box A faces
	glm::vec2 faceA = abs(dA) - hA - absC * hB;
	if (faceA.x > 0.0f || faceA.y > 0.0f)
		return 0;

	// Box B faces
	glm::vec2 faceB = abs(dB) - absCT * hA - hB;
	if (faceB.x > 0.0f || faceB.y > 0.0f)
		return 0;

	// Find best axis
	Axis axis;
	float separation;

	// Box A faces
	axis = FACE_A_X;
	separation = faceA.x;
	if (dA.x > 0.0f) normal = RotA[0];  // GLM: mat[i] gives column i
	else normal = -RotA[0];

	const float relativeTol = 0.95f;
	const float absoluteTol = 0.01f;

	if (faceA.y > relativeTol * separation + absoluteTol * hA.y)
	{
		axis = FACE_A_Y;
		separation = faceA.y;
		if (dA.y > 0.0f) normal = RotA[1];
		else normal = -RotA[1];
	}

	// Box B faces
	if (faceB.x > relativeTol * separation + absoluteTol * hB.x)
	{
		axis = FACE_B_X;
		separation = faceB.x;
		if (dB.x > 0.0f) normal = RotB[0];
		else normal = -RotB[0];
	}

	if (faceB.y > relativeTol * separation + absoluteTol * hB.y)
	{
		axis = FACE_B_Y;
		separation = faceB.y;
		if (dB.y > 0.0f) normal = RotB[1];
		else normal = -RotB[1];
	}

	// Setup clipping plane data based on the separating axis
	glm::vec2 frontNormal, sideNormal;
	ClipVertex incidentEdge[2];
	float front, negSide, posSide;
	char negEdge, posEdge;

	// Compute the clipping lines and the line segment to be clipped.
	switch (axis)
	{
	case FACE_A_X:
	{
		frontNormal = normal;
		front = dot(posA, frontNormal) + hA.x;
		sideNormal = RotA[1];
		float side = dot(posA, sideNormal);
		negSide = -side + hA.y;
		posSide = side + hA.y;
		negEdge = EDGE3;
		posEdge = EDGE1;
		ComputeIncidentEdge(incidentEdge, hB, posB, RotB, frontNormal);
	}
	break;

	case FACE_A_Y:
	{
		frontNormal = normal;
		front = dot(posA, frontNormal) + hA.y;
		sideNormal = RotA[0];
		float side = dot(posA, sideNormal);
		negSide = -side + hA.x;
		posSide = side + hA.x;
		negEdge = EDGE2;
		posEdge = EDGE4;
		ComputeIncidentEdge(incidentEdge, hB, posB, RotB, frontNormal);
	}
	break;

	case FACE_B_X:
	{
		frontNormal = -normal;
		front = dot(posB, frontNormal) + hB.x;
		sideNormal = RotB[1];
		float side = dot(posB, sideNormal);
		negSide = -side + hB.y;
		posSide = side + hB.y;
		negEdge = EDGE3;
		posEdge = EDGE1;
		ComputeIncidentEdge(incidentEdge, hA, posA, RotA, frontNormal);
	}
	break;

	case FACE_B_Y:
	{
		frontNormal = -normal;
		front = dot(posB, frontNormal) + hB.y;
		sideNormal = RotB[0];
		float side = dot(posB, sideNormal);
		negSide = -side + hB.x;
		posSide = side + hB.x;
		negEdge = EDGE2;
		posEdge = EDGE4;
		ComputeIncidentEdge(incidentEdge, hA, posA, RotA, frontNormal);
	}
	break;
	}

	// clip other face with 5 box planes (1 face plane, 4 edge planes)

	ClipVertex clipPoints1[2];
	ClipVertex clipPoints2[2];
	int np;

	// Clip to box side 1
	np = ClipSegmentToLine(clipPoints1, incidentEdge, -sideNormal, negSide, negEdge);

	if (np < 2)
		return 0;

	// Clip to negative box side 1
	np = ClipSegmentToLine(clipPoints2, clipPoints1, sideNormal, posSide, posEdge);

	if (np < 2)
		return 0;

	// Now clipPoints2 contains the clipping points.
	// Due to roundoff, it is possible that clipping removes all points.

	int numContacts = 0;
	for (int i = 0; i < 2; ++i)
	{
		float separation = dot(frontNormal, clipPoints2[i].v) - front;

		if (separation <= 0)
		{
			contacts[numContacts].normal = -normal;

			// slide contact point onto reference face (easy to cull)
			contacts[numContacts].rA = transpose(RotA) * (clipPoints2[i].v - frontNormal * separation - posA);
			contacts[numContacts].rB = transpose(RotB) * (clipPoints2[i].v - posB);
			contacts[numContacts].feature = clipPoints2[i].fp;

			if (axis == FACE_B_X || axis == FACE_B_Y)
			{
				Flip(contacts[numContacts].feature);
				contacts[numContacts].rA = transpose(RotA) * (clipPoints2[i].v - posA);
				contacts[numContacts].rB = transpose(RotB) * (clipPoints2[i].v - frontNormal * separation - posB);
			}
			++numContacts;
		}
	}

	return numContacts;
}

}
#include <basilisk/physics/solver.h>
#include <basilisk/physics/forces/manifold.h>
#include <basilisk/physics/rigid.h>
#include <basilisk/util/maths.h>
#include <basilisk/physics/collision/collider.h>
#include <basilisk/physics/collision/gjk.h>

namespace bsk::internal {

void getTransformedVertices(Rigid* body, std::vector<glm::vec2>& vertices) {
	vertices = body->getCollider()->getVertices();
	for (size_t i = 0; i < vertices.size(); ++i) vertices[i] = transform(body->getPosition(), body->getSize() * vertices[i]);
}

int Manifold::collide(
	const std::vector<glm::vec2>& verticesA,
	const std::vector<glm::vec2>& verticesB,
	Contact* contacts) {
	if (verticesA.empty() || verticesB.empty()) {
		return 0;
	}

	// construct structs
	glm::vec2 centroidA(0.0f);
	glm::vec2 centroidB(0.0f);
	for (const glm::vec2& v : verticesA) centroidA += v;
	for (const glm::vec2& v : verticesB) centroidB += v;
	centroidA /= static_cast<float>(verticesA.size());
	centroidB /= static_cast<float>(verticesB.size());
	ConvexShape shapeA(verticesA, centroidA, 0.0f, glm::vec2(1.0f));
	ConvexShape shapeB(verticesB, centroidB, 0.0f, glm::vec2(1.0f));
	CollisionPair pair;

	bool gjkOk = gjk(shapeA, shapeB, pair);
	
	if (!gjkOk) {
		return 0;
	}

	// NOTE for now run SAT, later run EPA
	sat(verticesA, verticesB, pair);

	pair.normal = -pair.normal;

	findContactPoints(verticesA, verticesB, pair);

	pair.normal = -pair.normal;

	// transform contact points to relative space
	const int numContacts = glm::max(pair.numA, pair.numB);

	for (int i = 0; i < numContacts; ++i) {
		// Contact candidates are in world space from SAT clipping.
		const glm::vec2 aWorld = (i > 0) ? pair.a2 : pair.a1;
		const glm::vec2 bWorld = (i > 0) ? pair.b2 : pair.b1;

		// This base collision path is world-space only.
		contacts[i].rA = aWorld;
		contacts[i].rB = bWorld;
		contacts[i].normal = pair.normal;
	}

	return numContacts;
}

// The normal points from A to B
int Manifold::collide(Rigid* bodyA, Rigid* bodyB, Contact* contacts) {
	std::vector<glm::vec2> verticesA;
	std::vector<glm::vec2> verticesB;
	getTransformedVertices(bodyA, verticesA);
	getTransformedVertices(bodyB, verticesB);

	const int numContacts = collide(verticesA, verticesB, contacts);
	const glm::vec2 posA = glm::vec2(bodyA->getPosition());
	const glm::vec2 posB = glm::vec2(bodyB->getPosition());
	const float rotA = bodyA->getPosition().z;
	const float rotB = bodyB->getPosition().z;
	for (int i = 0; i < numContacts; ++i) {
		contacts[i].rA = rotate(-rotA, contacts[i].rA - posA);
		contacts[i].rB = rotate(-rotB, contacts[i].rB - posB);
	}
	return numContacts;
}

int Manifold::collide(Rigid* bodyA, const std::vector<glm::vec2>& worldVerticesB, Contact* contacts) {
	std::vector<glm::vec2> verticesA;
	getTransformedVertices(bodyA, verticesA);

	const int numContacts = collide(verticesA, worldVerticesB, contacts);
	const glm::vec2 posA = glm::vec2(bodyA->getPosition());
	const float rotA = bodyA->getPosition().z;
	for (int i = 0; i < numContacts; ++i) {
		contacts[i].rA = rotate(-rotA, contacts[i].rA - posA);
		// For static world manifolds, keep B contact in world space.
	}
	return numContacts;
}

}
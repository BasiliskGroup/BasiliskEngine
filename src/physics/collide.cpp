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

// The normal points from A to B
int Manifold::collide(Rigid* bodyA, Rigid* bodyB, Contact* contacts) {
	// transform vertices into world space
	// NOTE don't do this, use model space
	std::vector<glm::vec2> verticesA;
	std::vector<glm::vec2> verticesB;

	getTransformedVertices(bodyA, verticesA);
	getTransformedVertices(bodyB, verticesB);

	// construct structs
	ConvexShape shapeA(verticesA, bodyA->getPosition(), bodyA->getPosition().z, bodyA->getSize());
	ConvexShape shapeB(verticesB, bodyB->getPosition(), bodyB->getPosition().z, bodyB->getSize());
	CollisionPair pair;

	bool gjkOk = gjk(shapeA, shapeB, pair);

	// std::cout << "gjkOk: " << gjkOk << std::endl;
	
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
	const glm::vec2 posA = glm::vec2(bodyA->getPosition());
	const glm::vec2 posB = glm::vec2(bodyB->getPosition());
	const float rotA = bodyA->getPosition().z;
	const float rotB = bodyB->getPosition().z;
	const glm::vec2 sizeA = bodyA->getSize();
	const glm::vec2 sizeB = bodyB->getSize();

	for (int i = 0; i < numContacts; ++i) {
		// Contact candidates are in world space from SAT clipping.
		const glm::vec2 aWorld = (i > 0) ? pair.a2 : pair.a1;
		const glm::vec2 bWorld = (i > 0) ? pair.b2 : pair.b1;

		// Contacts store local offsets so manifold Jacobians can rotate them each frame.
		contacts[i].rA = rotate(-rotA, aWorld - posA);
		contacts[i].rB = rotate(-rotB, bWorld - posB);
		contacts[i].normal = pair.normal;
	}

	return numContacts;
}

}
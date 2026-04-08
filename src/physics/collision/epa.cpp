#include <basilisk/physics/collision/gjk.h>

namespace bsk::internal {

bool epaInternal(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair) {
    glm::vec2 ab = pair.sps[1] - pair.sps[0];
    glm::vec2 ac = pair.sps[2] - pair.sps[0];
    float cross = ab.x * ac.y - ab.y * ac.x;
    if (cross < 0) {
        std::swap(pair.sps[1], pair.sps[2]);
    }

    pair.interior = pair.sps[0] + pair.sps[1] + pair.sps[2];
    const float initialCount = 3.0f;
    glm::vec2 centroid = pair.interior / initialCount;

    buildFace(pair, centroid, 0, 1, 0);
    buildFace(pair, centroid, 1, 2, 1);
    buildFace(pair, centroid, 2, 0, 2);

    ushort cloudSize = 3;
    ushort numFaces = 3;
    ushort setSize = 0;

    ushort frontIndex;
    float frontDistance;
    for (ushort _ = 0; _ < EPA_ITERATIONS; _++) {
        // quick access front data
        frontIndex = polytopeFront(pair.polytope, numFaces);
        frontDistance = pair.polytope[frontIndex].distance;

        pair.searchDir = pair.polytope[frontIndex].normal;
        addSupport(shapeA, shapeB, pair, cloudSize);

        // check if newly added point is not in the cloud
        // if so, we have found the edge and can stop
        for (ushort i = 0; i < cloudSize; i++) {
            if (glm::length2(pair.sps[cloudSize] - pair.sps[i]) < COLLISION_MARGIN * COLLISION_MARGIN) {
                pair.normal = pair.polytope[frontIndex].normal;
                return true;
            }
        }

        // check that the new found point is past the face
        // this is to ensure that the new point has expanded the polytope
        if (glm::dot(pair.polytope[frontIndex].normal, pair.sps[cloudSize]) - frontDistance < COLLISION_MARGIN) {
            pair.normal = pair.polytope[frontIndex].normal;
            return true;
        }

        // collect horizon edges
        setSize = 0;
        ushort i = 0;
        while (i < numFaces) {
            if (glm::dot(pair.polytope[i].normal, pair.sps[cloudSize]) > COLLISION_MARGIN * COLLISION_MARGIN) {
                setSize = insertHorizon(pair.supportSet, pair.polytope[i].va, setSize);
                setSize = insertHorizon(pair.supportSet, pair.polytope[i].vb, setSize);
                removeFace(pair.polytope, i, numFaces);
                numFaces -= 1;
            } else {
                i++;
            }
        }

        if (setSize != 2) {
            std::cerr << "Polytope horizon error" << std::endl;
            ushort failFront = polytopeFront(pair.polytope, numFaces);
            pair.normal = pair.polytope[failFront].normal;
            return false;
        }

        // there should be only 2 horizon vertices left, create new face
        pair.interior += pair.sps[cloudSize];
        glm::vec2 centroid = pair.interior / static_cast<float>(cloudSize + 1);

        // ensure supportSet[0] -> cloudSize -> supportSet[1] winds CCW
        glm::vec2 s0 = pair.sps[pair.supportSet[0]];
        glm::vec2 s1 = pair.sps[pair.supportSet[1]];
        glm::vec2 np = pair.sps[cloudSize];

        // cross product of (s0->np) x (s0->s1): if negative, s1 should come first
        float windCheck = (np.x - s0.x) * (s1.y - s0.y) - (np.y - s0.y) * (s1.x - s0.x);
        if (windCheck > 0) {
            std::swap(pair.supportSet[0], pair.supportSet[1]);
        }

        buildFace(pair, centroid, pair.supportSet[0], cloudSize, numFaces);
        buildFace(pair, centroid, cloudSize, pair.supportSet[1], numFaces + 1);        
        numFaces += 2;

        // we increment cloud size at the end to reduce subtractions in the middle of the loop
        cloudSize++;
    }

    // we timed out
    std::cout << "EPA timed out" << std::endl;
    ushort timeoutFront = polytopeFront(pair.polytope, numFaces);
    pair.normal = pair.polytope[timeoutFront].normal;
    return true;
}

bool epa(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair) {
    bool success = epaInternal(shapeA, shapeB, pair);
    glm::vec2 dir = shapeA.pos - shapeB.pos;
    if (glm::dot(pair.normal, dir) > 0) {
        pair.normal = -pair.normal;
    }
    return success;
}

ushort insertHorizon(SupportSet& supportSet, ushort spIndex, ushort setSize) {
    if (discardHorizon(supportSet, spIndex, setSize)) {
        return --setSize;
    }

    supportSet[setSize] = spIndex;
    setSize++;

    return setSize;
}

bool discardHorizon(SupportSet& supportSet, ushort spIndex, ushort setSize) {
    if (setSize == 0) {
        return false;
    }

    // uses setSize - 1 since swapping last index wouldn't move it
    for (ushort i = 0; i < setSize - 1; i++) {
        if (supportSet[i] == spIndex) {
            std::swap(supportSet[i], supportSet[setSize - 1]);
            break;
        }
    }

    // no need to swap last but we still need to check if it should be removed
    return supportSet[setSize - 1] == spIndex;
}

// returns the index of the face with the smallest distance
ushort polytopeFront(const Polytope& polytope, ushort numFaces) {
    ushort closeIndex = 0;
    float closeValue = polytope[0].distance;
    float value;

    for (ushort i = 1; i < numFaces; i++) {
        value = polytope[i].distance;
        if (value < closeValue) {
            closeValue = value;
            closeIndex = i;
        }
    }

    return closeIndex;
}

void removeFace(Polytope& polytope, ushort index, ushort numFaces) {
    polytope[index] = polytope[numFaces - 1];
}

void buildFace(CollisionPair& pair, glm::vec2 interior, ushort indexA, ushort indexB, ushort indexL) {
    PolytopeFace& face = pair.polytope[indexL];
    face.va = indexA;
    face.vb = indexB;

    glm::vec2 edge = pair.sps[indexB] - pair.sps[indexA];
    glm::vec2 normal = { edge.y, -edge.x };

    float dotCheck = glm::dot(interior - pair.sps[indexA], normal);
    if (dotCheck > 0) {
        normal = -normal;
    }

    face.normal = glm::normalize(normal);
    face.distance = glm::dot(face.normal, pair.sps[indexA]);
}

}

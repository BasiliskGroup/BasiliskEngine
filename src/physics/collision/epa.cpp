#include <basilisk/physics/collision/gjk.h>

namespace bsk::internal {

bool epa(const ConvexShape& shapeA, const ConvexShape& shapeB, CollisionPair& pair) {
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

        // ADD THIS
        std::cout << "Iter " << _ << " frontIndex=" << frontIndex
        << " frontDist=" << frontDistance
        << " normal=(" << pair.polytope[frontIndex].normal.x
        << ", " << pair.polytope[frontIndex].normal.y << ")" << std::endl;

        pair.searchDir = pair.polytope[frontIndex].normal;
        addSupport(shapeA, shapeB, pair, cloudSize);

        // ADD THIS
        std::cout << "  new sps[" << cloudSize << "]=("
            << pair.sps[cloudSize].x << ", " << pair.sps[cloudSize].y << ")"
            << " dot-frontDist="
            << glm::dot(pair.polytope[frontIndex].normal, pair.sps[cloudSize]) - frontDistance
            << std::endl;

        // check if newly added point is not in the cloud
        // if so, we have found the edge and can stop
        for (ushort i = 0; i < cloudSize; i++) {
            if (glm::length2(pair.sps[cloudSize] - pair.sps[i]) < COLLISION_MARGIN * COLLISION_MARGIN) {
                pair.normal = pair.polytope[frontIndex].normal;
                std::cout << "EPA found edge" << std::endl;
                return true;
            }
        }

        // check that the new found point is past the face
        // this is to ensure that the new point has expanded the polytope
        if (glm::dot(pair.polytope[frontIndex].normal, pair.sps[cloudSize]) - frontDistance < COLLISION_MARGIN) {
            pair.normal = pair.polytope[frontIndex].normal;
            std::cout << "EPA found edge" << std::endl;
            return true;
        }

        // collect horizon edges
        setSize = 0;
        ushort i = 0;
        // print("NUM FACES");
        // print(numFaces);
        while (i < numFaces) {
            if (glm::dot(pair.polytope[i].normal, pair.sps[cloudSize]) > COLLISION_MARGIN * COLLISION_MARGIN) {
                // print("adding face");
                // print(pair.polytope[i].va);
                // print(pair.polytope[i].vb);

                setSize = insertHorizon(pair.supportSet, pair.polytope[i].va, setSize);
                setSize = insertHorizon(pair.supportSet, pair.polytope[i].vb, setSize);
                removeFace(pair.polytope, i, numFaces);
                numFaces -= 1;

                // print("SpSet after add face");
                // for (ushort i = 0; i < setSize; i++) {
                //     print(pair.spSet[i]);
                // }
            } else {
                i++;
            }
        }

        // print("SpSet");
        // for (ushort i = 0; i < setSize; i++) {
        //     print(pair.spSet[i]);
        // }

        if (setSize != 2) {

            std::cout << "Horizon error: setSize=" << setSize << std::endl;
            std::cout << "cloudSize=" << cloudSize << " numFaces=" << numFaces << std::endl;
            std::cout << "New point: " << pair.sps[cloudSize].x << ", " << pair.sps[cloudSize].y << std::endl;
            std::cout << "SupportSet contents: ";
            for (ushort k = 0; k < setSize; k++) {
                std::cout << pair.supportSet[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "Remaining faces:" << std::endl;
            for (ushort k = 0; k < numFaces; k++) {
                std::cout << "  face " << k << ": va=" << pair.polytope[k].va
                        << " vb=" << pair.polytope[k].vb
                        << " normal=(" << pair.polytope[k].normal.x << ", " << pair.polytope[k].normal.y << ")"
                        << " dist=" << pair.polytope[k].distance << std::endl;
            }
            std::cout << "Cloud points:" << std::endl;
            for (ushort k = 0; k <= cloudSize; k++) {
                std::cout << "  sps[" << k << "]: " << pair.sps[k].x << ", " << pair.sps[k].y << std::endl;
            }

            std::cout << "Polytope horizon error" << std::endl;
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

        // debug
        float crossAfter = 0.0f;
        {
            glm::vec2 ab2 = pair.sps[1] - pair.sps[0];
            glm::vec2 ac2 = pair.sps[2] - pair.sps[0];
            crossAfter = ab2.x * ac2.y - ab2.y * ac2.x;
        }
        std::cout << "Cross after swap=" << crossAfter << std::endl;

        buildFace(pair, centroid, pair.supportSet[0], cloudSize, numFaces);
        buildFace(pair, centroid, cloudSize, pair.supportSet[1], numFaces + 1);

        // debug
        for (int k = 0; k < 3; k++) {
            std::cout << "  face " << k
                      << " dist=" << pair.polytope[k].distance
                      << " normal=(" << pair.polytope[k].normal.x
                      << ", " << pair.polytope[k].normal.y << ")"
                      << std::endl;
        }
        
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

ushort insertHorizon(SupportSet& supportSet, ushort spIndex, ushort setSize) {
    // print("SpSet before discard");
    // for (ushort i = 0; i < setSize; i++) {
    //     print(spSet[i]);
    // }

    if (discardHorizon(supportSet, spIndex, setSize)) {
        // print("SpSet before discarded");
        // for (ushort i = 0; i < setSize; i++) {
        //     print(spSet[i]);
        // }
        return --setSize;
    }

    supportSet[setSize] = spIndex;
    setSize++;
    // print("SpSet after add point");
    // for (ushort i = 0; i < setSize; i++) {
    //     print(spSet[i]);
    // }
    return setSize;
}

bool discardHorizon(SupportSet& supportSet, ushort spIndex, ushort setSize) {
    // print("SpSet enter discard");
    // for (ushort i = 0; i < setSize; i++) {
    //     print(spSet[i]);
    // }
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

    // print("SpSet in discard");
    // for (ushort i = 0; i < setSize; i++) {
    //     print(spSet[i]);
    // }

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

// // TODO check if the faces are oriented correctly
// void buildFace(CollisionPair& pair, ushort indexA, ushort indexB, ushort indexL) {
//     Polytope& polytope = pair.polytope;
//     PolytopeFace& face = polytope[indexL];
//     face.va = indexA;
//     face.vb = indexB;

//     // print("face indices");
//     // print(indexB);
//     // print(indexA);

//     glm::vec2 edge = pair.sps[indexB] - pair.sps[indexA];

//     // TODO check if we need midpoint or could just use vertex a
//     glm::vec2 normal = { -edge.y, edge.x };
//     if (glm::dot(pair.sps[indexA], normal) < 0) {
//         normal *= -1.0f;
//     }

//     face.normal = glm::normalize(normal);
//     face.distance = glm::dot(face.normal, pair.sps[indexA]);
// }

void buildFace(CollisionPair& pair, glm::vec2 interior, ushort indexA, ushort indexB, ushort indexL) {
    PolytopeFace& face = pair.polytope[indexL];
    face.va = indexA;
    face.vb = indexB;

    glm::vec2 edge = pair.sps[indexB] - pair.sps[indexA];
    glm::vec2 normal = { edge.y, -edge.x };

    float dotCheck = glm::dot(interior - pair.sps[indexA], normal);
    std::cout << "  buildFace[" << indexL << "] a=" << indexA << " b=" << indexB
              << " dotCheck=" << dotCheck
              << " flipped=" << (dotCheck > 0 ? "YES" : "NO") << std::endl;

    if (dotCheck > 0) {
        normal = -normal;
    }

    face.normal = glm::normalize(normal);
    face.distance = glm::dot(face.normal, pair.sps[indexA]);

    std::cout << "    -> dist=" << face.distance
              << " normal=(" << face.normal.x << ", " << face.normal.y << ")" << std::endl;
}

}

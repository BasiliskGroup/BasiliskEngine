#include <basilisk/physics/cellular/convexDecompose.h>

namespace bsk::internal {

void Convex::refreshTail()
{
    tail = head ? head->prev : nullptr;
}

void Convex::clearNodes() {
    if (!head) {
        vertexToNode.clear();
        return;
    }
    Node* cur = head;
    while (cur->next != head) {
        Node* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    delete cur;
    head = nullptr;
    tail = nullptr;
    vertexToNode.clear();
}

Convex::Convex(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
    Node* na = new Node{a, nullptr, nullptr};
    Node* nb = new Node{b, nullptr, nullptr};
    Node* nc = new Node{c, nullptr, nullptr};
    na->prev = nc; na->next = nb;
    nb->prev = na; nb->next = nc;
    nc->prev = nb; nc->next = na;
    head = na;
    tail = nc;
    vertexToNode[a] = na;
    vertexToNode[b] = nb;
    vertexToNode[c] = nc;
}

Convex::~Convex() {
    clearNodes();
}

Convex::Convex(Convex&& o) noexcept
    : head(o.head)
    , tail(o.tail)
    , vertexToNode(std::move(o.vertexToNode))
{
    o.head = nullptr;
    o.tail = nullptr;
}

Convex& Convex::operator=(Convex&& o) noexcept {
    if (this != &o) {
        clearNodes();
        head = o.head;
        tail = o.tail;
        vertexToNode = std::move(o.vertexToNode);
        o.head = nullptr;
        o.tail = nullptr;
    }
    return *this;
}

Convex::iterator Convex::operator[](const glm::vec2& v) {
    auto it = vertexToNode.find(v);
    if (it == vertexToNode.end()) {
        return end();
    }
    return iterator(it->second, tail);
}

Convex::const_iterator Convex::operator[](const glm::vec2& v) const {
    auto it = vertexToNode.find(v);
    if (it == vertexToNode.end()) {
        return end();
    }
    return const_iterator(it->second, tail);
}

bool Convex::add(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, glm::vec2& first, glm::vec2& last, glm::vec2& insert) {
    Node* na = nullptr;
    Node* nb = nullptr;
    Node* nc = nullptr;

    if (vertexToNode.find(a) != vertexToNode.end()) na = vertexToNode[a];
    if (vertexToNode.find(b) != vertexToNode.end()) nb = vertexToNode[b];
    if (vertexToNode.find(c) != vertexToNode.end()) nc = vertexToNode[c];
    
    bool a_found = na != nullptr;
    bool b_found = nb != nullptr;
    bool c_found = nc != nullptr;
    int found = a_found + b_found + c_found;

    // no edge found
    // = 3 is an error
    if (found != 2) return false;

    // find potential edge nodes
    Node* nfirst = a_found ? na : nb;
    Node* nlast = c_found ? nc : nb;

    // merge edges
    first = nfirst->pos;
    last = nlast->pos;
    insert = !a_found ? a : !b_found ? b : c;

    // check if we form an edge
    if (nlast->next == nfirst) {
        std::swap(nfirst, nlast);
    } else if (nfirst->next != nlast) {
        return false; // not an edge but contains both points (prove if this is possible)
    }

    // check if we form a convex shape
    if (!isConvex(nfirst->prev->pos, first, insert) || !isConvex(insert, last, nlast->next->pos)) {
        return false;
    }

    // insert new node between nfirst and nlast (nfirst -> ninsert -> nlast)
    Node* ninsert = new Node{insert, nlast, nfirst};
    nfirst->next = ninsert;
    nlast->prev = ninsert;
    vertexToNode[insert] = ninsert;

    refreshTail();

    return true;
}

// NOTE pass first and last in reverse order from add function
bool Convex::merge(Convex& other, glm::vec2& first, glm::vec2& last, glm::vec2& insert) {
    Node* nfirst = nullptr;
    Node* nlast = nullptr;
    Node* ninsert = nullptr;

    if (vertexToNode.find(first) != vertexToNode.end()) nfirst = vertexToNode[first];
    if (vertexToNode.find(last) != vertexToNode.end()) nlast = vertexToNode[last];
    if (vertexToNode.find(insert) != vertexToNode.end()) ninsert = vertexToNode[insert];

    bool first_found = nfirst != nullptr;
    bool last_found = nlast != nullptr;
    bool insert_found = ninsert != nullptr;
    int found = first_found + last_found + insert_found;

    if (found != 2) {
        return false;
    }
    (void)other;
    (void)nfirst;
    (void)nlast;
    (void)ninsert;
    return false;
}

bool Convex::isConvex(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) const {
    glm::vec2 ab = b - a;
    glm::vec2 bc = c - b;

    // 2D cross product (scalar z-component)
    float cross = ab.x * bc.y - ab.y * bc.x;

    return cross > 0.0f;
}

} // namespace bsk::internal

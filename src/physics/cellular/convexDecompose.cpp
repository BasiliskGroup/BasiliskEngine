#include <basilisk/physics/cellular/convexDecompose.h>

namespace bsk::internal {

void Convex::refreshTail() {
    if (!head) {
        tail = nullptr;
        return;
    }
    Node* cur = head;
    while (cur->next != head) {
        cur = cur->next;
    }
    tail = cur;
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

Convex::Convex(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    Node* na = new Node{a, nullptr};
    Node* nb = new Node{b, nullptr};
    Node* nc = new Node{c, nullptr};
    na->next = nb;
    nb->next = nc;
    nc->next = na;
    head = na;
    refreshTail();
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

bool Convex::add(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
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
    Node* first = a_found ? na : nb;
    Node* last = c_found ? nc : nb;

    // check if we form an edge
    if (last->next == first) {
        std::swap(first, last);
    } else if (first->next != last) {
        return false; // not an edge but contains both points (prove if this is possible)
    }

    // insert new node
    glm::vec2 insert_v = !a_found ? a : !b_found ? b : c;
    Node* insert = new Node{insert_v, last};
    first->next = insert;
    insert->next = last;
    vertexToNode[insert_v] = insert;

    // update tail
    if (last == head) { // eq to first = tail
        tail = insert;
    }

    return true;
}

} // namespace bsk::internal

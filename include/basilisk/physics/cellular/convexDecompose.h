#ifndef BSK_PHYSICS_CELLULAR_CONVEX_DECOMPOSE_H
#define BSK_PHYSICS_CELLULAR_CONVEX_DECOMPOSE_H

#include <basilisk/util/includes.h>
#include <basilisk/physics/cellular/helper.h>

#include <cstddef>
#include <iterator>
#include <unordered_map>

namespace bsk::internal {

// ----------------------------------------------
// Convex
// ----------------------------------------------

class Convex {
private:
    struct Node {
        glm::vec2 pos;
        Node* next = nullptr;
        Node* prev = nullptr;
    };

    Node* head = nullptr;
    Node* tail = nullptr;
    std::unordered_map<glm::vec2, Node*, Vec2Hash, Vec2KeyEq> vertexToNode;

    void refreshTail();
    void clearNodes();

public:
    class const_iterator {
        friend class Convex;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = glm::vec2;
        using reference = const glm::vec2&;
        using pointer = const glm::vec2*;

        const_iterator() = default;

        reference operator*() const { return cur_->pos; }
        pointer operator->() const { return &cur_->pos; }

        const_iterator& operator++()
        {
            if (!cur_) {
                return *this;
            }
            if (cur_ == tail_) {
                cur_ = nullptr;
            } else {
                cur_ = cur_->next;
            }
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++*this;
            return tmp;
        }

        friend bool operator==(const const_iterator& x, const const_iterator& y) { return x.cur_ == y.cur_; }
        friend bool operator!=(const const_iterator& x, const const_iterator& y) { return x.cur_ != y.cur_; }

    private:
        const_iterator(Node* cur, Node* tail) : cur_(cur), tail_(tail) {}

        Node* cur_ = nullptr;
        Node* tail_ = nullptr;
    };

    class iterator {
        friend class Convex;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = glm::vec2;
        using reference = glm::vec2&;
        using pointer = glm::vec2*;

        iterator() = default;

        reference operator*() const { return cur_->pos; }
        pointer operator->() const { return &cur_->pos; }

        iterator& operator++()
        {
            if (!cur_) {
                return *this;
            }
            if (cur_ == tail_) {
                cur_ = nullptr;
            } else {
                cur_ = cur_->next;
            }
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        operator const_iterator() const { return const_iterator(cur_, tail_); }

        friend bool operator==(const iterator& x, const iterator& y) { return x.cur_ == y.cur_; }
        friend bool operator!=(const iterator& x, const iterator& y) { return x.cur_ != y.cur_; }

    private:
        iterator(Node* cur, Node* tail) : cur_(cur), tail_(tail) {}

        Node* cur_ = nullptr;
        Node* tail_ = nullptr;
    };

    Convex(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);
    ~Convex();

    Convex(Convex&& o) noexcept;
    Convex& operator=(Convex&& o) noexcept;

    Convex(const Convex&) = delete;
    Convex& operator=(const Convex&) = delete;

    iterator begin() { return iterator(head, tail); }
    iterator end() { return iterator(nullptr, tail); }
    const_iterator begin() const { return const_iterator(head, tail); }
    const_iterator end() const { return const_iterator(nullptr, tail); }

    iterator operator[](const glm::vec2& v);
    const_iterator operator[](const glm::vec2& v) const;

    bool add(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, glm::vec2& first, glm::vec2& last, glm::vec2& insert);
    bool merge(Convex& other, glm::vec2& first, glm::vec2& last, glm::vec2& insert);
    bool isConvex(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) const;
};

}

#endif

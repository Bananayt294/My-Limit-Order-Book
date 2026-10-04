#pragma once

#include "book.hpp"
#include "limit.hpp"
#include "order.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace lob_test {

inline limit* rootOf(limit* node) {
    while (node != nullptr && node->get_parent() != nullptr) {
        node = node->get_parent();
    }
    return node;
}

inline int treeHeight(limit* node) {
    if (node == nullptr) {
        return 0;
    }

    return 1 + std::max(
        treeHeight(node->get_leftchild()),
        treeHeight(node->get_rightchild())
    );
}

inline std::vector<int> inOrder(limit* node) {
    std::vector<int> result;

    if (node == nullptr) {
        return result;
    }

    const auto left = inOrder(node->get_leftchild());
    const auto right = inOrder(node->get_rightchild());

    result.insert(result.end(), left.begin(), left.end());
    result.push_back(node->get_limitPrice());
    result.insert(result.end(), right.begin(), right.end());

    return result;
}

inline limit* findLimit(limit* node, int price) {
    while (node != nullptr) {
        if (price == node->get_limitPrice()) {
            return node;
        }

        node = price < node->get_limitPrice()
            ? node->get_leftchild()
            : node->get_rightchild();
    }

    return nullptr;
}

inline void verifyTree(limit* node, limit* expectedParent = nullptr) {
    if (node == nullptr) {
        return;
    }

    if (node->get_parent() != expectedParent) {
        throw std::logic_error("Invalid AVL parent pointer");
    }

    limit* left = node->get_leftchild();
    limit* right = node->get_rightchild();

    if (left != nullptr && left->get_limitPrice() >= node->get_limitPrice()) {
        throw std::logic_error("Invalid AVL ordering on left child");
    }

    if (right != nullptr && right->get_limitPrice() <= node->get_limitPrice()) {
        throw std::logic_error("Invalid AVL ordering on right child");
    }

    const int leftHeight = treeHeight(left);
    const int rightHeight = treeHeight(right);

    if (std::abs(leftHeight - rightHeight) > 1) {
        throw std::logic_error("AVL balance invariant violated");
    }

    verifyTree(left, node);
    verifyTree(right, node);
}

inline void verifyBookAvl(book& bookRef) {
    if (bookRef.getHighestBuy() != nullptr) {
        limit* buyRoot = rootOf(bookRef.getHighestBuy());
        verifyTree(buyRoot);
    }

    if (bookRef.getLowestSell() != nullptr) {
        limit* sellRoot = rootOf(bookRef.getLowestSell());
        verifyTree(sellRoot);
    }
}

inline void verifyBookEdges(book& bookRef) {
    if (bookRef.getHighestBuy() != nullptr) {
        limit* buyRoot = rootOf(bookRef.getHighestBuy());
        const auto prices = inOrder(buyRoot);
        if (prices.empty() ||
            bookRef.getHighestBuy()->get_limitPrice() != prices.back()) {
            throw std::logic_error("Highest buy edge is incorrect");
        }
    }

    if (bookRef.getLowestSell() != nullptr) {
        limit* sellRoot = rootOf(bookRef.getLowestSell());
        const auto prices = inOrder(sellRoot);
        if (prices.empty() ||
            bookRef.getLowestSell()->get_limitPrice() != prices.front()) {
            throw std::logic_error("Lowest sell edge is incorrect");
        }
    }
}

inline order* randomOrderOfType(book& bookRef, int type) {
    std::mt19937 rng(0xC0FFEEu);
    return bookRef.getRandomOrder(type, rng);
}

} // namespace lob_test

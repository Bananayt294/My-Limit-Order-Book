#include <gtest/gtest.h>
#include "test_support.hpp"

using lob_test::findLimit;
using lob_test::inOrder;
using lob_test::rootOf;
using lob_test::verifyBookAvl;
using lob_test::verifyBookEdges;

class AvlTest : public ::testing::Test {
protected:
    book book;
    int nextId = 1;

    void addBuy(int price, int shares = 10) {
        book.AddLimitOrder(nextId++, true, shares, price);
    }

    void addSell(int price, int shares = 10) {
        book.AddLimitOrder(nextId++, false, shares, price);
    }
};

TEST_F(AvlTest, SinglePriceLevelBecomesRoot) {
    addBuy(100);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    limit* root = rootOf(book.getHighestBuy());
    ASSERT_NE(root, nullptr);

    EXPECT_EQ(root->get_limitPrice(), 100);
    EXPECT_EQ(root->get_parent(), nullptr);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(AvlTest, LeftLeftInsertionRebalances) {
    addBuy(300);
    addBuy(200);
    addBuy(100);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    limit* root = rootOf(book.getHighestBuy());

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_limitPrice(), 200);
    ASSERT_NE(root->get_leftchild(), nullptr);
    ASSERT_NE(root->get_rightchild(), nullptr);
    EXPECT_EQ(root->get_leftchild()->get_limitPrice(), 100);
    EXPECT_EQ(root->get_rightchild()->get_limitPrice(), 300);
    EXPECT_NO_THROW(verifyBookAvl(book));
}

TEST_F(AvlTest, RightRightInsertionRebalances) {
    addBuy(100);
    addBuy(200);
    addBuy(300);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    limit* root = rootOf(book.getHighestBuy());

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_limitPrice(), 200);
    ASSERT_NE(root->get_leftchild(), nullptr);
    ASSERT_NE(root->get_rightchild(), nullptr);
    EXPECT_EQ(root->get_leftchild()->get_limitPrice(), 100);
    EXPECT_EQ(root->get_rightchild()->get_limitPrice(), 300);
    EXPECT_NO_THROW(verifyBookAvl(book));
}

TEST_F(AvlTest, LeftRightInsertionRebalances) {
    addBuy(300);
    addBuy(100);
    addBuy(200);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    limit* root = rootOf(book.getHighestBuy());

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_limitPrice(), 200);
    EXPECT_EQ(root->get_leftchild()->get_limitPrice(), 100);
    EXPECT_EQ(root->get_rightchild()->get_limitPrice(), 300);
    EXPECT_NO_THROW(verifyBookAvl(book));
}

TEST_F(AvlTest, RightLeftInsertionRebalances) {
    addBuy(100);
    addBuy(300);
    addBuy(200);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    limit* root = rootOf(book.getHighestBuy());

    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->get_limitPrice(), 200);
    EXPECT_EQ(root->get_leftchild()->get_limitPrice(), 100);
    EXPECT_EQ(root->get_rightchild()->get_limitPrice(), 300);
    EXPECT_NO_THROW(verifyBookAvl(book));
}

TEST_F(AvlTest, ParentPointersRemainCorrectAfterRotations) {
    addBuy(300);
    addBuy(100);
    addBuy(200);
    addBuy(50);
    addBuy(25);
    addBuy(400);
    addBuy(450);

    EXPECT_NO_THROW(verifyBookAvl(book));
}

TEST_F(AvlTest, InorderTraversalRemainsSorted) {
    addBuy(70);
    addBuy(20);
    addBuy(100);
    addBuy(10);
    addBuy(40);
    addBuy(90);
    addBuy(120);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    limit* root = rootOf(book.getHighestBuy());

    const std::vector<int> expected{10, 20, 40, 70, 90, 100, 120};
    EXPECT_EQ(inOrder(root), expected);
}

TEST_F(AvlTest, BalanceFactorRemainsWithinAvlRange) {
    const int prices[] = {
        500, 200, 800, 100, 300, 700, 900,
        50, 150, 250, 350, 650, 750, 850, 950
    };

    for (int price : prices) {
        addBuy(price);
        EXPECT_NO_THROW(verifyBookAvl(book));
    }
}

TEST_F(AvlTest, RandomInsertionMaintainsAllAvlInvariants) {
    std::vector<int> prices;
    for (int p = 1; p <= 1000; ++p) {
        prices.push_back(p * 2);
    }

    std::mt19937 rng(20261003);
    std::shuffle(prices.begin(), prices.end(), rng);

    int id = 1;
    for (const int price : prices) {
        book.AddLimitOrder(id++, true, 10 + (price % 37), price);
        ASSERT_NO_THROW(verifyBookAvl(book)) << "Failure after price " << price;
        ASSERT_NO_THROW(verifyBookEdges(book)) << "Edge failure after price " << price;
    }
}

TEST_F(AvlTest, BuyAndSellTreesAreIndependent) {
    addBuy(100);
    addBuy(90);
    addSell(110);
    addSell(120);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    ASSERT_NE(book.getLowestSell(), nullptr);

    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 110);

    EXPECT_NE(findLimit(rootOf(book.getHighestBuy()), 90), nullptr);
    EXPECT_NE(findLimit(rootOf(book.getLowestSell()), 120), nullptr);

    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

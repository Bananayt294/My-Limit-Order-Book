#include <gtest/gtest.h>
#include "test_support.hpp"

using lob_test::randomOrderOfType;
using lob_test::verifyBookAvl;
using lob_test::verifyBookEdges;

class StopOrderTest : public ::testing::Test {
protected:
    book book;
};

TEST_F(StopOrderTest, StopBuyCanBeAdded) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddStopOrder(2, true, 50, 110);

    order* order = randomOrderOfType(book, 1);
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->get_idNumber(), 2);
    EXPECT_TRUE(order->get_buyorsell());
    EXPECT_EQ(order->getshares(), 50);
    EXPECT_EQ(order->get_parent_limit()->get_limitPrice(), 110);
}

TEST_F(StopOrderTest, StopSellCanBeAdded) {
    book.AddLimitOrder(1, true, 100, 110);
    book.AddStopOrder(2, false, 50, 100);

    order* order = randomOrderOfType(book, 1);
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->get_idNumber(), 2);
    EXPECT_FALSE(order->get_buyorsell());
    EXPECT_EQ(order->getshares(), 50);
    EXPECT_EQ(order->get_parent_limit()->get_limitPrice(), 100);
}

TEST_F(StopOrderTest, CancelStopOrderRemovesTheorder) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddStopOrder(2, true, 50, 110);

    ASSERT_NE(randomOrderOfType(book, 1), nullptr);

    book.CancelStopOrder(2);

    EXPECT_EQ(randomOrderOfType(book, 1), nullptr);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(StopOrderTest, ModifyStopOrderChangesQuantityAndStopLevel) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddStopOrder(2, true, 50, 110);

    book.ModifyStopOrder(2, 125, 115);

    order* order = randomOrderOfType(book, 1);
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->get_idNumber(), 2);
    EXPECT_EQ(order->getshares(), 125);
    EXPECT_EQ(order->get_parent_limit()->get_limitPrice(), 115);
}

TEST_F(StopOrderTest, StopLimitOrderCanBeStored) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddStopLimitOrder(2, true, 50, 120, 110);

    order* order = randomOrderOfType(book, 2);
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->get_idNumber(), 2);
    EXPECT_TRUE(order->get_buyorsell());
    EXPECT_EQ(order->getshares(), 50);
    EXPECT_EQ(order->get_Limit(), 120);
    EXPECT_EQ(order->get_parent_limit()->get_limitPrice(), 110);
}

TEST_F(StopOrderTest, CancelStopLimitOrderRemovesTheorder) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddStopLimitOrder(2, true, 50, 120, 110);

    ASSERT_NE(randomOrderOfType(book, 2), nullptr);

    book.CancelStopLimitOrder(2);

    EXPECT_EQ(randomOrderOfType(book, 2), nullptr);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(StopOrderTest, ModifyStopLimitOrderUpdatesAllorderValues) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddStopLimitOrder(2, true, 50, 120, 110);

    book.ModifyStopLimitOrder(2, 175, 130, 115);

    order* order = randomOrderOfType(book, 2);
    ASSERT_NE(order, nullptr);
    EXPECT_EQ(order->get_idNumber(), 2);
    EXPECT_EQ(order->getshares(), 175);
    EXPECT_EQ(order->get_Limit(), 130);
    EXPECT_EQ(order->get_parent_limit()->get_limitPrice(), 115);
}

TEST_F(StopOrderTest, StopStructuresDoNotCorruptNormalBookEdges) {
    book.AddLimitOrder(1, true, 100, 95);
    book.AddLimitOrder(2, false, 100, 105);
    book.AddStopOrder(3, true, 50, 115);
    book.AddStopLimitOrder(4, false, 25, 90, 85);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 95);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 105);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

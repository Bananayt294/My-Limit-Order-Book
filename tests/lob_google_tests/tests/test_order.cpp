#include <gtest/gtest.h>
#include "test_support.hpp"

TEST(OrderTest, ConstructorStoresAllFields) {
    order order(42, true, 500, 101);

    EXPECT_EQ(order.get_idNumber(), 42);
    EXPECT_TRUE(order.get_buyorsell());
    EXPECT_EQ(order.getshares(), 500);
    EXPECT_EQ(order.get_Limit(), 101);
}

TEST(OrderTest, SellOrderStoresSellSide) {
    order order(7, false, 125, 205);

    EXPECT_EQ(order.get_idNumber(), 7);
    EXPECT_FALSE(order.get_buyorsell());
    EXPECT_EQ(order.getshares(), 125);
    EXPECT_EQ(order.get_Limit(), 205);
}

TEST(OrderTest, ModifyOrderUpdatesQuantityAndPrice) {
    order order(9, true, 100, 100);

    order.modifyorder(350, 115);

    EXPECT_EQ(order.getshares(), 350);
    EXPECT_EQ(order.get_Limit(), 115);
    EXPECT_EQ(order.get_idNumber(), 9);
    EXPECT_TRUE(order.get_buyorsell());
}

TEST(OrderTest, PartialFillReducesQuantity) {
    order order(10, true, 500, 100);

    order.partiallyFillOrder(125);

    EXPECT_EQ(order.getshares(), 375);
}

TEST(OrderTest, FullExecutionLeavesNoRemainingShares) {
    order order(11, true, 250, 100);

    order.execute();

    EXPECT_EQ(order.getshares(), 0);
}

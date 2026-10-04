#include <gtest/gtest.h>
#include "test_support.hpp"

using lob_test::findLimit;
using lob_test::rootOf;
using lob_test::verifyBookAvl;
using lob_test::verifyBookEdges;

class bookTest : public ::testing::Test {
protected:
    book book;
};

TEST_F(bookTest, EmptybookHasNoBestBidOrAsk) {
    EXPECT_EQ(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getLowestSell(), nullptr);
    EXPECT_NO_THROW(verifyBookAvl(book));
}

TEST_F(bookTest, BestBidIsHighestBuyPrice) {
    book.AddLimitOrder(1, true, 100, 100);
    book.AddLimitOrder(2, true, 100, 105);
    book.AddLimitOrder(3, true, 100, 101);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 105);
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(bookTest, BestAskIsLowestSellPrice) {
    book.AddLimitOrder(1, false, 100, 110);
    book.AddLimitOrder(2, false, 100, 105);
    book.AddLimitOrder(3, false, 100, 108);

    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 105);
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(bookTest, ordersAtSamePriceShareOnelimit) {
    book.AddLimitOrder(1, true, 100, 100);
    book.AddLimitOrder(2, true, 250, 100);
    book.AddLimitOrder(3, true, 50, 100);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_EQ(book.getHighestBuy()->get_size(), 3);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 400);
}

TEST_F(bookTest, ordersAtSamePricePreserveFifoHeadAndTail) {
    book.AddLimitOrder(1, true, 100, 100);
    book.AddLimitOrder(2, true, 200, 100);
    book.AddLimitOrder(3, true, 300, 100);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    ASSERT_NE(book.getHighestBuy()->get_headOrder(), nullptr);
    ASSERT_NE(book.getHighestBuy()->get_tailOrder(), nullptr);

    EXPECT_EQ(book.getHighestBuy()->get_headOrder()->get_idNumber(), 1);
    EXPECT_EQ(book.getHighestBuy()->get_tailOrder()->get_idNumber(), 3);
}

TEST_F(bookTest, CancelOnlyorderRemovesPriceLevel) {
    book.AddLimitOrder(1, true, 100, 100);
    ASSERT_NE(book.getHighestBuy(), nullptr);

    book.CancelLimitOrder(1);

    EXPECT_EQ(book.getHighestBuy(), nullptr);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(bookTest, CancelBestBidMovesBestBidToNextPrice) {
    book.AddLimitOrder(1, true, 100, 100);
    book.AddLimitOrder(2, true, 100, 105);
    book.AddLimitOrder(3, true, 100, 110);

    book.CancelLimitOrder(3);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 105);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(bookTest, CancelBestAskMovesBestAskToNextPrice) {
    book.AddLimitOrder(1, false, 100, 110);
    book.AddLimitOrder(2, false, 100, 105);
    book.AddLimitOrder(3, false, 100, 115);

    book.CancelLimitOrder(2);

    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 110);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(bookTest, CancelOneorderAtPriceLeavesOtherordersIntact) {
    book.AddLimitOrder(1, true, 100, 100);
    book.AddLimitOrder(2, true, 250, 100);

    book.CancelLimitOrder(1);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_size(), 1);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 250);
    ASSERT_NE(book.getHighestBuy()->get_headOrder(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_headOrder()->get_idNumber(), 2);
}

TEST_F(bookTest, ModifyLimitorderMovesorderToNewPrice) {
    book.AddLimitOrder(1, true, 100, 100);
    book.AddLimitOrder(2, true, 100, 110);

    book.ModifyLimitOrder(1, 150, 105);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 110);

    limit* root = rootOf(book.getHighestBuy());
    ASSERT_NE(findLimit(root, 105), nullptr);
    EXPECT_EQ(findLimit(root, 105)->get_totalshares(), 150);
    EXPECT_NO_THROW(verifyBookAvl(book));
}

TEST_F(bookTest, NonCrossinglimitordersRemainOnbook) {
    book.AddLimitOrder(1, false, 500, 110);
    book.AddLimitOrder(2, true, 300, 100);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 300);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 110);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 500);
}

TEST_F(bookTest, CrossingBuyConsumesSellOrder) {
    book.AddLimitOrder(1, false, 500, 100);
    book.AddLimitOrder(2, true, 300, 100);

    EXPECT_EQ(book.getHighestBuy(), nullptr);
    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 100);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 200);
}

TEST_F(bookTest, CrossingSellConsumesBuyorder) {
    book.AddLimitOrder(1, true, 500, 100);
    book.AddLimitOrder(2, false, 300, 100);

    EXPECT_EQ(book.getLowestSell(), nullptr);
    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 200);
}

TEST_F(bookTest, BuyMarketorderFullyConsumesBestAsk) {
    book.AddLimitOrder(1, false, 500, 100);
    book.AddLimitOrder(2, false, 300, 101);

    book.marketOrder(3, true, 500);

    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 101);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 300);
}

TEST_F(bookTest, SellMarketorderFullyConsumesBestBid) {
    book.AddLimitOrder(1, true, 500, 101);
    book.AddLimitOrder(2, true, 300, 100);

    book.marketOrder(3, false, 500);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 300);
}

TEST_F(bookTest, BuyMarketorderPartiallyFillsBestAsk) {
    book.AddLimitOrder(1, false, 500, 100);

    book.marketOrder(2, true, 200);

    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 300);
}

TEST_F(bookTest, SellMarketorderPartiallyFillsBestBid) {
    book.AddLimitOrder(1, true, 500, 100);

    book.marketOrder(2, false, 200);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 300);
}

TEST_F(bookTest, BuyMarketorderCanWalkMultiplePriceLevels) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddLimitOrder(2, false, 150, 101);
    book.AddLimitOrder(3, false, 200, 102);

    book.marketOrder(4, true, 250);

    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 102);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 200);
}

TEST_F(bookTest, SamePriceordersAreMatchedFIFO) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddLimitOrder(2, false, 200, 100);

    book.marketOrder(3, true, 150);

    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 100);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 150);
    ASSERT_NE(book.getLowestSell()->get_headOrder(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_headOrder()->get_idNumber(), 2);
}

TEST_F(bookTest, limitorderCanConsumeMultiplePriceLevels) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddLimitOrder(2, false, 100, 101);

    book.AddLimitOrder(3, true, 150, 101);

    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 101);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 50);
}

TEST_F(bookTest, MarketorderOnEmptybookIsSafe) {
    EXPECT_NO_THROW(book.marketOrder(1, true, 100));
    EXPECT_EQ(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getLowestSell(), nullptr);
}

TEST_F(bookTest, MarketorderFullyConsumesMultipleLevels) {
    book.AddLimitOrder(1, false, 100, 100);
    book.AddLimitOrder(2, false, 150, 101);

    book.marketOrder(3, true, 250);

    EXPECT_EQ(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getHighestBuy(), nullptr);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

TEST_F(bookTest, InorderTraversalIsSortedForBuyTree) {
    book.AddLimitOrder(1, true, 10, 100);
    book.AddLimitOrder(2, true, 10, 90);
    book.AddLimitOrder(3, true, 10, 110);

    ASSERT_NE(book.getHighestBuy(), nullptr);
    limit* root = rootOf(book.getHighestBuy());
    EXPECT_EQ(book.InorderTraversal(const_cast<limit*>(root)),
              (std::vector<int>{90, 100, 110}));
}

TEST_F(bookTest, bookInvariantsSurviveMixedBasicOperations) {
    book.AddLimitOrder(1, true, 100, 95);
    book.AddLimitOrder(2, true, 200, 100);
    book.AddLimitOrder(3, true, 150, 105);
    book.AddLimitOrder(4, false, 300, 110);
    book.AddLimitOrder(5, false, 150, 115);

    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));

    book.CancelLimitOrder(3);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));

    book.ModifyLimitOrder(1, 125, 97);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));

    book.marketOrder(99, true, 200);
    EXPECT_NO_THROW(verifyBookAvl(book));
    EXPECT_NO_THROW(verifyBookEdges(book));
}

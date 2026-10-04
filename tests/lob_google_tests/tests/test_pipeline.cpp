#include <gtest/gtest.h>
#include "OrderPipeline.hpp"
#include "../LOB/book.hpp"
#include "../LOB/limit.hpp"
#include <filesystem>
#include <fstream>

class OrderPipelineTest : public ::testing::Test {
protected:
    std::filesystem::path testFile;

    void SetUp() override {
        testFile = std::filesystem::temp_directory_path() / "lob_google_test_replay.txt";
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove(testFile, ec);
    }

    void writeFile(const std::string& contents) {
        std::ofstream out(testFile);
        ASSERT_TRUE(out.is_open());
        out << contents;
        out.close();
    }
};

TEST_F(OrderPipelineTest, ParsesAddLimitordersIntobook) {
    writeFile(
        "AddLimit 1 1 100 95\n"
        "AddLimit 2 1 200 100\n"
        "AddLimit 3 0 150 105\n"
    );

    book book;
    OrderPipeline pipeline(&book);

    pipeline.processOrdersFromFile(testFile.string());

    ASSERT_NE(book.getHighestBuy(), nullptr);
    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 200);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 105);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 150);
}

TEST_F(OrderPipelineTest, ParsesCancelLimitorder) {
    writeFile(
        "AddLimit 1 1 100 95\n"
        "AddLimit 2 1 200 100\n"
        "CancelLimit 2\n"
    );

    book book;
    OrderPipeline pipeline(&book);

    pipeline.processOrdersFromFile(testFile.string());

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 95);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 100);
}

TEST_F(OrderPipelineTest, ParsesModifyLimitorder) {
    writeFile(
        "AddLimit 1 1 100 95\n"
        "ModifyLimit 1 250 98\n"
    );

    book book;
    OrderPipeline pipeline(&book);

    pipeline.processOrdersFromFile(testFile.string());

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 98);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 250);
}

TEST_F(OrderPipelineTest, ProcessesMultipleOperationsInOrder) {
    writeFile(
        "AddLimit 1 1 100 95\n"
        "AddLimit 2 0 300 105\n"
        "AddLimit 3 1 50 100\n"
        "CancelLimit 1\n"
        "ModifyLimit 2 250 110\n"
    );

    book book;
    OrderPipeline pipeline(&book);

    pipeline.processOrdersFromFile(testFile.string());

    ASSERT_NE(book.getHighestBuy(), nullptr);
    ASSERT_NE(book.getLowestSell(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), 50);
    EXPECT_EQ(book.getLowestSell()->get_limitPrice(), 110);
    EXPECT_EQ(book.getLowestSell()->get_totalshares(), 250);
}

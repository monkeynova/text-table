#include "text_table.h"

#include "absl/flags/reflection.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "gtest/gtest.h"

using enum TextTable::Cell::Color;
using enum TextTable::Cell::Justify;

TEST(TextTable, Basic) {
    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Text"}});
    table.AddBreaker();
    EXPECT_EQ(absl::StrCat("\n", table.Render()),
R"TEXT(
+------+
| Text |
+------+
)TEXT");
}

TEST(TextTable, JustifyRight) {
    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Really Long Test"}});
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Foo", .justify = kRight}});
    table.AddBreaker();
    EXPECT_EQ(absl::StrCat("\n", table.Render()),
R"TEXT(
+------------------+
| Really Long Test |
+------------------+
|              Foo |
+------------------+
)TEXT");
}

TEST(TextTable, JustifyLeft) {
    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Really Long Test"}});
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Foo", .justify = kLeft}});
    table.AddBreaker();
    EXPECT_EQ(absl::StrCat("\n", table.Render()),
R"TEXT(
+------------------+
| Really Long Test |
+------------------+
| Foo              |
+------------------+
)TEXT");
}

TEST(TextTable, JustifyCenter) {
    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Really Long Test"}});
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Foo", .justify = kCenter}});
    table.AddBreaker();
    EXPECT_EQ(absl::StrCat("\n", table.Render()),
R"TEXT(
+------------------+
| Really Long Test |
+------------------+
|       Foo        |
+------------------+
)TEXT");
}

TEST(TextTable, Span) {
    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Really Long Test", .span = 2}});
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Foo"},
                  TextTable::Cell{.entry = "Bar"}});
    table.AddBreaker();
    EXPECT_EQ(absl::StrCat("\n", table.Render()),
R"TEXT(
+-------------------+
| Really Long Test  |
+---------+---------+
| Foo     | Bar     |
+---------+---------+
)TEXT");
}

TEST(TextTable, Bold) {
    auto flag = absl::FindCommandLineFlag("color");
    std::string error;
    ASSERT_TRUE(flag->ParseFrom("force", &error)) << error;

    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Foo", .bold = true}});
    table.AddBreaker();
    EXPECT_EQ(table.Render(),
              "+-----+\n| \x1B[1mFoo\x1B[0m |\n+-----+\n");
}

TEST(TextTable, ColorRed) {
    auto flag = absl::FindCommandLineFlag("color");
    std::string error;
    ASSERT_TRUE(flag->ParseFrom("force", &error)) << error;

    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Foo", .color = kRed}});
    table.AddBreaker();
    EXPECT_EQ(table.Render(),
              "+-----+\n| \x1B[31mFoo\x1B[0m |\n+-----+\n");
}

TEST(TextTable, ColorYellow) {
    auto flag = absl::FindCommandLineFlag("color");
    std::string error;
    ASSERT_TRUE(flag->ParseFrom("force", &error)) << error;

    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Foo", .color = kYellow}});
    table.AddBreaker();
    EXPECT_EQ(table.Render(),
              "+-----+\n| \x1B[33mFoo\x1B[0m |\n+-----+\n");
}

TEST(TextTable, ColorGreen) {
    auto flag = absl::FindCommandLineFlag("color");
    std::string error;
    ASSERT_TRUE(flag->ParseFrom("force", &error)) << error;

    TextTable table;
    table.AddBreaker();
    table.AddRow({TextTable::Cell{.entry = "Foo", .color = kGreen}});
    table.AddBreaker();
    EXPECT_EQ(table.Render(),
              "+-----+\n| \x1B[32mFoo\x1B[0m |\n+-----+\n");
}

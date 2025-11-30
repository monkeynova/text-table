#include "text_table.h"

#include "absl/strings/str_cat.h"
#include "gtest/gtest.h"

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

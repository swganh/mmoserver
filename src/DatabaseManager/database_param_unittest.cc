// Values bound to prepared statements: BString::getAnsi() is null for a
// string that is not ANSI, and that must bind as empty text, not throw.

#include <gtest/gtest.h>

#include "DatabaseManager/Database.h"

TEST(DatabaseParamTest, NullTextIsEmpty) {
    const char* none = nullptr;
    DatabaseParams params = {none, "O'Brien %s", std::string("x\0y", 3)};
    EXPECT_EQ("", params[0].value);
    EXPECT_EQ("O'Brien %s", params[1].value);
    EXPECT_EQ(3u, params[2].value.size());
}

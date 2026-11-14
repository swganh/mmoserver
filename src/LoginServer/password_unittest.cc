// Account passwords: Argon2id hashes, and old SHA-1 ones accepted once.

#include <gtest/gtest.h>

#include "LoginServer/Password.h"

class PasswordTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() { ASSERT_TRUE(password::init()); }
};

TEST_F(PasswordTest, HashIsSaltedArgon2id) {
    std::string a = password::hash("secret password");
    std::string b = password::hash("secret password");
    EXPECT_TRUE(password::isArgon2id(a));
    EXPECT_NE(a, b);  // a new salt each time
}

TEST_F(PasswordTest, RightAndWrongPasswords) {
    std::string stored = password::hash("secret password");
    EXPECT_EQ(password::Check::Right, password::check(stored, "secret password", false));
    EXPECT_EQ(password::Check::Wrong, password::check(stored, "secret passwore", false));
    EXPECT_EQ(password::Check::Wrong, password::check(stored, "", false));
    // The database's SHA-1 answer counts only for an old password.
    EXPECT_EQ(password::Check::Wrong, password::check(stored, "wrong", true));
}

TEST_F(PasswordTest, OldSha1PasswordIsAcceptedOnceAndReplaced) {
    const std::string sha1 = "e5e9fa1ba31ecd1ae84f75caaa474f3a663f05f4";  // SHA1("secret")
    EXPECT_EQ(password::Check::RightRehash, password::check(sha1, "secret", true));
    EXPECT_EQ(password::Check::Wrong, password::check(sha1, "secret", false));
}

TEST_F(PasswordTest, DamagedHashIsWrong) {
    EXPECT_EQ(password::Check::Wrong, password::check("$argon2id$garbage", "secret", true));
    EXPECT_EQ(password::Check::Wrong, password::check("", "secret", false));
}

// Unicode16 BStrings hold 16-bit UTF-16 units, as the client sends them.
// These tests guard the conversions on platforms where wchar_t is 32 bits
// (Linux), where client text used to arrive garbled.

#include "Utils/bstring.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

// Builds a string the way Message::getStringUnicode16 does: a 32-bit length
// followed by UTF-16 code units.
BString FromClientUnicode16(const std::u16string& text) {
    std::vector<char> packet(4 + text.size() * 2 + 2, 0);
    uint32_t length = static_cast<uint32_t>(text.size());
    std::memcpy(packet.data(), &length, sizeof(length));
    std::memcpy(packet.data() + 4, text.data(), text.size() * 2);

    BString result;
    result.initRawBSTR(packet.data(), BSTRType_Unicode16);
    return result;
}

TEST(BStringTests, ClientUnicode16ConvertsToAnsi) {
    BString name = FromClientUnicode16(u"mos_eisley");
    ASSERT_EQ(10, name.getLength());

    name.convert(BSTRType_ANSI);
    EXPECT_STREQ("mos_eisley", name.getAnsi());
}

TEST(BStringTests, ClientUnicode16NumbersParseAfterConversion) {
    BString ids = FromClientUnicode16(u"123456789 42");
    ids.convert(BSTRType_ANSI);

    BStringVector tokens;
    ASSERT_EQ(2, ids.split(tokens, ' '));
    EXPECT_STREQ("123456789", tokens[0].getAnsi());
    EXPECT_STREQ("42", tokens[1].getAnsi());
}

TEST(BStringTests, NonLatin1CharactersBecomeQuestionMarks) {
    BString text = FromClientUnicode16(u"a中b");
    text.convert(BSTRType_ANSI);
    EXPECT_STREQ("a?b", text.getAnsi());
}

TEST(BStringTests, GetUnicode16ReturnsWideText) {
    BString name = FromClientUnicode16(u"Han Solo");
    EXPECT_EQ(std::wstring(L"Han Solo"), name.getUnicode16());
}

TEST(BStringTests, AssignFromWideStringKeepsEveryCharacter) {
    BString text;
    text = L"Mos Espa";

    EXPECT_EQ(8, text.getLength());
    EXPECT_EQ(std::wstring(L"Mos Espa"), text.getUnicode16());

    text.convert(BSTRType_ANSI);
    EXPECT_STREQ("Mos Espa", text.getAnsi());
}

TEST(BStringTests, AssignFromUint16StringMeasuresLengthCorrectly) {
    const uint16_t data[] = {'t', 'a', 't', 'o', 'o', 'i', 'n', 'e', 0};
    BString text;
    text = data;

    EXPECT_EQ(8, text.getLength());
    EXPECT_EQ(std::wstring(L"tatooine"), text.getUnicode16());
}

TEST(BStringTests, Unicode16SubstringCopiesTheRange) {
    BString text;
    text = L"Mos Eisley";

    BString part;
    text.substring(part, 4, 10);
    EXPECT_EQ(std::wstring(L"Eisley"), part.getUnicode16());
}

}  // namespace

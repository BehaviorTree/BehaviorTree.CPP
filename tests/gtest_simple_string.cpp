/* Copyright (C) 2018-2023 Davide Faconti, Eurecat - All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "behaviortree_cpp/utils/simple_string.hpp"

#include <gtest/gtest.h>

using namespace SafeAny;

// Test default constructor
TEST(SimpleStringTest, DefaultConstructor)
{
  SimpleString s;
  EXPECT_EQ(s.size(), 0);
  EXPECT_STREQ(s.data(), "");
  EXPECT_TRUE(s.isSOO());
}

// Test construction from empty string
TEST(SimpleStringTest, EmptyString)
{
  SimpleString s("");
  EXPECT_EQ(s.size(), 0);
  EXPECT_STREQ(s.data(), "");
  EXPECT_TRUE(s.isSOO());
}

// Test construction from const char*
TEST(SimpleStringTest, ConstructFromCString)
{
  SimpleString s("hello");
  EXPECT_EQ(s.size(), 5);
  EXPECT_STREQ(s.data(), "hello");
  EXPECT_TRUE(s.isSOO());
}

// Test construction from const char* with explicit size
TEST(SimpleStringTest, ConstructFromCStringWithSize)
{
  const char* text = "hello world";
  SimpleString s(text, 5);
  EXPECT_EQ(s.size(), 5);
  EXPECT_STREQ(s.data(), "hello");
  EXPECT_TRUE(s.isSOO());
}

// Test construction from std::string
TEST(SimpleStringTest, ConstructFromStdString)
{
  std::string str = "testing";
  SimpleString s(str);
  EXPECT_EQ(s.size(), 7);
  EXPECT_STREQ(s.data(), "testing");
  EXPECT_TRUE(s.isSOO());
}

// Test construction from std::string_view
TEST(SimpleStringTest, ConstructFromStringView)
{
  std::string_view sv = "view test";
  SimpleString s(sv);
  EXPECT_EQ(s.size(), 9);
  EXPECT_STREQ(s.data(), "view test");
  EXPECT_TRUE(s.isSOO());
}

// Test SOO boundary - exactly 15 characters (max SOO capacity)
TEST(SimpleStringTest, SOOBoundaryExact)
{
  // Exactly 15 characters - should still use SOO
  SimpleString s("123456789012345");
  EXPECT_EQ(s.size(), 15);
  EXPECT_STREQ(s.data(), "123456789012345");
  EXPECT_TRUE(s.isSOO());
}

// Test SOO boundary - 16 characters (exceeds SOO capacity)
TEST(SimpleStringTest, SOOBoundaryExceeded)
{
  // 16 characters - should use heap allocation
  SimpleString s("1234567890123456");
  EXPECT_EQ(s.size(), 16);
  EXPECT_STREQ(s.data(), "1234567890123456");
  EXPECT_FALSE(s.isSOO());
}

// Test long string (non-SOO)
TEST(SimpleStringTest, LongString)
{
  std::string longStr(100, 'x');
  SimpleString s(longStr);
  EXPECT_EQ(s.size(), 100);
  EXPECT_EQ(s.toStdString(), longStr);
  EXPECT_FALSE(s.isSOO());
}

// Test copy constructor with SOO string
TEST(SimpleStringTest, CopyConstructorSOO)
{
  SimpleString s1("hello");
  SimpleString s2(s1);
  EXPECT_EQ(s1.size(), s2.size());
  EXPECT_STREQ(s1.data(), s2.data());
  EXPECT_TRUE(s1.isSOO());
  EXPECT_TRUE(s2.isSOO());
}

// Test copy constructor with non-SOO string
TEST(SimpleStringTest, CopyConstructorNonSOO)
{
  std::string longStr(50, 'a');
  SimpleString s1(longStr);
  SimpleString s2(s1);
  EXPECT_EQ(s1.size(), s2.size());
  EXPECT_STREQ(s1.data(), s2.data());
  EXPECT_FALSE(s1.isSOO());
  EXPECT_FALSE(s2.isSOO());
  // Ensure they have independent storage
  EXPECT_NE(s1.data(), s2.data());
}

// Test copy assignment with SOO string
TEST(SimpleStringTest, CopyAssignmentSOO)
{
  SimpleString s1("hello");
  SimpleString s2("world");
  s2 = s1;
  EXPECT_EQ(s1.size(), s2.size());
  EXPECT_STREQ(s1.data(), s2.data());
}

// Test copy assignment with default constructed target
TEST(SimpleStringTest, CopyAssignmentToDefault)
{
  SimpleString s1("hello");
  SimpleString s2;
  s2 = s1;
  EXPECT_EQ(s1.size(), s2.size());
  EXPECT_STREQ(s1.data(), s2.data());
}

// Test self copy assignment
TEST(SimpleStringTest, SelfCopyAssignment)
{
  SimpleString s("test");
  s = s;
  EXPECT_EQ(s.size(), 4);
  EXPECT_STREQ(s.data(), "test");
}

// Test copy assignment with non-SOO string
TEST(SimpleStringTest, CopyAssignmentNonSOO)
{
  std::string longStr(50, 'b');
  SimpleString s1(longStr);
  SimpleString s2("temp");
  s2 = s1;
  EXPECT_EQ(s1.size(), s2.size());
  EXPECT_STREQ(s1.data(), s2.data());
  EXPECT_NE(s1.data(), s2.data());
}

// Test move constructor
TEST(SimpleStringTest, MoveConstructor)
{
  SimpleString s1("hello");
  SimpleString s2(std::move(s1));
  EXPECT_EQ(s2.size(), 5);
  EXPECT_STREQ(s2.data(), "hello");
}

// Test move constructor with non-SOO string
TEST(SimpleStringTest, MoveConstructorNonSOO)
{
  std::string longStr(50, 'c');
  SimpleString s1(longStr);
  const char* originalData = s1.data();
  SimpleString s2(std::move(s1));
  EXPECT_EQ(s2.size(), 50);
  EXPECT_EQ(s2.toStdString(), longStr);
  // After move, s2 should have taken over the pointer
  EXPECT_EQ(s2.data(), originalData);
}

// Test move assignment
TEST(SimpleStringTest, MoveAssignment)
{
  SimpleString s1("hello");
  SimpleString s2("world");
  s2 = std::move(s1);
  EXPECT_EQ(s2.size(), 5);
  EXPECT_STREQ(s2.data(), "hello");
}

// Test move assignment to default constructed
TEST(SimpleStringTest, MoveAssignmentToDefault)
{
  SimpleString s1("hello");
  SimpleString s2;
  s2 = std::move(s1);
  EXPECT_EQ(s2.size(), 5);
  EXPECT_STREQ(s2.data(), "hello");
}

// Test self move assignment
TEST(SimpleStringTest, SelfMoveAssignment)
{
  SimpleString s("test");
  s = std::move(s);
  EXPECT_EQ(s.size(), 4);
  EXPECT_STREQ(s.data(), "test");
}

// Test move assignment with non-SOO string
TEST(SimpleStringTest, MoveAssignmentNonSOO)
{
  std::string longStr(50, 'd');
  SimpleString s1(longStr);
  const char* originalData = s1.data();
  SimpleString s2("temp");
  s2 = std::move(s1);
  EXPECT_EQ(s2.size(), 50);
  EXPECT_EQ(s2.toStdString(), longStr);
  EXPECT_EQ(s2.data(), originalData);
}

// Test toStdString()
TEST(SimpleStringTest, ToStdString)
{
  SimpleString s("convert me");
  std::string str = s.toStdString();
  EXPECT_EQ(str, "convert me");
}

// Test toStdString() with empty string
TEST(SimpleStringTest, ToStdStringEmpty)
{
  SimpleString s;
  std::string str = s.toStdString();
  EXPECT_TRUE(str.empty());
}

// Test toStdStringView()
TEST(SimpleStringTest, ToStdStringView)
{
  SimpleString s("view me");
  std::string_view sv = s.toStdStringView();
  EXPECT_EQ(sv, "view me");
}

// Test toStdStringView() with empty string
TEST(SimpleStringTest, ToStdStringViewEmpty)
{
  SimpleString s;
  std::string_view sv = s.toStdStringView();
  EXPECT_TRUE(sv.empty());
}

// Test equality operator
TEST(SimpleStringTest, EqualityOperator)
{
  SimpleString s1("hello");
  SimpleString s2("hello");
  SimpleString s3("world");
  SimpleString s4("hell");

  EXPECT_TRUE(s1 == s2);
  EXPECT_FALSE(s1 == s3);
  EXPECT_FALSE(s1 == s4);
}

// Test inequality operator
TEST(SimpleStringTest, InequalityOperator)
{
  SimpleString s1("hello");
  SimpleString s2("hello");
  SimpleString s3("world");

  EXPECT_FALSE(s1 != s2);
  EXPECT_TRUE(s1 != s3);
}

// Test less than operator
TEST(SimpleStringTest, LessThanOperator)
{
  SimpleString s1("apple");
  SimpleString s2("banana");
  SimpleString s3("apple");
  SimpleString s4("app");

  EXPECT_TRUE(s1 < s2);
  EXPECT_FALSE(s2 < s1);
  EXPECT_FALSE(s1 < s3);
  EXPECT_FALSE(s1 < s4);  // "apple" > "app"
  EXPECT_TRUE(s4 < s1);   // "app" < "apple"
}

// Test greater than operator
TEST(SimpleStringTest, GreaterThanOperator)
{
  SimpleString s1("banana");
  SimpleString s2("apple");
  SimpleString s3("banana");
  SimpleString s4("ban");

  EXPECT_TRUE(s1 > s2);
  EXPECT_FALSE(s2 > s1);
  EXPECT_FALSE(s1 > s3);
  EXPECT_TRUE(s1 > s4);   // "banana" > "ban"
  EXPECT_FALSE(s4 > s1);  // "ban" < "banana"
}

// Test less than or equal operator
TEST(SimpleStringTest, LessEqualOperator)
{
  SimpleString s1("apple");
  SimpleString s2("banana");
  SimpleString s3("apple");

  EXPECT_TRUE(s1 <= s2);
  EXPECT_TRUE(s1 <= s3);
  EXPECT_FALSE(s2 <= s1);
}

// Test greater than or equal operator
TEST(SimpleStringTest, GreaterEqualOperator)
{
  SimpleString s1("banana");
  SimpleString s2("apple");
  SimpleString s3("banana");

  EXPECT_TRUE(s1 >= s2);
  EXPECT_TRUE(s1 >= s3);
  EXPECT_FALSE(s2 >= s1);
}

// Test comparison with non-SOO strings
TEST(SimpleStringTest, ComparisonNonSOO)
{
  std::string longStr1(50, 'a');
  std::string longStr2(50, 'b');
  std::string longStr3(50, 'a');

  SimpleString s1(longStr1);
  SimpleString s2(longStr2);
  SimpleString s3(longStr3);

  EXPECT_TRUE(s1 == s3);
  EXPECT_TRUE(s1 != s2);
  EXPECT_TRUE(s1 < s2);
  EXPECT_TRUE(s2 > s1);
  EXPECT_TRUE(s1 <= s3);
  EXPECT_TRUE(s1 >= s3);
}

// Test empty string comparisons
TEST(SimpleStringTest, EmptyStringComparison)
{
  SimpleString empty1;
  SimpleString empty2;
  SimpleString nonEmpty("a");

  EXPECT_TRUE(empty1 == empty2);
  EXPECT_FALSE(empty1 != empty2);
  EXPECT_TRUE(empty1 < nonEmpty);
  EXPECT_TRUE(nonEmpty > empty1);
  EXPECT_TRUE(empty1 <= nonEmpty);
  EXPECT_TRUE(nonEmpty >= empty1);
}

// Test that SimpleString size is as expected (16 bytes)
TEST(SimpleStringTest, SizeOfSimpleString)
{
  EXPECT_EQ(sizeof(SimpleString), 16);
}

// Test assignment from SOO to non-SOO
TEST(SimpleStringTest, AssignmentSOOToNonSOO)
{
  SimpleString s1("short");
  std::string longStr(50, 'x');
  SimpleString s2(longStr);

  s2 = s1;
  EXPECT_EQ(s2.size(), 5);
  EXPECT_STREQ(s2.data(), "short");
  EXPECT_TRUE(s2.isSOO());
}

// Test assignment from non-SOO to SOO
TEST(SimpleStringTest, AssignmentNonSOOToSOO)
{
  std::string longStr(50, 'y');
  SimpleString s1(longStr);
  SimpleString s2("tiny");

  s2 = s1;
  EXPECT_EQ(s2.size(), 50);
  EXPECT_EQ(s2.toStdString(), longStr);
  EXPECT_FALSE(s2.isSOO());
}

// Test very long string construction (non-SOO)
TEST(SimpleStringTest, VeryLongString)
{
  std::string veryLong(10000, 'z');
  SimpleString s(veryLong);
  EXPECT_EQ(s.size(), 10000);
  EXPECT_EQ(s.toStdString(), veryLong);
  EXPECT_FALSE(s.isSOO());
}

// Test reassignment from SOO to non-SOO
TEST(SimpleStringTest, ReassignSOOToNonSOO)
{
  SimpleString s("first");
  EXPECT_TRUE(s.isSOO());

  s = SimpleString("second value here");
  EXPECT_STREQ(s.data(), "second value here");
  EXPECT_FALSE(s.isSOO());
}

// Test reassignment from non-SOO to SOO
TEST(SimpleStringTest, ReassignNonSOOToSOO)
{
  SimpleString s("second value here");
  EXPECT_FALSE(s.isSOO());

  s = SimpleString("third");
  EXPECT_STREQ(s.data(), "third");
  EXPECT_TRUE(s.isSOO());
}

// Test reassignment from non-SOO to non-SOO
TEST(SimpleStringTest, ReassignNonSOOToNonSOO)
{
  std::string longStr1(50, 'a');
  std::string longStr2(100, 'b');

  SimpleString s(longStr1);
  EXPECT_FALSE(s.isSOO());

  s = SimpleString(longStr2);
  EXPECT_EQ(s.toStdString(), longStr2);
  EXPECT_FALSE(s.isSOO());
}

// Test construction from single character
TEST(SimpleStringTest, SingleCharacter)
{
  SimpleString s("a");
  EXPECT_EQ(s.size(), 1);
  EXPECT_STREQ(s.data(), "a");
  EXPECT_TRUE(s.isSOO());
}

// Test construction from exactly CAPACITY-1 chars
TEST(SimpleStringTest, CapacityMinus1)
{
  // 14 characters
  SimpleString s("12345678901234");
  EXPECT_EQ(s.size(), 14);
  EXPECT_STREQ(s.data(), "12345678901234");
  EXPECT_TRUE(s.isSOO());
}

// Test construction from exactly CAPACITY+1 chars
TEST(SimpleStringTest, CapacityPlus1)
{
  // 16 characters
  SimpleString s("1234567890123456");
  EXPECT_EQ(s.size(), 16);
  EXPECT_STREQ(s.data(), "1234567890123456");
  EXPECT_FALSE(s.isSOO());
}

// Test that data() returns null-terminated string for SOO
TEST(SimpleStringTest, NullTerminatedSOO)
{
  SimpleString s("test");
  const char* d = s.data();
  EXPECT_EQ(d[4], '\0');
}

// Test that data() returns null-terminated string for non-SOO
TEST(SimpleStringTest, NullTerminatedNonSOO)
{
  std::string longStr(50, 'x');
  SimpleString s(longStr);
  const char* d = s.data();
  EXPECT_EQ(d[50], '\0');
}

// Test copy of empty string
TEST(SimpleStringTest, CopyEmptyString)
{
  SimpleString s1;
  SimpleString s2(s1);
  EXPECT_EQ(s2.size(), 0);
  EXPECT_STREQ(s2.data(), "");
}

// Test move of empty string
TEST(SimpleStringTest, MoveEmptyString)
{
  SimpleString s1;
  SimpleString s2(std::move(s1));
  EXPECT_EQ(s2.size(), 0);
  EXPECT_STREQ(s2.data(), "");
}

// Test exception on size too large
TEST(SimpleStringTest, SizeTooLarge)
{
  const char* data = "test";
  // MAX_SIZE is 100MB, attempting to create larger should throw
  EXPECT_THROW(SimpleString(data, 200UL * 1024UL * 1024UL), std::invalid_argument);
}

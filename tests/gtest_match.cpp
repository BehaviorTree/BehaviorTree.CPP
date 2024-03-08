#include <gtest/gtest.h>
#include "wildcards/wildcards.hpp"

TEST(Match, Match)
{
  ASSERT_TRUE(wildcards::match("prefix/suffix", "*/suffix"));
  ASSERT_TRUE(wildcards::match("prefix/suffix", "prefix/*"));
  ASSERT_TRUE(wildcards::match("prefix/suffix", "pre*fix"));

  ASSERT_FALSE(wildcards::match("prefix/suffix", "*/suff"));
  ASSERT_FALSE(wildcards::match("prefix/suffix", "pre/*"));
  ASSERT_FALSE(wildcards::match("prefix/suffix", "pre*fi"));
  ASSERT_FALSE(wildcards::match("prefix/suffix", "re*fix"));
}

#include <gtest/gtest.h>
#include "behaviortree_cpp/utils/wildcards.hpp"

TEST(Match, Match)
{
  ASSERT_TRUE(wildcards_match("prefix/suffix", "*/suffix"));
  ASSERT_TRUE(wildcards_match("prefix/suffix", "prefix/*"));
  ASSERT_TRUE(wildcards_match("prefix/suffix", "pre*fix"));

  ASSERT_FALSE(wildcards_match("prefix/suffix", "*/suff"));
  ASSERT_FALSE(wildcards_match("prefix/suffix", "pre/*"));
  ASSERT_FALSE(wildcards_match("prefix/suffix", "pre*fi"));
  ASSERT_FALSE(wildcards_match("prefix/suffix", "re*fix"));
}

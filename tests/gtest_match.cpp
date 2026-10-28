#include "behaviortree_cpp/utils/wildcards.hpp"

#include <string>

#include <gtest/gtest.h>

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

TEST(Match, LongInputDoesNotExhaustStack)
{
  // A node path comes from the name attributes in the XML and a filter from a
  // substitution rule; neither has a length limit. The matcher used to recurse
  // once per character and crashed with a stack overflow on long inputs.
  const std::string long_name(1000000, 'a');
  const std::string path = "MainTree/" + long_name + "/Action";

  ASSERT_TRUE(wildcards_match(path, "*/Action"));
  ASSERT_TRUE(wildcards_match(path, "MainTree/*a?/Act*"));
  ASSERT_FALSE(wildcards_match(path, "*/Condition"));

  const std::string many_stars(1000000, '*');
  ASSERT_TRUE(wildcards_match(path, many_stars));
  ASSERT_FALSE(wildcards_match(path, many_stars + "x"));
}

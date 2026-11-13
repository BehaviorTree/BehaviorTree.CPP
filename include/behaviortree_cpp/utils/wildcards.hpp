#pragma once

#include <cstddef>
#include <string_view>

/**
 * @file wildcards.hpp
 * @brief Simple wildcard matching function supporting '*' and '?'.
 *
 * This file provides a function to match strings against patterns containing
 * wildcard characters:
 * - '*' matches any sequence of characters (including the empty sequence)
 * - '?' matches any single character
 *
 * The implementation is iterative and uses constant memory: when a mismatch
 * happens after a '*', only the most recent '*' needs to be retried, one
 * character further. Neither the recursion depth nor an allocation grows with
 * the length of the string or of the pattern, so a long node path or filter
 * cannot exhaust the stack.
 */

inline bool wildcards_match(std::string_view str, std::string_view pattern)
{
  const size_t n = str.size();
  const size_t m = pattern.size();

  size_t i = 0;
  size_t j = 0;
  // position of the last '*' in the pattern, and of the character in str
  // that it is currently expected to match up to
  size_t star = std::string_view::npos;
  size_t star_i = 0;

  while(i < n)
  {
    if(j < m && pattern[j] == '*')
    {
      star = j++;
      star_i = i;
    }
    else if(j < m && (pattern[j] == '?' || pattern[j] == str[i]))
    {
      ++i;
      ++j;
    }
    else if(star != std::string_view::npos)
    {
      // let the last '*' absorb one more character and retry from there
      j = star + 1;
      i = ++star_i;
    }
    else
    {
      return false;
    }
  }
  // the remaining part of the pattern must match the empty string
  while(j < m && pattern[j] == '*')
  {
    ++j;
  }
  return j == m;
}

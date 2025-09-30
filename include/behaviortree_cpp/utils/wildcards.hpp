#pragma once

/**
 * @file wildcards.hpp
 * @brief Simple wildcard matching function supporting '*' and '?'.
 *
 * This file provides a function to match strings against patterns containing
 * wildcard characters:
 * - '*' matches any sequence of characters (including the empty sequence)
 * - '?' matches any single character
 *
 * The implementation uses recursion with memoization to efficiently handle
 * overlapping subproblems.
 */

inline bool wildcards_match(std::string_view str, std::string_view pattern)
{
  const size_t n = str.size();
  const size_t m = pattern.size();

  // Pre-allocate memo table: -1 = not computed, 0 = false, 1 = true
  std::vector<int8_t> memo((n + 1) * (m + 1), -1);

  auto get_memo = [&](size_t i, size_t j) -> int8_t& { return memo[i * (m + 1) + j]; };

  auto match = [&](auto& match_ref, size_t i, size_t j) -> bool {
    if(j == m)
    {
      return i == n;
    }

    int8_t& cached = get_memo(i, j);
    if(cached != -1)
    {
      return cached == 1;
    }

    bool result;
    if(pattern[j] == '*')
    {
      result = match_ref(match_ref, i, j + 1);
      if(!result && i < n)
      {
        result = match_ref(match_ref, i + 1, j);
      }
    }
    else if(i < n && (pattern[j] == '?' || pattern[j] == str[i]))
    {
      result = match_ref(match_ref, i + 1, j + 1);
    }
    else
    {
      result = false;
    }

    cached = result ? 1 : 0;
    return result;
  };

  return match(match, 0, 0);
}

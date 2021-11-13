/******************************************************************************
 * @file lis.cpp
 * @author Jay Sharma
 * @brief Longest Increasing Subsequence Problem (Dynamic Programming)
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 *****************************************************************************/

#include "lis.h"

/******************************************************************************
 * @brief Given a sequence of integers,
 *        return the indices of the longest strictly increasing subsequence
 * 
 * @param sequence 
 * @return std::vector<unsigned> 
 *****************************************************************************/
std::vector<unsigned> 
longest_increasing_subsequence(std::vector<int> const& sequence) 
{
    // Allocated/reserve table, init all to 0 - including init case
    unsigned size = sequence.size();
    std::vector<unsigned> lis(size, 0);
    lis[0] = 1;

    // Initialize return vector and variables
    std::vector<unsigned> answer; //vector of indices corresponding to the LIS
    unsigned left = 0, right = 1; //pointer indices for traversing lis

    // Main logic - Populate lis
    for (unsigned i = 1; i < size; ++i)
    {
      lis[i] = 1;
      for (unsigned j = 0; j < i; ++j)
      {
        if (sequence[i] > sequence[j] && lis[i] < lis[j] + 1)
        {
          lis[i] = lis[j] + 1;
        }
      }
    }

    // Populate return vector
    for (; right < size; ++right)
    {
      if (lis[left] == lis[right])
      {
        left = right;
      }
      else if (lis[left] < lis[right])
      {
        answer.push_back(left);
        left = right;
      }
    }
    answer.push_back(left);

    return answer;
}

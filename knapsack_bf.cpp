/******************************************************************************
 * @file knapsack_bf.cpp
 * @author Jay Sharma
 * @brief Knapsack Optimization using Grey Code
 * @version 0.1
 * @date 2021-10-24
 * 
 * @copyright Copyright (c) 2021
 * 
 *****************************************************************************/
#include "knapsack_brute_force_minchange.h"
//#include <iostream>

#ifdef _MSC_VER
#include <intrin.h>
static inline int __builtin_ctzll(unsigned long long x) {
  unsigned long res;
  _BitScanForward(&res, x);
  return (int)res;
}
#endif

/******************************************************************************
 * @brief Constructor for Grey Code
 * 
 * @param s 
 *****************************************************************************/
GreyCode::GreyCode( int s ) : n(s), count(0), code(0,0), numbers(0,0), save(0,0)
{
  for (int i = 0; i < n; ++i)
  {
    code.push_back(0);
  }

  for (int i = 0; i < 1 << s; ++i)
  {
    numbers.push_back(i >> 1 ^ i);
  }

  for (int i = 0; i < 1 << s; ++i)
  {
    save.push_back(i >> 1 ^ i);
  }
}

/******************************************************************************
 * @brief Gets the next bit permutation
 * 
 * @return std::pair<bool, std::pair<bool, int>>
 *                     ^              ^     ^
 *                    last           add    pos
 *****************************************************************************/
std::pair< bool, std::pair< bool, int > > GreyCode::Next() 
{  
  // which position is modified (which item to change)
  int pos = __builtin_ctzll(++count);

  // is true if new value is 1 (add item), false otherwise 
  int index = n - 1 - pos;
  if (code[index] == 0)
  {
    code[index] = 1;
  }
  else
  {
    code[index] = 0;
  }
  bool add = code[index];

  // last subset is equal to 2 ^ (n-1)
  bool last;
  save.push_back(numbers[0]);
  numbers.pop_front();
  if (numbers[0] == pow(2, n-1))
  {
    last = true;
  }
  else
  {
    last = false;
  }

  return std::make_pair( !last, std::make_pair( add, pos ) );
}

/******************************************************************************
 * @brief Gets the code
 * 
 * @return std::vector<int>
 *****************************************************************************/
std::vector<int> GreyCode::GetCode()
{
  return code;
}

/******************************************************************************
 * @brief Gets the code
 * 
 * @return std::vector<int>
 *****************************************************************************/
std::vector<int> GreyCode::GetSave()
{
  return save;
}

/******************************************************************************
 * @brief Solve the knapsack problem given a list of items (each with a weight and a value)
 *  and a maximum weight threshold
 *
 * @param items
 * @param W
 * 
 * @return std::vector<int>
 *****************************************************************************/
////////////////////////////////////////////////////////////////////////////////
//  item has weight and value
//  W is the max weight
std::vector<bool> knapsack_brute_force( std::vector<Item> const& items, Weight const& W )
{
  std::vector<int> values;
  std::vector<bool> res;

  GreyCode gc(items.size());
  std::vector<int> numbers = gc.GetSave();

  Weight totalW;
  int totalV = 0;
  bool go = true;
  int largestVal = 0;
  int largestPos = 0;

  // Go through every permutation
  while (go)
  {
    std::pair<bool, std::pair<bool, int>> r = gc.Next();
    go = r.first;
    bool add = r.second.first;
    int pos = r.second.second;
    int index = items.size() - 1 - pos;
    std::vector<int> c = gc.GetCode();

    if (add)
    {
      totalW += items[index].GetWeight();
      totalV += items[index].GetValue();

      if (totalW > W)
      {
        values.push_back(0);
        continue;
      }
    }
    else
    {
      totalW -= items[index].GetWeight();
      totalV -= items[index].GetValue();

      if (totalW > W)
      {
        values.push_back(0);
        continue;
      }
    }

    values.push_back(totalV);
  }

  // Find the largest value in the vector, 
  // get its corresponding number from numbers
  for (size_t i = 0; i < values.size(); ++i)
  {
    if (values[i] > largestVal)
    {
      largestVal = values[i];
      largestPos = i;
    }
  }

  ++largestPos;

  // Convert that number to a bitset, iterate through it,
  // if the bit is 0, push back false, else true
  std::bitset<128> bits(numbers[largestPos]);

  //std::cout << bits << std::endl;

  for (size_t i = 0; i < bits.size(); ++i)
  {
    if (bits[bits.size() - 1 - i] == 0)
    {
      res.push_back(false);
    }
    else
    {
      res.push_back(true);
    }
  }

  // Truncate the leading zeroes
  res.erase(res.begin(), res.end() - items.size());

  return res;
}

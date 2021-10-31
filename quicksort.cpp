/******************************************************************************
 * @file quicksort.cpp
 * @author Jay Sharma
 * @brief Quick Sort Implementation
 * @version 0.1
 * @date 2021-10-27
 * 
 * @copyright Copyright (c) 2021
 * 
 *****************************************************************************/
#include "quicksort.h"

/******************************************************************************
 * @brief Swap Helper Function
 * 
 * @param a
 * @param b
 * @return void
 *****************************************************************************/
static void swap(int* a, int* b)
{
  int t = *a;
  *a = *b;
  *b = t;
}

/******************************************************************************
 * @brief Sets pivot as last element, sorts it, and moves everything smaller
 *  to the left and everything greater to the right
 * 
 * @param a
 * @param l
 * @param r
 * @return void
 *****************************************************************************/
static int partition (int* a, int l, int r)
{
  int start = a[l];
  int i = l;
  int j;

  for(j = l + 1; j < r; ++j)
  {
    if(a[j] <= start)
    {
      ++i;
      swap(&a[i], &a[j]);
    }
  }
  swap(&a[i], &a[l]);
  return i;
}

/******************************************************************************
 * @brief Conducts quick sort
 * 
 * @param a
 * @param l
 * @param r
 * @return void
 *****************************************************************************/
void quicksort(int* a, unsigned l, unsigned r)
{
  if (l < r)
  {
    // Partition index
    int pi = partition(a, l, r);

    // Before partition...
    quicksort(a, l, pi);
    // After partition...
    quicksort(a, pi + 1, r);
  }
}
/******************************************************************************
 * @file mergesort.cpp
 * @author Jay Sharma
 * @brief Merge Sort Optimization that uses a single memory allocation
 * @version 0.1
 * @date 2021-10-27
 * 
 * @copyright Copyright (c) 2021
 * 
 *****************************************************************************/
#include "mergesort.h"
#include <cstring>

/******************************************************************************
 * @brief Merges and sorts two arrays
 * 
 * @param start // pointer to the first element of source array
 * @param mid   // pointer to the mid element of source array
 * @param end   // pointer to the last element of source array
 * @param dest  // destination array to write
 * @return void
 *****************************************************************************/
static void merge(int* start, int* mid, int* end, int* dest) 
{
  // Initialize pointers
  int* startL = start;
  int* startR = mid;
  int* endL   = mid;
  int* endR   = end;

  // Sort
  while (startL < endL && startR < endR) 
  {
    *dest++ = *startL < *startR ? *startL++ : *startR++;
  }
  while (startL < endL)
  {
    *dest++ = *startL++;
  } 
  while (startR < endR)
  {
    *dest++ = *startR++;
  } 
}

/******************************************************************************
 * @brief Auxiliary recursive function for merge sort
 * 
 * @param src   // source array to read
 * @param dest  // destination array to write
 * @param len   // length of source array
 * @return void
 *****************************************************************************/
static void merge_rec(int* src, int* dest, unsigned len)
{
  if (len < 2) 
  {
    return;
  }

  // Call merge_rec on first half and second half
  unsigned l_len = len / 2;
  merge_rec(dest, src, l_len);
  merge_rec(dest + l_len, src + l_len, len - l_len);

  // Merge the two halves
  merge(src, src + l_len, src + len, dest);
}

/******************************************************************************
 * @brief Conducts merge sort
 * 
 * @param a   // destination array
 * @param r   // length of array
 * @return void
 *****************************************************************************/
void mergesort(int* a, unsigned r)
{
  if (r <= 1) 
  {
    return;
  }

  // Allocate array once, merge, then delete
  int* left = new int[r];
  memcpy(left, a, sizeof(int) * r);
  merge_rec(left, a, r);
  delete [] left;
}
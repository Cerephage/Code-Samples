/******************************************************************************
 * @file OAHashTable.cpp
 * @author Jay Sharma
 * @brief Implementation for a Hash Table class that utilizes Open Addressing
 *  (linear probing) collision resolution
 * @version 0.1
 * @date 2021-04-07
 * 
 * @copyright Copyright (c) 2021
 * 
 *****************************************************************************/

#include "OAHashTable.h"

/******************************************************************************
 * @brief Construct a new OAHashTable<T>::OAHashTable object
 * 
 * @tparam T 
 * @param Config 
 *****************************************************************************/
template<class T> OAHashTable<T>::
OAHashTable(const OAHashTable<T>::OAHTConfig &Config)
: config(Config)
{
  try
  {
    // Make the initial table
    table = new OAHTSlot[config.InitialTableSize_];
  }
  catch(const std::bad_alloc&)
  {
    throw (OAHashTableException(OAHashTableException::E_NO_MEMORY,
      "The table does not have enough memory"));
  }

  InitTable();
}

/******************************************************************************
 * @brief Destroy the OAHashTable<T>::OAHashTable object
 * 
 * @tparam T 
 *****************************************************************************/
template<class T> OAHashTable<T>::~OAHashTable()
{
  // Clear the table
  clear();

  // Delete the table
  delete [] table;
}

/******************************************************************************
 * @brief Inserts a Key/Data pair into the table. Throws an exception if the 
 * data cannot be inserted. (E_DUPLICATE, E_NO_MEMORY)
 * 
 * @tparam T 
 * @param Key 
 * @param Data 
 *****************************************************************************/
template<class T> void OAHashTable<T>::insert(const char *Key, const T &Data)
{
  if (((stats.Count_ + 1) / static_cast<double>(stats.TableSize_)) 
  > config.MaxLoadFactor_)
  {
    GrowTable();
  }

  // Place in hash table where this will be inserted
  unsigned PIndex = config.PrimaryHashFunc_(Key, stats.TableSize_);

  // Increment probe counter for initally finding place
  // in hash table
  ++stats.Probes_;

  // If collision occurs
  if (table[PIndex].State == OAHTSlot::OCCUPIED)
  {
    if (strncmp(Key, table[PIndex].Key, MAX_KEYLEN) == 0)
    {
      throw (OAHashTableException(OAHashTableException::E_DUPLICATE,
      "Item being inserted is a duplicate"));
    }

    // Get Sindex from secondary hash function
    unsigned SIndex = 1;
    if (config.SecondaryHashFunc_)
    {
      SIndex = config.SecondaryHashFunc_(Key, stats.TableSize_ - 1) + 1;
    }
    unsigned i = 1;
    unsigned targetIndex = 0;
    bool foundDeleted = false;

    for (; i < stats.TableSize_; ++i)
    {
      // Increment probe counter
      ++stats.Probes_;

      // Get newIndex
      unsigned newIndex = (PIndex + i * SIndex) % stats.TableSize_;

      // If duplicate is found, throw exception
      if (table[newIndex].State == OAHTSlot::OCCUPIED && strncmp(Key, table[newIndex].Key, MAX_KEYLEN) == 0)
      {
        throw (OAHashTableException(OAHashTableException::E_DUPLICATE,
        "Item being inserted is a duplicate"));
      }

      // Keep track of the first deleted slot
      if (table[newIndex].State == OAHTSlot::DELETED && !foundDeleted)
      {
        targetIndex = newIndex;
        foundDeleted = true;
      }

      // If no collision occurs, store the slot
      if (table[newIndex].State == OAHTSlot::UNOCCUPIED)
      {
        // If there was a deleted slot, insert the key there
        if (!foundDeleted)
        {
          targetIndex = newIndex;
        }
        break;
      }
    }
    PIndex = targetIndex;
  }

  if (table[PIndex].Key != Key)
  {
    strncpy(table[PIndex].Key, Key, MAX_KEYLEN - 1);
  }
  table[PIndex].Data = Data;
  table[PIndex].State = OAHTSlot::OCCUPIED;
  //table[PIndex].probes = stats.Probes_;

  ++stats.Count_;
}

/******************************************************************************
 * @brief Removes a Key/Data pair by key. Throws an exception if the pair 
 * cannot be removed. (E_ITEM_NOT_FOUND)
 * 
 * @tparam T 
 * @param Key 
 *****************************************************************************/
template<class T> void OAHashTable<T>::remove(const char *Key)
{
  // Initialize indices
  unsigned PIndex = stats.PrimaryHashFunc_(Key, stats.TableSize_);
  unsigned SIndex = 1;
  if (stats.SecondaryHashFunc_)
  {
    SIndex = stats.SecondaryHashFunc_(Key, stats.TableSize_ - 1) + 1;
  }
  unsigned i = 0;

  // Walk through the table until the end of the cluster is reached
  for (; i < stats.TableSize_; ++i)
  {
    OAHTSlot& slot = table[(PIndex + i * SIndex) % stats.TableSize_];

    ++stats.Probes_;

    if (slot.State == OAHTSlot::UNOCCUPIED)
    {
      break;
    }

    // If the key is found
    if (slot.State == OAHTSlot::OCCUPIED && strncmp(Key, slot.Key, MAX_KEYLEN) == 0)
    {
      --stats.Count_;

      if (config.FreeProc_)
      {
        config.FreeProc_(slot.Data);
      }

      if (config.DeletionPolicy_ == OAHTDeletionPolicy::MARK)
      {
        slot.State = OAHTSlot::DELETED;
      }
      else // PACK
      {
        slot.State = OAHTSlot::UNOCCUPIED;
        // Compress the table
        for (unsigned j = 1; j < stats.TableSize_; ++j) {
          OAHTSlot& slot2 = table[(PIndex + (i + j) * SIndex) % stats.TableSize_];
          if (slot2.State == OAHTSlot::OCCUPIED)
          {
            slot2.State = OAHTSlot::UNOCCUPIED;
            --stats.Count_;
            insert(slot2.Key, slot2.Data);
          }
          else 
          {
            break;
          }
        }
      }
      return;
    }
  }

  // If we made it to this point, then the item was not found
  throw(OAHashTableException(OAHashTableException::E_ITEM_NOT_FOUND,
    "Key not in table."));
}

/******************************************************************************
 * @brief Finds the data by key and returns a reference to the constant data. 
 * Throws an exception if Key isn't found. (E_ITEM_NOT_FOUND)
 * 
 * @tparam T 
 * @param Key 
 * @return const T& 
 *****************************************************************************/
template<class T> const T &OAHashTable<T>::find(const char *Key) const
{
  // Initialize indices
  unsigned PIndex = stats.PrimaryHashFunc_(Key, stats.TableSize_);
  unsigned SIndex = 1;
  if (stats.SecondaryHashFunc_)
  {
    SIndex = stats.SecondaryHashFunc_(Key, stats.TableSize_ - 1) + 1;
  }

  unsigned i;

  // Walk through the table until the key is found
  //for (unsigned i = PIndex; ; i = (i + SIndex) % stats.TableSize_)
  for (i = 0; i < stats.TableSize_; ++i)
  {
    OAHTSlot& slot = table[(PIndex + i * SIndex) % stats.TableSize_];
    ++stats.Probes_;
    if (slot.State == OAHTSlot::OCCUPIED && strncmp(Key, slot.Key, MAX_KEYLEN) == 0)
    {
      return slot.Data;
    }

    // If the slot is unoccupied, then the item does not exist
    if (slot.State == OAHTSlot::UNOCCUPIED)
    {
      break;
    }
  }

  throw(OAHashTableException(OAHashTableException::E_ITEM_NOT_FOUND,
    "Item not found in table."));
}

/******************************************************************************
 * @brief Removes all Key/Data pairs in the table, freeing the data if 
 * necessary. This does not free the hash table itself. If a FreeProc was 
 * provided, you must pass the data in each slot to that function and then 
 * set the slot as unoccupied.
 * 
 * @tparam T 
 *****************************************************************************/
template<class T> void OAHashTable<T>::clear()
{
  for (unsigned i = 0; i < stats.TableSize_; ++i)
  {
    if(config.FreeProc_ && table[i].State == OAHTSlot::OCCUPIED)
    {
      config.FreeProc_(table[i].Data);
    }
    table[i].State = OAHTSlot::UNOCCUPIED;
    //table[i].probes = 0;
  }
  stats.Count_ = 0;
}

/******************************************************************************
 * @brief Returns a struct that contains information on the status of the 
 * table for debugging and testing. The struct is defined in the header file.
 * 
 * @tparam T 
 * @return OAHTStats 
 *****************************************************************************/
template<class T> OAHTStats OAHashTable<T>::GetStats() const
{
  return stats;
}

/******************************************************************************
 * @brief Get the table
 * 
 * @tparam T 
 * @return const OAHashTable<T>::OAHTSlot* 
 *****************************************************************************/
template<class T> const typename OAHashTable<T>::OAHTSlot* 
OAHashTable<T>::GetTable() const
{
  return table;
}

///////////////////////////////////////////////////////////////////////////////
//--  HELPER FUNCTIONS  --/////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
 * @brief Helper function for initializing a table when it gets made
 * 
 * @tparam T 
 *****************************************************************************/
template<class T> void OAHashTable<T>::InitTable()
{
  // Initialize table
  for (unsigned i = 0; i < config.InitialTableSize_; ++i)
  {
    table[i].State = OAHTSlot::UNOCCUPIED;
    table[i].probes = 0;
  }

  // Store the initial stats from config
  stats.TableSize_ = config.InitialTableSize_;
  stats.Probes_ = 0;
  stats.Expansions_ = 0;
  stats.Count_ = 0;
  stats.PrimaryHashFunc_ = config.PrimaryHashFunc_;
  stats.SecondaryHashFunc_ = config.SecondaryHashFunc_;
}

/******************************************************************************
 * @brief Helper function for growing a table
 * 
 * @tparam T 
 *****************************************************************************/
template<class T> void OAHashTable<T>::GrowTable()
{
  // Record old table size
  unsigned old_table_size = stats.TableSize_;

  // Calculate new table size
  double factor = std::ceil(stats.TableSize_ * config.GrowthFactor_);
  unsigned new_table_size = GetClosestPrime(static_cast<unsigned>(factor));
  stats.TableSize_ = new_table_size;

  // Make the new table
  OAHTSlot* old_table = table;
  try
  {
    table = new OAHTSlot[stats.TableSize_];
  }
  catch(const std::bad_alloc&)
  {
    throw (OAHashTableException(OAHashTableException::E_NO_MEMORY,
      "The table does not have enough memory"));
  }
  
  stats.Count_ = 0;

  // Initialize table
  for (unsigned i = 0; i < stats.TableSize_; ++i)
  {
    table[i].State = OAHTSlot::UNOCCUPIED;
    table[i].probes = 0;
  }

  // Move slots over
  for (unsigned i = 0; i < old_table_size; ++i)
  {
    if (old_table[i].State == OAHTSlot::OCCUPIED)
    {
      insert(old_table[i].Key, old_table[i].Data);
    }
  }

  // Delete the old table and set the new one
  delete [] old_table;
  ++stats.Expansions_;
}
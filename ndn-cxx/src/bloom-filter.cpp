#include <cstdint>

#include "bloom-filter.hpp"
#include "murmur3.hpp"

#include "ns3/log.h"
NS_LOG_COMPONENT_DEFINE("ndn-cxx.BloomFilter");

using namespace std;

BloomFilter::BloomFilter()
      : m_numHashes(10)
      , m_bits(std::vector<bool>(64, false))
{
  
}

BloomFilter::BloomFilter(uint64_t size, uint8_t numHashes)
      : m_numHashes(numHashes)
      , m_bits(std::vector<bool>(size, false))
{
}

std::array<uint64_t,2> hashbf(const uint8_t *data, std::size_t len)
{
  std::array<uint64_t, 2> hashValue {0, 0};
  MurmurHash3_x64_128(data, len, 0, hashValue.data());
  return hashValue;
}

inline uint64_t nthHash(uint8_t n, uint64_t hashA, uint64_t hashB, uint64_t filterSize) 
{
    return (hashA + n * hashB) % filterSize;
}

void BloomFilter::add(uint64_t data) 
{
  std::array<uint8_t, 8> data_array {0, 0, 0, 0, 0, 0, 0, 0}
  for(int i = 0; i < 8; i++) {
    data_array[i] = ((data >> (i*8)) & 0x00FF);
  }
  auto hashValues = hashbf(data_array.data(), len);

  for (int n = 0; n < m_numHashes; n++) 
  {
      m_bits[nthHash(n, hashValues[0], hashValues[1], m_bits.size())] = true;
  }
}

bool BloomFilter::possiblyContains(uint64_t data) const 
{
  std::array<uint8_t, 8> data_array {0, 0, 0, 0, 0, 0, 0, 0}
  for(int i = 0; i < 8; i++) {
    data_array[i] = ((data >> (i*8)) & 0x00FF);
  }
  auto hashValues = hashbf(data_array.data(), len);

  for (int n = 0; n < m_numHashes; n++) 
  {
      if (!m_bits[nthHash(n, hashValues[0], hashValues[1], m_bits.size())]) 
      {
          return false;
      }
  }

  return true;
}

BloomFilter
BloomFilter::merge(BloomFilter b1, BloomFilter b2)
{
  //add merge of two ibfs


  std::vector<bool> ibf1 = b1.m_bits;
  std::vector<bool> ibf2 = b2.m_bits;

  uint64_t ibf1Size = ibf1.size();
  uint64_t ibf2Size = ibf2.size();

  BloomFilter mergedBF = BloomFilter(ibf1Size, b1.getNumHashes());
  
    if (ibf1Size != ibf2Size)
  {
    NS_LOG_INFO("Bloom filters to be compared are of different length");
    return false;
  }

  std::vector<bool>::iterator ibf1_it = ibf1.begin();
  std::vector<bool>::iterator ibf2_it = ibf2.begin();
  std::vector<bool>::iterator merged_it = ibf2.begin();

  for (;ibf1_it != ibf1.end() && ibf2_it != ibf2.end(); ++ibf1_it, ++ibf2_it, ++merged_it)
  {
    *merged_it = *ibf1_it || *ibf2_it;
  }

  return mergedBF;
}

uint64_t BloomFilter::getSize()
{
  return m_bits.size();
}

uint8_t BloomFilter::getNumHashes()
{
  return m_numHashes;
}

void BloomFilter::setNumHashes(uint8_t n)
{
  m_numHashes = n;
}

bool BloomFilter::matchLID(BloomFilter b)
{
  std::vector<bool> ibf = m_bits;
  std::vector<bool> lid = b.m_bits;

  uint64_t ibfSize = ibf.size();
  uint64_t lidSize = lid.size();

  if (ibfSize != lidSize)
  {
    NS_LOG_INFO("Bloom filters to be compared are of different length");
    return false;
  }

  std::vector<bool>::iterator ibf_it = ibf.begin();
  std::vector<bool>::iterator lid_it = lid.begin();

  for (;ibf_it != ibf.end() && lid_it != lid.end(); ++ibf_it, ++lid_it)
  {
    if (*ibf_it && *lid_it != *lid_it)
    {
      return false;
    }
  }

  return true;
}

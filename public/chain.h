#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <memory>

#include <cryptopp/keccak.h>
#include <boost/endian/conversion.hpp>

#include "ecc.h"
#include "mpz.h"
#include "mkl_tree.h"

inline G1 HashNameI(h256_t const& name, uint64_t i) {
  std::string s;
  s.reserve(64);
  s.assign((char*)name.data(), name.size());
  s += std::to_string(i);
  return MapToG1(s);
}

inline Fr Chain(uint8_t const* seed_buf, uint64_t seed_len, uint64_t index) {
  auto index_big = boost::endian::native_to_big(index);
  uint8_t digest[32];
  CryptoPP::Keccak_256 hash;  
  hash.Update(seed_buf, seed_len);  
  hash.Update((uint8_t const*)&index_big, sizeof(index_big));  
  hash.Final(digest);

  mpz_class z = MpzFromBE(digest, sizeof(digest));

  // setArrayMaskMod want little endian
  uint8_t digest2[32];
  MpzToLE(z, digest2, sizeof(digest2));

  Fr r;  
  r.setArrayMaskMod(digest2, sizeof(digest2));
  return r;
}

inline Fr Chain(mpz_class const& seed, uint64_t index) {
  mpz_class v = seed + index;

  uint8_t buf[32];
  MpzToBE(v, buf, sizeof(buf));

  uint8_t digest[32];
  CryptoPP::Keccak_256 hash;  
  hash.Update(buf, sizeof(buf));  
  hash.Final(digest);
  
  mpz_class z = MpzFromBE(digest, sizeof(digest));

  // setArrayMaskMod want little endian
  uint8_t digest2[32];
  MpzToLE(z, digest2, sizeof(digest2));

  Fr r;  
  r.setArrayMaskMod(digest2, sizeof(digest2));
  return r;
}

inline uint32_t ChainUint32(mpz_class const& seed, uint64_t index) {
  return (uint32_t)Chain(seed, index).getMpz().get_ui();
}

inline uint64_t ChainUint64(mpz_class const& seed, uint64_t index) {
  return (uint64_t)Chain(seed, index).getMpz().get_ui();
}

inline size_t PackGCount(size_t count) {
  assert(count);
  auto align_count = mkl::Pow2UB(count);
  if (align_count == 1) return 1;
  return align_count / 2;
}

inline size_t PackXCount(size_t count) { return mkl::Log2UB(PackGCount(count)); }
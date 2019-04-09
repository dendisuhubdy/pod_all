#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4101)
#pragma warning(disable : 4702)
#endif
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>
#include <yas/types/std/array.hpp>
#include <yas/types/std/pair.hpp>
#include <yas/types/std/string.hpp>
#include <yas/types/std/vector.hpp>
#include <yas/mem_streams.hpp>
#include <yas/file_streams.hpp>
#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/json_iarchive.hpp>
#include <yas/json_oarchive.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "basic_types.h"
#include "ecc.h"

// save
template<typename Ar>
void serialize(Ar &ar, Range const& t) {
  ar &YAS_OBJECT_NVP("Range", ("", t.start), ("", t.count));
}

// load
template<typename Ar>
void serialize(Ar &ar, Range &t) {
  ar &YAS_OBJECT_NVP("Range", ("", t.start), ("", t.count));
}

namespace mcl {
// save
template<typename Ar>
void serialize(Ar &ar, G1 const& t) {
  h256_t bin = G1ToBin(t);
  ar &YAS_OBJECT_NVP("G1", ("", bin));
}

// load
template<typename Ar>
void serialize(Ar &ar, G1 &t) {
  h256_t bin;
  ar &YAS_OBJECT_NVP("G1", ("", bin));
  t = BinToG1(bin.data()); // throw  
}

// save
template<typename Ar>
void serialize(Ar &ar, G2 const& t) {
  std::array<uint8_t, 64> bin;
  G2ToBin(t, bin.data());
  ar &YAS_OBJECT_NVP("G2", ("", bin));
}

// load
template<typename Ar>
void serialize(Ar &ar, G2 &t) {
  std::array<uint8_t, 64> bin;
  ar &YAS_OBJECT_NVP("G1", ("", bin));
  t = BinToG2(bin.data()); // throw  
}

// save
template<typename Ar>
void serialize(Ar &ar, Fr const& t) {
  h256_t bin = FrToBin(t);
  ar &YAS_OBJECT_NVP("Fr", ("", bin));
}

// load
template<typename Ar>
void serialize(Ar &ar, Fr &t) {
  h256_t bin;
  ar &YAS_OBJECT_NVP("Fr", ("", bin));
  t = BinToFr32(bin.data()); // throw
}
}

// TODO: vrf::Psk<>
#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_table_batch_protocol.h"

namespace scheme::table::batch {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("stb::Request", ("s", t.seed2_seed), ("d", t.demands));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("stb::Request", ("s", t.seed2_seed), ("d", t.demands));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("stb::Response", ("k", t.k), ("m", t.m));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("stb::Response", ("k", t.k), ("m", t.m));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  ar &YAS_OBJECT_NVP("stb::Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("stb::Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("stb::Secret", ("s", t.seed0));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("stb::Secret", ("s", t.seed0));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Claim const &t) {
  ar &YAS_OBJECT_NVP("stb::Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  ar &YAS_OBJECT_NVP("stb::Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
}
}  // namespace scheme::table::batch

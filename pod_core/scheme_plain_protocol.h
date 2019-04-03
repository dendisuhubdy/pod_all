#pragma once

#include <string>
#include <vector>

#include "basic_types.h"
#include "ecc.h"
#include "vrf.h"

namespace scheme::plain::range {
struct Request {
  h256_t seed2_seed;
  Range demand;
};

struct Response {
  std::vector<G1> k;
  std::vector<Fr> m;
};

struct Receipt {
  h256_t seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace scheme::plain::range

namespace scheme::plain::otrange {
struct NegoARequest {
  G2 s;
};

struct NegoAResponse {
  G2 s_exp_beta;
};

struct NegoBRequest {
  G1 t;
};

struct NegoBResponse {
  G1 t_exp_alpha;
};

struct Request {
  h256_t seed2_seed;
  Range phantom;          // = L
  std::vector<G1> ot_vi;  // sizeof() = K
  G1 ot_v;
};

struct Response {
  std::vector<G1> k;      // sizeof() = L
  std::vector<G1> ot_ui;  // sizeof() = K
  std::vector<Fr> m;      // sizeof() = L
};

struct Receipt {
  h256_t seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace scheme::plain::otrange

namespace scheme::plain::batch {
struct Request {
  h256_t seed2_seed;
  std::vector<Range> demands;
};

struct Response {
  std::vector<G1> k;
  std::vector<Fr> m;  // sizeof() = L
};

struct Receipt {
  h256_t seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace scheme::plain::batch

namespace scheme::plain::otbatch {
struct NegoARequest {
  G2 s;
};

struct NegoAResponse {
  G2 s_exp_beta;
};

struct NegoBRequest {
  G1 t;
};

struct NegoBResponse {
  G1 t_exp_alpha;
};

struct Request {
  h256_t seed2_seed;
  std::vector<Range> phantoms;  // sizeof() = L
  std::vector<G1> ot_vi;        // sizeof() = K
  G1 ot_v;
};

struct Response {
  std::vector<G1> k;      // sizeof() = L
  std::vector<G1> ot_ui;  // sizeof() = K
  std::vector<Fr> m;      // sizeof() = L
};

struct Receipt {
  h256_t seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace scheme::plain::otbatch

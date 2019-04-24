#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"
#include "scheme_table_a.h"
#include "scheme_table_batch3_protocol.h"
#include "scheme_table_batch3_misc.h"

namespace scheme::table::batch3 {

class Session {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Session(APtr a, h256_t const& self_id, h256_t const& peer_id);

 public:
  bool OnRequest(Request request, Response& response);
  bool OnReceipt(Receipt const& receipt, Secret& secret);

 private:
  void BuildMapping();
  void BuildK();
  void BuildX();
  void BuildUK(std::vector<std::vector<G1>>& uk);
  void BuildUX0(std::vector<G1>& ux0);
  void BuildU0X(std::vector<std::vector<G1>>& u0x);
  void BuildG2X0(std::vector<G2>& g2x0);
  void BuildM(std::vector<Fr>& encrypted_m);
  void BuildEK(std::vector<std::vector<Fr>>& ek);
  void BuildEX(std::vector<std::vector<Fr>>& ex);
  void BuildCommitmentD(std::vector<G1>& ud, G2& g2d);
  Fr const& GetK(uint64_t i, uint64_t j, uint64_t p);
  Fr const& GetX(uint64_t j, uint64_t p);

 private:
  APtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;
  Fr const d_;

 private:
  Request request_;
  uint64_t align_c_ = 0;
  uint64_t align_s_ = 0;
  uint64_t log_c_ = 0;
  uint64_t log_s_ = 0;
  std::vector<std::vector<Fr>> k_;
  std::vector<std::vector<Fr>> x_;
  G1 u0d_;
  G1 u0_x0_lgs_;
  Fr x0_lgs_;
  RomChallenge challenge_;

 private:
  struct Mapping {
    uint64_t index_of_m;
  };
  uint64_t demands_count_ = 0;
  std::vector<Mapping> mappings_;
};

typedef std::shared_ptr<Session> SessionPtr;
}  // namespace scheme::table::batch3
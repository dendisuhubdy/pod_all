#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_table_b.h"
#include "scheme_table_protocol.h"

namespace scheme::table::batch2 {

class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
         std::vector<Range> demands);

 public:
  void GetRequest(Request& request);
  bool OnResponse(Response response, Receipt& receipt);
  bool OnSecret(Secret const& secret);
  bool SaveDecrypted(std::string const& file);

 private:
  void BuildMapping();
  bool CheckEncryptedM();
  bool CheckKVW();
  void DecryptM(std::vector<Fr> const& v);

 private:
  BPtr b_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;
  std::vector<Range> const demands_;
  uint64_t demands_count_ = 0;
  h256_t seed2_seed_;

 private:
  std::vector<G1> k_;   // sizeof() = (count + 1) * s
  std::vector<Fr> vw_;  // sizeof() = s

 private:
  struct Mapping {
    uint64_t index_of_m;
  };
  std::vector<Mapping> mappings_;
  Fr sigma_vw_;

 private:
  h256_t seed2_;
  std::vector<Fr> w_;  // size() = count
  std::vector<Fr> decrypted_m_;
  std::vector<Fr> encrypted_m_;
};
}  // namespace scheme::table::batch2
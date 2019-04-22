#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_plain_b.h"
#include "scheme_plain_batch3_protocol.h"

namespace scheme::plain::batch3 {

class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
         std::vector<Range> demands);

 public:
  void GetRequest(Request& request);
  bool OnCommitment(Commitment commitment, Challenge& challenge);
  bool OnResponse(Response response, Receipt& receipt);
  bool OnSecret(Secret secret);
  bool SaveDecrypted(std::string const& file);

 private:
  void BuildMapping();
  bool CheckEncryptedM();
  bool CheckUX0();
  bool CheckEK();
  bool CheckEX();  
  void ComputeChallenge(h256_t const& r);
  bool CheckCommitmentOfD();
  void DecryptK();
  void DecryptX();
  void DecryptM();

 private:
  BPtr b_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;
  std::vector<Range> const demands_;
  h256_t const r_;

 private:
  uint64_t demands_count_ = 0;
  uint64_t align_c_;
  uint64_t align_s_;
  uint64_t log_c_;
  uint64_t log_s_;
  Fr c_;
  Fr e1_;
  Fr e2_;
  Fr e1_square_;
  Fr e2_square_;
  Fr e1_e2_inverse_;
  Commitment commitment_;
  Response response_;
  Receipt receipt_;
  Secret secret_;
  std::vector<std::vector<Fr>> k_;
  std::vector<std::vector<Fr>> x_;  

 private:
  struct Mapping {
    uint64_t index_of_m;
  };
  std::vector<Mapping> mappings_;

 private:
  std::vector<Fr> decrypted_m_;
  std::vector<Fr> encrypted_m_;
};

typedef std::shared_ptr<Client> ClientPtr;
}  // namespace scheme::plain::batch3
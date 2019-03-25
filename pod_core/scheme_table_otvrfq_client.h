#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_table_b.h"
#include "scheme_table_protocol.h"
#include "vrf.h"

namespace scheme::table {

struct VrfKeyMeta;

namespace otvrfq {
class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BPtr b, h256_t const& self_id, h256_t const& peer_id,
         std::string const& query_key,
         std::vector<std::string> const& query_values,
         std::vector<std::string> const& phantoms);

 public:
  void GetNegoReqeust(NegoBRequest& request);
  bool OnNegoRequest(NegoARequest const& request, NegoAResponse& response);
  bool OnNegoResponse(NegoBResponse const& response);

 public:
  void GetRequest(Request& request);
  bool OnResponse(Response const& request, Receipt& receipt);
  bool OnSecret(Secret const& secret,
                std::vector<std::vector<uint64_t>>& positions);

 private:
  BPtr b_;
  h256_t const self_id_;
  h256_t const peer_id_;
  std::string const query_key_;
  std::vector<std::string> const query_values_;
  size_t const phantoms_count_;

 private:
  VrfKeyMeta const* vrf_key_;
  std::vector<h256_t> value_digests_;
  std::vector<h256_t> shuffled_value_digests_;
  G1 g_exp_r_;
  std::vector<G1> last_psk_exp_r_;
  std::vector<vrf::Fsk> fsk_;
  std::vector<size_t> shuffle_reference_;

 private:
  G1 ot_self_pk_;
  G2 ot_peer_pk_;
  G1 ot_sk_;
  Fr ot_beta_;
  Fr ot_rand_a_;
  Fr ot_rand_b_;
};
}  // namespace otvrfq

}  // namespace scheme::table
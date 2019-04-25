#include "c_api.h"

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../scheme_plain_a.h"
#include "../scheme_plain_b.h"
#include "../scheme_plain_otrange_client.h"
#include "../scheme_plain_otrange_session.h"
#include "../scheme_plain_otrange_serialize.h"
#include "../scheme_plain_range_serialize.h"
#include "../scheme_plain_range_client.h"
#include "../scheme_plain_range_session.h"
#include "../scheme_plain_batch3_serialize.h"
#include "../scheme_plain_batch3_client.h"
#include "../scheme_plain_batch3_session.h"
#include "../scheme_plain_otbatch3_client.h"
#include "../scheme_plain_otbatch3_serialize.h"
#include "../scheme_plain_otbatch3_session.h"
#include "ecc.h"
#include "ecc_pub.h"

#include "scheme_plain.inc"
#include "scheme_plain_range.inc"
#include "scheme_plain_otrange.inc"
#include "scheme_plain_batch3.inc"
#include "scheme_plain_otbatch3.inc"

extern "C" {
EXPORT handle_t E_PlainANew(char const* publish_path) {
  using namespace scheme::plain;
  try {
    auto p = new A(publish_path);
    AddA(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT handle_t E_PlainBNew(char const* bulletin_file,
                            char const* public_path) {
  using namespace scheme::plain;
  try {
    auto p = new B(bulletin_file, public_path);
    AddB(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainABulletin(handle_t h, plain_bulletin_t* bulletin) {
  using namespace scheme::plain;
  APtr a = GetAPtr(h);
  if (!a) return false;
  Bulletin const& v = a->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  bulletin->size = v.size;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  return true;
}

EXPORT bool E_PlainBBulletin(handle_t h, plain_bulletin_t* bulletin) {
  using namespace scheme::plain;
  BPtr b = GetBPtr(h);
  if (!b) return false;
  Bulletin const& v = b->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  bulletin->size = v.size;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  return true;
}

EXPORT bool E_PlainAFree(handle_t h) {
  using namespace scheme::plain;
  return DelA((A*)h);
}

EXPORT bool E_PlainBFree(handle_t h) {
  using namespace scheme::plain;
  return DelB((B*)h);
}
}  // extern "C"

// range
extern "C" {
EXPORT handle_t E_PlainRangeSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  APtr a = GetAPtr(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session(a, self_id, peer_id);
    AddSession(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainRangeSessionOnRequest(handle_t c_session,
                                         char const* request_file,
                                         char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    Response response;
    if (!session->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainRangeSessionOnReceipt(handle_t c_session,
                                         char const* receipt_file,
                                         char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!session->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainRangeSessionSetEvil(handle_t c_session) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_PlainRangeSessionFree(handle_t h) {
  using namespace scheme::plain::range;
  return DelSession((Session*)h);
}

EXPORT handle_t E_PlainRangeClientNew(handle_t c_b, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id,
                                      range_t c_demand) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  BPtr b = GetBPtr(c_b);
  if (!b) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);
  Range demand(c_demand.start, c_demand.count);

  try {
    auto p = new Client(b, self_id, peer_id, demand);
    AddClient(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainRangeClientGetRequest(handle_t c_client,
                                         char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Request request;
    client->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainRangeClientOnResponse(handle_t c_client,
                                         char const* response_file,
                                         char const* receipt_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainRangeClientOnSecret(handle_t c_client,
                                       char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return client->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainRangeClientGenerateClaim(handle_t c_client,
                                            char const* claim_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Claim claim;
    if (!client->GenerateClaim(claim)) return false;
    
    yas::file_ostream os(claim_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(claim);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainRangeClientSaveDecrypted(handle_t c_client,
                                            char const* file) {
  using namespace scheme::plain;
  using namespace scheme::plain::range;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainRangeClientFree(handle_t h) {
  using namespace scheme::plain::range;
  return DelClient((Client*)h);
}
}  // extern "C" range

// otrange
extern "C" {
EXPORT handle_t E_PlainOtRangeSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  APtr a = GetAPtr(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session(a, self_id, peer_id);
    AddSession(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainOtRangeSessionGetNegoRequest(handle_t c_session,
                                                char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    NegoARequest request;
    session->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeSessionOnNegoRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    NegoBRequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoBResponse response;
    if (!session->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeSessionOnNegoResponse(handle_t c_session,
                                                char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    NegoAResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    if (!session->OnNegoResponse(response)) return false;
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeSessionOnRequest(handle_t c_session,
                                           char const* request_file,
                                           char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    Response response;
    if (!session->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeSessionOnReceipt(handle_t c_session,
                                           char const* receipt_file,
                                           char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!session->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeSessionSetEvil(handle_t c_session) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_PlainOtRangeSessionFree(handle_t h) {
  using namespace scheme::plain::otrange;
  return DelSession((Session*)h);
}

EXPORT handle_t E_PlainOtRangeClientNew(handle_t c_b, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id,
                                        range_t c_demand, range_t c_phantom) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  BPtr b = GetBPtr(c_b);
  if (!b) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);
  Range demand(c_demand.start, c_demand.count);
  Range phantom(c_phantom.start, c_phantom.count);

  try {
    auto p = new Client(b, self_id, peer_id, demand, phantom);
    AddClient(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainOtRangeClientGetNegoRequest(handle_t c_client,
                                               char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    NegoBRequest request;
    client->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeClientOnNegoRequest(handle_t c_client,
                                              char const* request_file,
                                              char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    NegoARequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoAResponse response;
    if (!client->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeClientOnNegoResponse(handle_t c_client,
                                               char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    NegoBResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);
    return client->OnNegoResponse(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeClientGetRequest(handle_t c_client,
                                           char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Request request;
    client->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeClientOnResponse(handle_t c_client,
                                           char const* response_file,
                                           char const* receipt_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeClientOnSecret(handle_t c_client,
                                         char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return client->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeClientGenerateClaim(handle_t c_client,
                                              char const* claim_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Claim claim;
    if (!client->GenerateClaim(claim)) return false;
    
    yas::file_ostream os(claim_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(claim);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtRangeClientSaveDecrypted(handle_t c_client,
                                              char const* file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otrange;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainOtRangeClientFree(handle_t h) {
  using namespace scheme::plain::otrange;
  return DelClient((Client*)h);
}

}  // extern "C" otrange

// batch3
extern "C" {
EXPORT handle_t E_PlainBatch3SessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id) {
  using namespace scheme::plain;
  using namespace scheme::plain::batch3;
  APtr a = GetAPtr(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session(a, self_id, peer_id);
    AddSession(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainBatch3SessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::batch3;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    Response response;
    if (!session->OnRequest(request, response)) {
      std::cerr << "Session::OnRequest failed\n";
      return false;
    }

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return false;
  }

  return true;
}

EXPORT bool E_PlainBatch3SessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::batch3;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!session->OnReceipt(receipt, secret)) {
      std::cerr << "Session::OnReceipt failed\n";
      return false;
    }

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return false;
  }

  return true;
}

EXPORT bool E_PlainBatch3SessionFree(handle_t h) {
  using namespace scheme::plain::batch3;
  return DelSession((Session*)h);
}

EXPORT handle_t E_PlainBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count) {
  using namespace scheme::plain;
  using namespace scheme::plain::batch3;
  BPtr b = GetBPtr(c_b);
  if (!b) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<Range> demands(c_demand_count);
  for (uint64_t i = 0; i < c_demand_count; ++i) {
    demands[i].start = c_demand[i].start;
    demands[i].count = c_demand[i].count;
  }

  try {
    auto p = new Client(b, self_id, peer_id, std::move(demands));
    AddClient(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainBatch3ClientGetRequest(handle_t c_client,
                                          char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::batch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Request request;
    client->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainBatch3ClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::batch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(std::move(response), receipt)) {
      std::cerr << "Client::OnResponse failed\n";
      return false;
    }

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return false;
  }

  return true;
}

EXPORT bool E_PlainBatch3ClientOnSecret(handle_t c_client,
                                        char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::batch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return client->OnSecret(std::move(secret));
  } catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return false;
  }

  return true;
}

EXPORT bool E_PlainBatch3ClientSaveDecrypted(handle_t c_client,
                                             char const* file) {
  using namespace scheme::plain;
  using namespace scheme::plain::batch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainBatch3ClientFree(handle_t h) {
  using namespace scheme::plain::batch3;
  return DelClient((Client*)h);
}
}  // extern "C" batch3

// otbatch3
extern "C" {
EXPORT handle_t E_PlainOtBatch3SessionNew(handle_t c_a,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  APtr a = GetAPtr(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session(a, self_id, peer_id);
    AddSession(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainOtBatch3SessionGetNegoRequest(handle_t c_session,
                                                 char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    NegoARequest request;
    session->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3SessionOnNegoRequest(handle_t c_session,
                                                char const* request_file,
                                                char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    NegoBRequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoBResponse response;
    if (!session->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3SessionOnNegoResponse(handle_t c_session,
                                                 char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    NegoAResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    if (!session->OnNegoResponse(response)) return false;
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3SessionOnRequest(handle_t c_session,
                                            char const* request_file,
                                            char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    Response response;
    if (!session->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3SessionOnReceipt(handle_t c_session,
                                            char const* receipt_file,
                                            char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  SessionPtr session = GetSessionPtr(c_session);
  if (!session) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!session->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3SessionFree(handle_t h) {
  using namespace scheme::plain::otbatch3;
  return DelSession((Session*)h);
}

EXPORT handle_t E_PlainOtBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id,
                                         range_t const* c_demand,
                                         uint64_t c_demand_count,
                                         range_t const* c_phantom,
                                         uint64_t c_phantom_count) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  BPtr b = GetBPtr(c_b);
  if (!b) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<Range> demands(c_demand_count);
  for (uint64_t i = 0; i < c_demand_count; ++i) {
    demands[i].start = c_demand[i].start;
    demands[i].count = c_demand[i].count;
  }

  std::vector<Range> phantoms(c_phantom_count);
  for (uint64_t i = 0; i < c_phantom_count; ++i) {
    phantoms[i].start = c_phantom[i].start;
    phantoms[i].count = c_phantom[i].count;
  }

  try {
    auto p = new Client(b, self_id, peer_id, std::move(demands),
                        std::move(phantoms));
    AddClient(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainOtBatch3ClientGetNegoRequest(handle_t c_client,
                                                char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    NegoBRequest request;
    client->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3ClientOnNegoRequest(handle_t c_client,
                                               char const* request_file,
                                               char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    NegoARequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoAResponse response;
    if (!client->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3ClientOnNegoResponse(handle_t c_client,
                                                char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    NegoBResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);
    return client->OnNegoResponse(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3ClientGetRequest(handle_t c_client,
                                            char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Request request;
    client->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3ClientOnResponse(handle_t c_client,
                                            char const* response_file,
                                            char const* receipt_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3ClientOnSecret(handle_t c_client,
                                          char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return client->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtBatch3ClientSaveDecrypted(handle_t c_client,
                                               char const* file) {
  using namespace scheme::plain;
  using namespace scheme::plain::otbatch3;
  ClientPtr client = GetClientPtr(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainOtBatch3ClientFree(handle_t h) {
  using namespace scheme::plain::otbatch3;
  return DelClient((Client*)h);
}
}  // extern "C" otbatch3
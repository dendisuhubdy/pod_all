#include "scheme_plain_otrange_test.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_b.h"
#include "scheme_plain_notary.h"
#include "scheme_plain_otrange_client.h"
#include "scheme_plain_otrange_session.h"
#include "scheme_plain_protocol.h"

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t{1};
const h256_t kDummyClientId = h256_t{2};
}  // namespace

namespace scheme::plain::otrange {

bool Test(std::string const& output_path, APtr a, BPtr b, Range const& demand,
          Range const& phantom, bool evil) {
  Tick _tick_(__FUNCTION__);

  auto output_file = output_path + "/decrypted_data";

  Session session(a, kDummySessionId, kDummyClientId);
  Client client(b, kDummyClientId, kDummySessionId, demand, phantom);
  if (evil) session.TestSetEvil();

  NegoBRequest b_nego_request;
  client.GetNegoReqeust(b_nego_request);
  NegoBResponse b_nego_response;
  if (!session.OnNegoRequest(b_nego_request, b_nego_response)) {
    assert(false);
    return false;
  }
  if (!client.OnNegoResponse(b_nego_response)) {
    assert(false);
    return false;
  }

  NegoARequest a_nego_request;
  session.GetNegoReqeust(a_nego_request);
  NegoAResponse a_nego_response;
  if (!client.OnNegoRequest(a_nego_request, a_nego_response)) {
    assert(false);
    return false;
  }
  if (!session.OnNegoResponse(a_nego_response)) {
    assert(false);
    return false;
  }

  Request request;
  client.GetRequest(request);

  Response response;
  if (!session.OnRequest(std::move(request), response)) {
    assert(false);
    return false;
  }

  Receipt receipt;
  if (!client.OnResponse(std::move(response), receipt)) {
    assert(false);
    return false;
  }

  Secret secret;
  if (!session.OnReceipt(receipt, secret)) {
    assert(false);
    return false;
  }

  if (!evil) {
    if (!client.OnSecret(secret)) {
      assert(false);
      return false;
    }

    if (!client.SaveDecrypted(output_file)) {
      assert(false);
      return false;
    }
  } else {
    if (client.OnSecret(secret)) {
      assert(false);
      return false;
    }
    Claim claim;
    if (!client.GenerateClaim(claim)) {
      assert(false);
      return false;
    }
    std::cout << "claim: " << claim.i << "," << claim.j << "\n";
    if (!VerifyClaim(a->bulletin().s, receipt, secret, claim)) {
      assert(false);
      return false;
    }
  }

  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          Range const& demand, Range const& phantom, bool test_evil) {
  try {
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);
    return Test(output_path, a, b, demand, phantom, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::plain::otrange
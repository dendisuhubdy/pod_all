#include "scheme_batch_test.h"
#include "scheme_batch_client.h"
#include "scheme_batch_notary.h"
#include "scheme_batch_protocol.h"
#include "scheme_batch_session.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_b.h"
#include "scheme_table.h"
#include "scheme_table_a.h"
#include "scheme_table_b.h"

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just use two dummy values for test.
const h256_t kDummySessionId = h256_t{1};
const h256_t kDummyClientId = h256_t{2};
}  // namespace

namespace scheme::batch {

template <typename A, typename B>
bool Test(std::string const& output_path, std::shared_ptr<A> a,
          std::shared_ptr<B> b, std::vector<Range> const& demands, bool evil) {
  Tick _tick_(__FUNCTION__);

  auto output_file = output_path + "/decrypted_data";

  Session session(a, kDummySessionId, kDummyClientId);
  Client client(b, kDummyClientId, kDummySessionId, demands);
  if (evil) session.TestSetEvil();

  Request request;
  client.GetRequest(request);

  Response response;
  if (!session.OnRequest(request, response)) {
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

    std::cout << "success: save to " << output_file << "\n";
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
    std::cout << "verify claim success\n";
  }

  return true;
}

}  // namespace scheme::batch

namespace scheme::plain::batch {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, bool test_evil) {
  try {
    using scheme::plain::A;
    using scheme::plain::B;
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);
    auto const& bulletin = b->bulletin();
    std::cout << "n: " << bulletin.n << ", s: " << bulletin.s
              << ", size: " << bulletin.size << "\n";
    return scheme::batch::Test(output_path, a, b, demands, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}
}  // namespace scheme::plain::batch

namespace scheme::table::batch {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, bool test_evil) {
  try {
    using scheme::table::A;
    using scheme::table::B;
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);
    auto const& bulletin = b->bulletin();
    std::cout << "n: " << bulletin.n << ", s: " << bulletin.s << "\n";
    return scheme::batch::Test(output_path, a, b, demands, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}
}  // namespace scheme::table::batch
#include "scheme_misc.h"

#include "basic_types.h"
#include "chain.h"
#include "ecc_pub.h"
#include "misc.h"
#include "mkl_tree.h"
#include "multiexp.h"
#include "public.h"

namespace scheme {

bool GetBulletinMode(std::string const& file, Mode& mode) {
  try {
    pt::ptree tree;
    pt::read_json(file, tree);
    auto str = tree.get<std::string>("mode");
    if (str == "table") {
      mode = Mode::kTable;
    } else if (str == "plain") {
      mode = Mode::kPlain;
    } else {
      return false;
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

void LoadMij(uint8_t const* data_start, uint8_t const* data_end, uint64_t i,
             uint64_t j, uint64_t s, Fr& mij) {
  auto offset = i * s + j;
  uint8_t const* p = data_start + offset * 31;
  uint8_t const* q = p + 31;
  if (p >= data_end) {
    mij = FrZero();
  } else {
    if (q > data_end) q = data_end;
    mij = BinToFr31(p, q);
  }
}

bool CopyData(std::string const& src, std::string const& dst) {
  try {
    io::mapped_file_params src_params;
    src_params.path = src;
    src_params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source src_view(src_params);

    io::mapped_file_params dst_params;
    dst_params.path = dst;
    dst_params.flags = io::mapped_file_base::readwrite;
    dst_params.new_file_size = src_view.size();
    io::mapped_file dst_view(dst_params);

    memcpy(dst_view.data(), src_view.data(), src_view.size());

    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveMkl(std::string const& output, std::vector<h256_t> const& mkl_tree) {
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = mkl_tree.size() * 32;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < mkl_tree.size(); ++i) {
      h256_t const& h = mkl_tree[i];
      memcpy(start + i * 32, h.data(), 32);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadMkl(std::string const& input, uint64_t n,
             std::vector<h256_t>& mkl_tree) {
  try {
    io::mapped_file_params params;
    params.path = input;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    auto tree_size = mkl::GetTreeSize(n);
    if (view.size() != tree_size * 32) {
      assert(false);
      return false;
    }
    mkl_tree.resize(tree_size);
    auto start = (uint8_t*)view.data();
    for (size_t i = 0; i < tree_size; ++i) {
      h256_t& h = mkl_tree[i];
      memcpy(h.data(), start + i * h.size(), h.size());
    }

    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

std::vector<G1> CalcSigma(std::vector<Fr> const& m, uint64_t n, uint64_t s) {
  assert(m.size() == n * s);

  auto const& ecc_pub = GetEccPub();

  auto const& u1 = ecc_pub.u1();
  std::vector<G1> sigmas(n);
#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)n; ++i) {
    G1& sigma = sigmas[i];
    auto is = i * s;
    if (s > 1024) {
      Fr const* mi0 = &m[is];
      sigma = MultiExpBdlo12(u1.data(), mi0, s);
    } else {
      sigma = G1Zero();
      for (uint64_t j = 0; j < s; ++j) {
        sigma += ecc_pub.PowerU1(j, m[is + j]);
      }
    }
  }
  return sigmas;
}

bool SaveSigma(std::string const& output, std::vector<G1> const& sigma) {
  Tick _tick_(__FUNCTION__);
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = sigma.size() * 32;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < sigma.size(); ++i) {
      G1ToBin(sigma[i], start + i * 32);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadSigma(std::string const& input, uint64_t n, h256_t const* root,
               std::vector<G1>& sigmas) {
  try {
    io::mapped_file_params params;
    params.path = input;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    if (view.size() != n * 32) return false;
    auto start = (uint8_t*)view.data();

    if (root) {
      auto get_sigma = [start, n](uint64_t i) -> h256_t {
        assert(i < n);
        h256_t h;
        memcpy(h.data(), start + i * 32, 32);
        return h;
      };
      if (*root != mkl::CalcRoot(std::move(get_sigma), n)) {
        assert(false);
        return false;
      }
    }

    sigmas.resize(n);
    for (size_t i = 0; i < n; ++i) {
      sigmas[i] = BinToG1(start + i * 32);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool SaveMatrix(std::string const& output, std::vector<Fr> const& m) {
  Tick _tick_(__FUNCTION__);
  try {
    io::mapped_file_params params;
    params.path = output;
    params.flags = io::mapped_file_base::readwrite;
    params.new_file_size = m.size() * 32;
    io::mapped_file view(params);
    uint8_t* start = (uint8_t*)view.data();
    for (size_t i = 0; i < m.size(); ++i) {
      FrToBin(m[i], start + i * 32);
    }
    return true;
  } catch (std::exception&) {
    assert(false);
    return false;
  }
}

bool LoadMatrix(std::string const& input, uint64_t ns, std::vector<Fr>& m) {
  try {
    io::mapped_file_params params;
    params.path = input;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    if (view.size() != 32 * ns) return false;

    auto start = (uint8_t*)view.data();
    m.resize(ns);
    for (uint64_t i = 0; i < m.size(); ++i) {
      if (!BinToFr32(start + i * 32, &m[i])) {
        assert(false);
        return false;
      }
    }
    return true;
  } catch (std::exception&) {
    return false;
  }
}

std::vector<h256_t> BuildSigmaMklTree(std::vector<G1> const& sigmas) {
  auto get_sigma = [&sigmas](uint64_t i) -> h256_t {
    return G1ToBin(sigmas[i]);
  };
  return mkl::BuildTree(sigmas.size(), get_sigma);
}

bool IsElementUnique(std::vector<Fr> const v) {
  std::vector<Fr const*> pv(v.size());
  for (size_t i = 0; i < v.size(); ++i) pv[i] = &v[i];

  auto compare = [](Fr const* a, Fr const* b) {
    return a->getMpz() < b->getMpz();
  };

  std::sort(pv.begin(), pv.end(), compare);

  return std::adjacent_find(pv.begin(), pv.end(), compare) == pv.end();
}

void H2(h256_t const& seed, uint64_t count, std::vector<Fr>& v) {
  Tick _tick_(__FUNCTION__);

  v.resize(count);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)count; ++i) {
    v[i] = Chain(seed, i);
  }
}

namespace {
h256_t KToH256(G1 const& g) {
  assert(g.isNormalized());

  const size_t kFpBufSize = 32;

  uint8_t x[32] = {0};
  if (g.z == 1) g.x.serialize(x, kFpBufSize);

  uint8_t y[32] = {0};
  if (g.z == 1) g.y.serialize(y, kFpBufSize);

  h256_t digest;
  CryptoPP::Keccak_256 hash;
  hash.Update(x, sizeof(x));
  hash.Update(y, sizeof(y));
  hash.Final(digest.data());
  return digest;
}
}  // namespace

// since we need to verify the mkl path in contract, we use plain G1
h256_t CalcRootOfK(std::vector<G1> const& k) {
  Tick _tick_(__FUNCTION__);
  auto get_k = [&k](uint64_t i) -> h256_t {
    assert(i < k.size());
    return KToH256(k[i]);
  };
  return mkl::CalcRoot(std::move(get_k), k.size());
}

// since we need to verify the mkl path in contract, we use plain G1
h256_t CalcPathOfK(std::vector<G1> const& k, uint64_t ij,
                   std::vector<h256_t>& path) {
  auto root = mkl::CalcPath(
      [&k](uint64_t i) -> h256_t {
        assert(i < k.size());
        return KToH256(k[i]);
      },
      k.size(), ij, &path);
  return root;
}

// since we need to verify the mkl path in contract, we use plain G1
bool VerifyPathOfK(G1 const& kij, uint64_t ij, uint64_t ns, h256_t const& root,
                   std::vector<h256_t> const& path) {
  h256_t k_bin = KToH256(kij);
  return mkl::VerifyPath(ij, k_bin, ns, root, path);
}

void BuildK(std::vector<Fr> const& v, std::vector<G1>& k, uint64_t s) {
  Tick _tick_(__FUNCTION__);

  assert(v.size() % s == 0);

  auto const& ecc_pub = GetEccPub();
  uint64_t n = v.size() / s;
  k.resize(v.size());

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)n; ++i) {
    for (int64_t j = 0; j < (int64_t)s; ++j) {
      auto offset = i * s + j;
      k[offset] = ecc_pub.PowerU1(j, v[offset]);
      k[offset].normalize();  // since we will serialize k (mkl root) later
    }
  }
}

h256_t CalcSeed2(h256_t const& seed, h256_t const& k_mkl_root) {
  h256_t ret;
  CryptoPP::Keccak_256 hash;
  hash.Update(seed.data(), seed.size());
  hash.Update(k_mkl_root.data(), k_mkl_root.size());
  hash.Final(ret.data());
  return ret;
}
}  // namespace scheme

namespace std {
std::istream& operator>>(std::istream& in, scheme::Mode& t) {
  std::string token;
  in >> token;
  if (token == "plain") {
    t = scheme::Mode::kPlain;
  } else if (token == "table") {
    t = scheme::Mode::kTable;
  } else {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

std::ostream& operator<<(std::ostream& os, scheme::Mode const& t) {
  if (t == scheme::Mode::kPlain) {
    os << "plain";
  } else if (t == scheme::Mode::kTable) {
    os << "table";
  } else {
    os.setstate(std::ios_base::failbit);
  }
  return os;
}

std::istream& operator>>(std::istream& in, scheme::Action& t) {
  std::string token;
  in >> token;
  if (token == "range_pod") {
    t = scheme::Action::kRangePod;
  } else if (token == "ot_range_pod") {
    t = scheme::Action::kOtRangePod;
  } else if (token == "vrf_query") {
    t = scheme::Action::kVrfQuery;
  } else if (token == "ot_vrf_query") {
    t = scheme::Action::kOtVrfQuery;
  } else if (token == "vrf_pod") {
    t = scheme::Action::kVrfPod;
  } else if (token == "ot_vrf_pod") {
    t = scheme::Action::kOtVrfPod;
  } else if (token == "batch_pod") {
    t = scheme::Action::kBatchPod;
  } else if (token == "ot_batch_pod") {
    t = scheme::Action::kOtBatchPod;
  } else if (token == "batch2_pod") {
    t = scheme::Action::kBatch2Pod;
  } else if (token == "batch3_pod") {
    t = scheme::Action::kBatch3Pod;
  } else if (token == "otbatch3_pod") {
    t = scheme::Action::kOtBatch3Pod;
  } else {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

std::ostream& operator<<(std::ostream& os, scheme::Action const& t) {
  if (t == scheme::Action::kRangePod) {
    os << "range_pod";
  } else if (t == scheme::Action::kOtRangePod) {
    os << "ot_range_pod";
  } else if (t == scheme::Action::kVrfQuery) {
    os << "vrf_query";
  } else if (t == scheme::Action::kOtVrfQuery) {
    os << "ot_vrf_query";
  } else if (t == scheme::Action::kVrfPod) {
    os << "vrf_pod";
  } else if (t == scheme::Action::kOtVrfPod) {
    os << "ot_vrf_pod";
  } else if (t == scheme::Action::kBatchPod) {
    os << "batch_pod";
  } else if (t == scheme::Action::kOtBatchPod) {
    os << "ot_batch_pod";
  } else if (t == scheme::Action::kBatch2Pod) {
    os << "batch2_pod";
  } else if (t == scheme::Action::kBatch3Pod) {
    os << "batch3_pod";
  } else if (t == scheme::Action::kOtBatch3Pod) {
    os << "otbatch3_pod";
  } else {
    os.setstate(std::ios_base::failbit);
  }
  return os;
}

std::istream& operator>>(std::istream& in, Range& t) {
  try {
    std::string token;
    in >> token;
    auto pos = token.find_first_of('-');
    if (pos != std::string::npos) {
      std::string s1 = token.substr(0, pos);
      std::string s2 = token.substr(pos + 1);
      t.start = boost::lexical_cast<uint64_t>(s1);
      t.count = boost::lexical_cast<uint64_t>(s2);
    } else {
      t.start = boost::lexical_cast<uint64_t>(token);
      t.count = 1;
    }
  } catch (std::exception&) {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

std::ostream& operator<<(std::ostream& os, Range const& t) {
  os << t.start << "-" << t.count;
  return os;
}

}  // namespace std
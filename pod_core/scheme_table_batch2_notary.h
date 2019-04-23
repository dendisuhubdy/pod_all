#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_table_batch2_protocol.h"

namespace scheme::table::batch2 {
bool VerifyProof(uint64_t s, Receipt const& receipt, Secret const& secret);
bool VerifyProof(uint64_t s, uint64_t count, Fr const& sigma_vw,
                 std::vector<Fr> const& v, std::vector<Fr> const& w);
}  // namespace scheme::table::batch2
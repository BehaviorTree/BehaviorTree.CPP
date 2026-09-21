#ifndef BTCPP_CONTRIB_JSON_HPP
#define BTCPP_CONTRIB_JSON_HPP

// BehaviorTree.CPP bundles a vendored copy of nlohmann::json
// (see nlohmann_json.hpp). Set USE_VENDORED_JSON=OFF to use an
// externally provided nlohmann_json package instead, so that the
// library and your application share the exact same JSON version
// and avoid ABI/link mismatches.

#ifdef BTCPP_USE_EXTERNAL_JSON
#include <nlohmann/json.hpp>
#else
#include "behaviortree_cpp/contrib/nlohmann_json.hpp"
#endif

#endif  // BTCPP_CONTRIB_JSON_HPP

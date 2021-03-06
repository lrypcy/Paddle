/* Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/fluid/framework/ir/fusion_group/code_generator_helper.h"
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include "glog/logging.h"
#include "paddle/fluid/framework/ir/fusion_group/operation.h"

namespace paddle {
namespace framework {
namespace ir {
namespace fusion_group {

template <typename T>
static T StringTo(const std::string& str) {
  std::istringstream is(str);
  T value;
  is >> value;
  return value;
}

std::string OperationExpression::GetRHS(size_t i) {
  auto rhs = OperationMap::Instance().Get(op_).exprs[i];
  for (size_t i = 0; i < rhs.size(); i++) {
    size_t pos = i;
    if (rhs[pos] == '$' && rhs[pos + 1] == '{') {
      int length = 0;
      while (rhs[pos + 2 + length] != '}') {
        length++;
      }
      std::string index_str = rhs.substr(pos + 2, length);
      int index = StringTo<int>(index_str);
      PADDLE_ENFORCE_LT(index, input_ids_.size(),
                        "Only %d inputs are provided, but need %d.",
                        input_ids_.size(), index + 1);
      rhs.replace(pos, length + 3, VarName(input_ids_[index]) + R"([idx])");
    }
  }
  return rhs;
}

std::string OperationExpression::GetLHS(size_t i) {
  std::stringstream ret;
  ret << VarName(output_ids_[i]) << R"([idx])";
  return ret.str();
}

bool OperationExpression::IsSupport() {
  return OperationMap::Instance().Has(op_);
}

// we Traverse the graph and get the group , all input id and output id is
// unique for the node which belong the group
std::string OperationExpression::GetExpression() {
  std::stringstream ret;
  if (IsSupport()) {
    for (size_t i = 0; i < output_ids_.size(); ++i) {
      ret << GetLHS(i) << " = " << GetRHS(i) << ";";
    }
  }

  return ret.str();
}

}  // namespace fusion_group
}  // namespace ir
}  // namespace framework
}  // namespace paddle

/*
 *  Copyright 2019 NVIDIA Corporation
 *  Modifications Copyright (c) 2020, Advanced Micro Devices, Inc.  All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

THRUST_BEGIN_NS
namespace hip_rocprim {

namespace detail {

    template<typename Size>
    struct make_unsigned_special;

    template<>
    struct make_unsigned_special<int> { typedef unsigned int type; };

    // this is special, because HIP's atomicAdd doesn't have an overload
    // for unsigned long, for some godforsaken reason
    template<>
    struct make_unsigned_special<long> { typedef unsigned long long type; };

    template<>
    struct make_unsigned_special<long long> { typedef unsigned long long type; };

}
}
THRUST_END_NS

/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file is part of zmqpp.
 * Copyright (c) 2011-2015 Contributors as noted in the AUTHORS file.
 */

#include "signal.hpp"

namespace std
{

    ostream &operator<<(ostream &s, const zmqpp::signal &sig)
    {
        s << static_cast<int64_t> (sig);
        return s;
    }
}
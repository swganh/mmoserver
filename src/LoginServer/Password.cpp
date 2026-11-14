/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "LoginServer/Password.h"

#include <sodium.h>

namespace password {

// libsodium's interactive limits: about 64 MB and a few tens of
// milliseconds per hash, for a login a player waits for.
static const unsigned long long kOps = crypto_pwhash_OPSLIMIT_INTERACTIVE;
static const size_t kMemory = crypto_pwhash_MEMLIMIT_INTERACTIVE;

void checkNothing(const std::string& text);

bool init()
{
    return sodium_init() >= 0;
}

std::string hash(const std::string& text)
{
    char out[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(out, text.data(), text.size(), kOps, kMemory) != 0) {
        return std::string();
    }
    return std::string(out);
}

bool isArgon2id(const std::string& stored)
{
    return stored.compare(0, 10, "$argon2id$") == 0;
}

Check check(const std::string& stored, const std::string& text, bool legacyMatch)
{
    if (isArgon2id(stored)) {
        if (stored.size() >= crypto_pwhash_STRBYTES ||
            crypto_pwhash_str_verify(stored.c_str(), text.data(), text.size()) != 0) {
            return Check::Wrong;
        }
        // Stronger limits than when it was stored: store it again.
        return crypto_pwhash_str_needs_rehash(stored.c_str(), kOps, kMemory) == 0 ? Check::Right : Check::RightRehash;
    }

    // An old unsalted SHA-1 password: compared by the database. A wrong one
    // spends a check's time too, or the quick refusal would tell which
    // usernames exist while accounts still hold SHA-1.
    if (!legacyMatch) {
        checkNothing(text);
        return Check::Wrong;
    }
    return Check::RightRehash;
}

void checkNothing(const std::string& text)
{
    static const std::string dummy = hash("no such account");
    if (!dummy.empty()) {
        (void)!crypto_pwhash_str_verify(dummy.c_str(), text.data(), text.size());
    }
}

}  // namespace password

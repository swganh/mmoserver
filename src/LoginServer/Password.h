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

#ifndef ANH_LOGINSERVER_PASSWORD_H
#define ANH_LOGINSERVER_PASSWORD_H

#include <string>

// Account passwords are stored as Argon2id hashes in libsodium's string form
// ("$argon2id$v=19$m=...,t=...,p=1$salt$hash"): salted, and slow to guess.
// Accounts made before that hold an unsalted SHA-1 of the password, which is
// accepted once and replaced by an Argon2id hash at that login.
namespace password {

// Readies libsodium; false if it cannot be used.
bool init();

// The Argon2id hash of text; empty if it could not be made (out of memory).
std::string hash(const std::string& text);

// Whether stored is an Argon2id hash (and not an old SHA-1 one).
bool isArgon2id(const std::string& stored);

enum class Check {
    Wrong,
    Right,
    RightRehash   // right, but stored in an old form: store hash(text)
};

// Checks text against the stored password. legacyMatch is the database's
// answer to "stored = SHA1(text)", used only for an old SHA-1 password.
Check check(const std::string& stored, const std::string& text, bool legacyMatch);

// Spends the time a check takes, for a login to an account that does not
// exist, so the reply time does not tell which usernames exist.
void checkNothing(const std::string& text);

}  // namespace password

#endif  // ANH_LOGINSERVER_PASSWORD_H

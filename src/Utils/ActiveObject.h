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

#ifndef SRC_UTILS_ACTIVEOBJECT_H_
#define SRC_UTILS_ACTIVEOBJECT_H_

#include <functional>
#include <memory>

#include "Utils/ConcurrentQueue.h"

namespace boost {
    class thread;
}


/// The utils namespace hosts a number of useful utility classes intended to
/// be used and reused in domain specific classes.
namespace utils {

/**
 * There are many times when it makes sense to break an object off and run it
 * concurrently while the rest of the application runs. The ActiveObject is a 
 * reusable facility that encourages the encapsulation of data by using asynchronus
 * messages to process requests in a private thread. This implementation is based
 * on a design discussed by Herb Sutter.
 *
 * @see http://www.drdobbs.com/go-parallel/article/showArticle.jhtml?articleID=225700095
 */
class ActiveObject {
public:
    /// Messages are implemented as std::function to allow maximum flexibility for
    /// how a message can be created with support for functions, functors, class members,
    /// and most importantly lambdas.
    typedef std::function<void()> Message;

public:
    /// Default constructor kicks off the private thread that listens for incoming messages.
    ActiveObject();

    /// Default destructor sends an end message and waits for the private thread to complete.
    ~ActiveObject();

    /**
     * Sends a message to be handled by the ActiveObject's private thread.
     *
     * \param Message The message to process on the private thread.
     */
    void Send(Message message);

private:
    ::utils::ConcurrentQueue<Message> message_queue_;
    std::unique_ptr<boost::thread> thread_;
    bool done_;


    /// Runs the ActiveObject's message loop until an end message is received.
    void Run();
};

}

#endif  // SRC_UTILS_ACTIVEOBJECT_H_
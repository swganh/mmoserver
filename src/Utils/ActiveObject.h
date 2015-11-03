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

#include <boost/thread.hpp>
#include <tbb/concurrent_queue.h>

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
 * Note that using an ActiveObject can result in up to 100% of the otherwise unused cpu
 * being consumed. This is intentional as ActiveObjects should primarily be targeted at
 * uses where being highly response is a priority.
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
     * \param message The message to process on the private thread.
     */
    void Send(Message message);

private:
    /// Runs the ActiveObject's message loop until an end message is received.
    void Run();

    tbb::concurrent_queue<Message> message_queue_;

    boost::thread thread_;
    boost::condition_variable condition_;
    boost::mutex mutex_;
    
    bool done_;
};

}

#endif  // SRC_UTILS_ACTIVEOBJECT_H_
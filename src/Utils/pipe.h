/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)
Copyright (c) 2015 Unofficial Hope Edition, lei
Use of this source code is governed by the GPL v3 license, see COPYING in project root.
---------------------------------------------------------------------------------------
Pipe Utility

For polling queues the wait method can be used as replacement for the sleep. If there
is work to do, a send after something pushes into the queue will then wake the wait.

To reflect load situations, wait returns the number of items in the pipe. So if wait
returns higher values than 1, the pushers put more into the queue as the worker can pop
in its cycle. The worker might use the return then to bypass the wait for the next
cycle.

Note: Due to the nature of pipes and select, each wait needs its own pipe.

Note: If methods are realized in cpp, this class would share them over program borders
      which may lead to issues. Instead of changing the cmake ruleset, they are simply
      implemented here.
---------------------------------------------------------------------------------------
*/

#ifndef ANH_PIPE_H
#define ANH_PIPE_H

#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "logger.h"
#include <Utils/logger.h>


namespace utils {

        using namespace std;

        class Pipe{
                public:
                        Pipe(){
                                if(pipe2(pipefd,O_NONBLOCK)<0){
                                        LOG(FATAL) << "Pipe::Pipe: failed to create pipe. Exit.";
                                               _ready = false;
                                        } else _ready = true;
                                }
                        ~Pipe(){
                                if(_ready) { close(pipefd[0]); close(pipefd[1]); }
                                _ready = false;
                                }

                int     wait(uint32 timeout_millis){
                                if(!_ready) return -1;

                                FD_ZERO(&fdset);
                                FD_SET(pipefd[0], &fdset);

                                timeout.tv_sec  = 0;
                                timeout.tv_nsec = timeout_millis * 1000;

                                sigemptyset(&sigmask);

                                int done=pselect(pipefd[0]+1, &fdset, NULL,NULL,&timeout,&sigmask);
                                if(done < 0){
                                        switch(errno){
                                                case  EINTR : LOG(ERR) << "Pipe::wait: pselect received signal.";
                                                        break;
                                                case  EBADF : LOG(FATAL) << "Pipe::wait: file descriptor invalid or not open,exit!";
                                                        break;
                                                case  EINVAL : LOG(FATAL) << "Pipe::wait: Parameter Error (fd negative or invalid timeout),exit.";
                                                        break;
                                                case  ENOMEM : LOG(FATAL) << "Pipe::wait: unable to allocate memory,exit.";
                                                        break;
                                                }
                                        //somebody else would assert here
                                        }

                                if (FD_ISSET(pipefd[0], &fdset)!=0) {
                                        char c;
                                        int ctr = 0;
                                        memset(buffer,0,sizeof(buffer));

                                        while(read(pipefd[0],&c,1)>0) {
                                                buffer[ctr++] = c;
                                                if(ctr>63) break;
                                                }
                                                buffer[ctr] = 0;
                                        return(ctr);
                                        }

                        return 0;
                        }

                void    send(char something){ if(_ready) write(pipefd[1],&something,1); }



                private:
                  Pipe(const Pipe&);
                  Pipe& operator=(const Pipe&);

                int             pipefd[2];
                fd_set          fdset;
                sigset_t        sigmask,oldmask;
                struct timespec timeout;
                char            buffer[64];
                bool            _ready;

        };

} // namespace

#endif





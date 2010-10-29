#!/bin/bash

#conn=`ps aux | grep -v grep | grep connserver`
#if [! conn]; then
    nohup ./src/ConnectionServer/connserver &
#fi
#
#chat=`ps aux | grep -v grep | grep chatserver`
#if [! chat]; then
    nohup ./src/ChatServer/chatserver &
#fi
#
#login=`ps aux | grep -v grep | grep loginserver`
#if [! login]; then
    nohup ./src/LoginServer/loginserver &
#fi
#
#ping=`ps aux | grep -v grep | grep pingserver`
#if [! ping]; then
    nohup ./src/PingServer/pingserver &
#fi
#
#tutorial=`ps aux | grep -v grep | grep tutorial`
#if [! tutorial]; then
#    nohup ./src/ZoneServer/zoneserver tutorial
#fi
#
#tatooine=`ps aux | grep -v grep | grep tatooine`
#if [! tatooine]; then
#    nohup ./src/ZoneServer/zoneserver tatooine &
#fi
clear
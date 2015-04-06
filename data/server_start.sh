#!/bin/bash

#conn=`ps aux | grep -v grep | grep connserver`
#if [! conn]; then
    nohup ./src/ConnectionServer/ConnectionServer &
#fi
#
#chat=`ps aux | grep -v grep | grep chatserver`
#if [! chat]; then
    nohup ./src/ChatServer/ChatServer &
#fi
#
#login=`ps aux | grep -v grep | grep loginserver`
#if [! login]; then
    nohup ./src/LoginServer/LoginServer &
#fi
#
#ping=`ps aux | grep -v grep | grep pingserver`
#if [! ping]; then
    nohup ./src/PingServer/PingServer &
#fi
#
#tutorial=`ps aux | grep -v grep | grep tutorial`
#if [! tutorial]; then
#    nohup ./src/ZoneServer/ZoneServer tutorial
#fi
#
#tatooine=`ps aux | grep -v grep | grep tatooine`
#if [! tatooine]; then
#    nohup ./src/ZoneServer/ZoneServer tatooine &
#fi
clear

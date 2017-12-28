#! /usr/bin/env python
#
# Example program using irc.client.
#
# This program is free without restrictions; do anything you like with
# it.
#
# Joel Rosdahl <joel@rosdahl.net>

import sys
import argparse

import irc.client

server = "chat.freenode.net"
target = "#musescore"
"The nick or channel to which to send messages"
port = 6667
nickname = "BuildServiceWin"
message = None


def on_connect(connection, event):
    if irc.client.is_channel(target):
        connection.join(target)
        return
    main_loop(connection)

def on_join(connection, event):
    main_loop(connection)

def main_loop(connection):
    connection.privmsg(target, message)
    connection.quit("Using irc.client.py")

def on_disconnect(connection, event):
    raise SystemExit()

def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('message')
    return parser.parse_args()

def main():
    global server
    global target
    global port
    global nickname
    global message

    args = get_args()
    message = args.message

    client = irc.client.Reactor()
    try:
        c = client.server().connect(server, port, nickname)
    except irc.client.ServerConnectionError:
        print(sys.exc_info()[1])
        raise SystemExit(0)

    c.add_global_handler("welcome", on_connect)
    c.add_global_handler("join", on_join)
    c.add_global_handler("disconnect", on_disconnect)

    client.process_forever()

if __name__ == '__main__':
    main()

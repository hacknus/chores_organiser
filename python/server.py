import logging
from websocket_server import WebsocketServer
import json
from chore import Chore, timedChore


def new_client(client, server):
    print("hello")
    server.send_message_to_all("Hey all, a new client has joined us")


def new_msg(client, server, message):
    print("new message")
    print(client)
    print(server)
    print(message)
    d = json.loads(message)
    print(d["linus"])
    print(d["johannes"])
    server.send_message_to_all("ok")


def init():
    file = open("state.json")
    d = json.load(file)
    chores = []
    timedChores = []
    for c in d["chores"]:
        chores.append(Chore(c["name"], c["period"], c["countdown"], c["who"]))
    for c in d["timedChores"]:
        timedChores.append(timedChore(c["name"], c["day"], c["n"], c["last"], c["who"]))


def main():
    # if new day
    # update all chores
    pass


init()

server = WebsocketServer(13254, host="0.0.0.0", loglevel=logging.INFO)
server.set_fn_new_client(new_client)
server.set_fn_message_received(new_msg)
server.run_forever()

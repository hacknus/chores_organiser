import logging
from websocket_server import WebsocketServer
import json
from chore import Chore
from datetime import datetime, timedelta
import operator


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


class Organiser:
    def __init__(self):
        file = open("state.json")
        d = json.load(file)
        self.chores = []
        for c in d["chores"]:
            next_date = datetime.now().date() + timedelta(days=c["countdown"])
            self.chores.append(Chore(c["name"], c["period"], next_date, c["who"]))
        for c in d["fixedChores"]:
            next_date = datetime.strptime(c["last"], '%d-%m-%Y').date()
            while next_date + timedelta(days=c["period"]) < datetime.now().date():
                next_date = next_date + timedelta(days=c["period"])
            self.chores.append(Chore(c["name"], c["period"], next_date, c["who"], True))

    def get_due_today(self):
        due_today = []
        for chore in self.chores:
            if chore.next_date <= datetime.today().date():
                print(f"{chore.name} is due today")
                due_today.append(chore)
        due_today = self.sort(due_today)
        return due_today

    def sort(self, chores):
        chores.sort(key=operator.attrgetter('next_date'))
        return chores


planner = Organiser()
due_today = planner.get_due_today()
print(due_today)

server = WebsocketServer(13254, host="0.0.0.0", loglevel=logging.INFO)
server.set_fn_new_client(new_client)
server.set_fn_message_received(new_msg)
server.run_forever()

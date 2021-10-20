import logging
from websocket_server import WebsocketServer
import json
from chore import Chore
from datetime import datetime, timedelta


class Server:

    def __init__(self):
        self.clients = []
        self.display = None
        self.server = WebsocketServer(13254, host="0.0.0.0", loglevel=logging.INFO)

    def new_client(self, client, server):
        # server.send_message(client, "OK")
        self.clients.append(client)

    def bye_client(self, client, server):
        self.clients.pop(client)

    def new_msg(self, client, server, message):
        print("new message")
        print(message)
        try:
            d = json.loads(message)
            if d["client"] == "display":
                self.display = client
                planner = Organiser()
                dt_linus, dt_johannes = planner.get_due_today()
                row1 = dt_linus[0]
                row2 = dt_johannes[0]
                row1 = "Es git nie z'phil collins"
                row2 = "genösis"
                server.send_message(client, row1)
                server.send_message(client, row2)
                return [d["linus"], d["johannes"]]
        except KeyError:
            return [0, 0]

    def main(self):
        self.server.set_fn_new_client(self.new_client)
        self.server.set_fn_message_received(self.new_msg)
        self.server.set_fn_client_left(self.bye_client)
        self.server.run_forever()


class Organiser:
    def __init__(self):
        file = open("state.json")
        d = json.load(file)
        self.chores = []
        for c in d["chores"]:
            next_date = datetime.strptime(c["last"], '%d-%m-%Y').date()
            self.chores.append(Chore(c["name"], c["period"], next_date, c["who"]))
        for c in d["fixedChores"]:
            next_date = datetime.strptime(c["last"], '%d-%m-%Y').date()
            while next_date + timedelta(days=c["period"]) < datetime.now().date():
                next_date = next_date + timedelta(days=c["period"])
            self.chores.append(Chore(c["name"], c["period"], next_date, c["who"], True))

    def get_due_today(self):
        due_today_linus = []
        due_today_johannes = []
        for chore in self.chores:
            if chore.next_date <= datetime.today().date():
                print(f"{chore.name} is due today")
                if chore.who == 1:
                    due_today_linus.append(chore)
                if chore.who == 0:
                    due_today_johannes.append(chore)
        due_today_linus.sort(key=lambda x: x.next_date)
        due_today_johannes.sort(key=lambda x: x.next_date)
        if len(due_today_johannes) == 0:
            due_today_johannes = [""]
        if len(due_today_linus) == 0:
            due_today_linus = [""]
        return due_today_linus, due_today_johannes


if __name__ == "__main__":
    socket = Server()
    socket.main()

import logging
from websocket_server import WebsocketServer
import json
from chore import Chore
from datetime import datetime, timedelta
import threading
import time


class Server:

    def __init__(self, organiser):
        self.clients = []
        self.display = None
        self.server = WebsocketServer(13254, host="0.0.0.0", loglevel=logging.INFO)
        self.organiser = organiser

    def new_client(self, client, server):
        # server.send_message(client, "OK")
        print("client connected")
        self.clients.append(client)
        if client["address"][0] == "192.168.178.35":
            print("display connected")
            self.display = client
            time.sleep(1)
            self.update_display()

    def bye_client(self, client, server):
        self.clients.remove(client)
        print("client left")

    def receive_msg(self, client, server, message):
        print("new message")
        print(message)
        try:
            d = json.loads(message)
            if d["client"] == "display":
                self.display = client
                self.update_display()
                self.organiser.done([d["johannes"], d["linus"]])
        except KeyError:
            pass

    def update_display(self):
        dt_linus, dt_johannes = self.organiser.get_due_today()
        if dt_linus[0] != "":
            row2 = dt_linus[0].name + (len(dt_linus) - 1) * "."
        else:
            row2 = " "
        if dt_johannes[0] != "":
            row1 = dt_johannes[0].name + (len(dt_johannes) - 1) * "."
        else:
            row1 = " "
        # row1 = "Hoi Simon"
        # row2 = "Misser Bresidend"
        message = row2 + "\n" + row1
        if self.display:
            print("updating display")
            print(f"sending message: {message}")
            self.server.send_message(self.display, message)
            print("updated display")

    def update(self):
        last_day = -1
        while True:
            now = datetime.now()
            day = now.day
            hour = now.hour
            if hour >= 4 and last_day != day:
                self.update_display()
                last_day = day
                time.sleep(60 * 60)

    def run_server(self):
        self.server.set_fn_new_client(self.new_client)
        self.server.set_fn_message_received(self.receive_msg)
        self.server.set_fn_client_left(self.bye_client)
        self.server.run_forever()


class Organiser:
    def __init__(self):
        file = open("state.json")
        d = json.load(file)
        self.due_today_linus = []
        self.due_today_johannes = []
        self.chores = []
        for c in d["chores"]:
            next_date = datetime.strptime(c["last"], '%d-%m-%Y').date()
            self.chores.append(Chore(c["name"], int(c["period"]), next_date, int(c["who"])))
        for c in d["fixedChores"]:
            next_date = datetime.strptime(c["last"], '%d-%m-%Y').date()
            while next_date + timedelta(days=int(c["period"])) < datetime.now().date():
                next_date = next_date + timedelta(days=int(c["period"]))
            self.chores.append(Chore(c["name"], int(c["period"]), next_date, int(c["who"]), True))

    def get_due_today(self):
        self.due_today_linus = []
        self.due_today_johannes = []
        for chore in self.chores:
            if chore.next_date <= datetime.today().date():
                print(f"{chore.name} is due today by {chore.who}")
                if chore.who == 1:
                    self.due_today_linus.append(chore)
                if chore.who == 0:
                    self.due_today_johannes.append(chore)
        self.due_today_linus.sort(key=lambda x: x.next_date)
        self.due_today_johannes.sort(key=lambda x: x.next_date)
        if len(self.due_today_johannes) == 0:
            self.due_today_johannes = [""]
        if len(self.due_today_linus) == 0:
            self.due_today_linus = [""]
        return self.due_today_linus, self.due_today_johannes

    def done(self, msg):
        johannes = int(msg[0])
        linus = int(msg[1])
        if linus == 1 and self.due_today_linus[0] != "":
            ind = self.chores.index(self.due_today_linus[0])
            self.chores[ind].done()
        if johannes == 1 and self.due_today_johannes[0] != "":
            ind = self.chores.index(self.due_today_johannes[0])
            self.chores[ind].done()
        d = {}
        chores = []
        fixedChores = []
        print("updating json")
        print(len(self.chores))
        for c in self.chores:
            print(c.name, c.next_date)
            if c.fixed:
                chores.append({
                    "name": c.name,
                    "last": c.next_date.strftime("%d-%m-%Y"),
                    "period": str(c.period),
                    "who": str(c.who)})
            else:
                fixedChores.append({
                    "name": c.name,
                    "last": c.next_date.strftime("%d-%m-%Y"),
                    "period": str(c.period),
                    "who": str(c.who)})
        d["chores"] = chores
        d["fixedChores"] = fixedChores
        print(d)
        with open('state.json', 'w') as f:
            json.dump(d, f)


if __name__ == "__main__":
    organiser = Organiser()
    socket = Server(organiser)
    x = threading.Thread(target=socket.run_server)
    y = threading.Thread(target=socket.update)
    x.start()
    y.start()

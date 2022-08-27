import logging
from websocket_server import WebsocketServer
import json
from chore import Chore
from datetime import datetime, timedelta, date
import threading
from threading import Event
from timedThread import timedThread
import time
import logging
import pandas as pd

logging.basicConfig(format='%(asctime)s %(message)s', filename='history.log',
                    level=logging.DEBUG, force=True)


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
                self.organiser.done([d["johannes"], d["linus"]])
                self.update_display()
        except KeyError:
            pass

    def update_display(self, msg=None, reset=False):
        chores_johannes_name = []
        chores_johannes_due = []
        chores_linus_name = []
        chores_linus_due = []
        for chore in self.organiser.chores:
            if chore.who == 1:
                chores_johannes_name.append(chore.name)
                d = (chore.next_date - date.today()).days
                chores_johannes_due.append(d)
            elif chore.who == 0:
                chores_linus_name.append(chore.name)
                d = (chore.next_date - date.today()).days
                chores_linus_due.append(d)
        johannes = {"name": chores_johannes_name,
                    "due": chores_johannes_due}
        linus = {"name": chores_linus_name,
                 "due": chores_linus_due}
        df_linus = pd.DataFrame(data=linus)
        df_linus.sort_values(by=['due'], inplace=True)
        df_johannes = pd.DataFrame(data=johannes)
        df_johannes.sort_values(by=['due'], inplace=True)
        df_linus.to_csv("linus.csv", index=False)
        df_johannes.to_csv("johannes.csv", index=False)

        if reset:
            self.organiser.shifted_johannes = 0
            self.organiser.shifted_linus = 0
        if msg == None: msg = self.organiser.disp_message
        if msg == None:
            dt_linus, dt_johannes = self.organiser.get_due_today()
            self.organiser.shifted_johannes = self.organiser.shifted_johannes % len(dt_johannes)
            self.organiser.shifted_linus = self.organiser.shifted_linus % len(dt_linus)
            if dt_linus[self.organiser.shifted_linus] != "":
                row2 = dt_linus[self.organiser.shifted_linus].name + (len(dt_linus) - 1) * "."
            else:
                row2 = " "
            if dt_johannes[self.organiser.shifted_johannes] != "":
                row1 = dt_johannes[self.organiser.shifted_johannes].name + (len(dt_johannes) - 1) * "."
            else:
                row1 = " "
        else:
            row1 = msg["row1"]
            row2 = msg["row2"]
        if row1 == " ":
            row1 = self.available_message("johannes")
        if row2 == " ":
            row2 = self.available_message("linus")
        message = row1 + "\n" + row2
        if self.display:
            print("updating display")
            print(f"sending message: {message}")
            self.server.send_message(self.display, message)
            print("updated display")

    def available_message(self, name):
        with open("message.json") as file:
            d = json.load(file)
        return d[name]

    def update(self, msg=None):
        now = datetime.now()
        self.update_display(msg=msg)
        return 1 * 60 * 60

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
        self.disp_message = None
        self.shifted_linus = 0
        self.shifted_johannes = 0
        logging.info("server started")
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
                if chore.who == 0:
                    self.due_today_linus.append(chore)
                if chore.who == 1:
                    self.due_today_johannes.append(chore)
        self.due_today_linus.sort(key=lambda x: x.next_date)
        newly_sorted = self.due_today_linus
        for chore in self.due_today_linus:
            if chore != "" and chore.fixed:
                self.due_today_linus.remove(chore)
                newly_sorted = [chore] + self.due_today_linus
                break
        self.due_today_linus = newly_sorted

        self.due_today_johannes.sort(key=lambda x: x.next_date)
        newly_sorted = self.due_today_johannes
        for chore in self.due_today_johannes:
            if chore != "" and chore.fixed:
                self.due_today_johannes.remove(chore)
                newly_sorted = [chore] + self.due_today_johannes
                break
        self.due_today_johannes = newly_sorted

        if len(self.due_today_johannes) == 0:
            self.due_today_johannes = [""]
        if len(self.due_today_linus) == 0:
            self.due_today_linus = [""]
        return self.due_today_linus, self.due_today_johannes

    def done(self, msg):
        johannes = int(msg[0])
        linus = int(msg[1])
        if linus == 1 and self.due_today_linus[self.shifted_linus] != "":
            ind = self.chores.index(self.due_today_linus[self.shifted_linus])
            logging.info(f'Linus has done {self.chores[ind].name}')
            self.chores[ind].done()
        elif linus == 2 and self.due_today_linus[self.shifted_linus] != "":
            self.shifted_linus += 1
        if johannes == 1 and self.due_today_johannes[self.shifted_johannes] != "":
            ind = self.chores.index(self.due_today_johannes[self.shifted_johannes])
            logging.info(f'Johannes has done {self.chores[ind].name}')
            self.chores[ind].done()
        elif johannes == 2 and self.due_today_johannes[self.shifted_johannes] != "":
            self.shifted_johannes += 1

        d = {}
        chores = []
        fixedChores = []
        print("updating json")
        print(len(self.chores))
        for c in self.chores:
            print(c.name, c.next_date)
            if not c.fixed:
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

    stopFlag = Event()
    thread = timedThread(stopFlag, func=socket.update)
    thread.start()
    x.start()

import calendar
from datetime import datetime

c = calendar.Calendar(firstweekday=calendar.SUNDAY)


class Chore:
    def __init__(self, name, period, countdown, who):
        self.name = name
        self.period = period
        self.countdown = countdown
        self.who = who

    def done(self):
        self.countdown = self.period
        self.who = 1 - self.who

    def update(self):
        self.countdown -= 1


class timedChore:
    def __init__(self, name, day, n, last, who):
        self.name = name
        self.day = day
        self.n = n - 1
        self.active = 0
        self.who = who
        self.who = last
        self.counter = 0

    def done(self):
        self.active = 0
        self.who = 1 - self.who

    def update(self):
        weekday = datetime.now().weekday
        if weekday == self.day:
            self.counter += 1
        if self.counter == self.n:
            self.active = 1
            self.counter = 0


if __name__ == "__main__":
    test = timedChore("test", 2, 3, 0)
    test.update()
    print(test.active)

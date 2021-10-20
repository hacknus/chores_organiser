import calendar
from datetime import datetime, timedelta

class Chore:
    def __init__(self, name, period, next_date, who, fixed=False):
        self.name = name
        self.period = period
        self.next_date = next_date
        self.who = who
        self.fixed = fixed

    def done(self):
        if self.fixed:
            self.next_date = self.next_date + timedelta(days=self.period)
        else:
            self.next_date = datetime.now() + timedelta(days=self.period)
        self.who = 1 - self.who


if __name__ == "__main__":
    test = Chore("test", 2, 3, 0)

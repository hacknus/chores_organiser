from threading import Thread


class timedThread(Thread):
    def __init__(self, event, func, timedelta=0):
        Thread.__init__(self)
        self.stopped = event
        self.func = func
        self.timedelta = timedelta

    def run(self):
        while not self.stopped.wait(self.timedelta):
            self.timedelta = self.func()

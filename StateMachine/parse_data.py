import time
import datetime
import socket
import asyncio
import matplotlib.pyplot as plt
import numpy as np
from sklearn import datasets
from sklearn import metrics
from sklearn.tree import DecisionTreeClassifier
from sklearn.datasets import load_svmlight_file
from sklearn.datasets import load_iris
from sklearn.model_selection import validation_curve
from sklearn.linear_model import Ridge
from sklearn.svm import SVC

plt.ion()

def clear_logfile(f_name):
    with open(f_name, "r+") as writer:
        writer.truncate(0)

class Sleeper:
    def __init__(self, duration, config_file):
        #TODO: add timescale functionality
        self.duration = int(duration)
        self.time_start = datetime.datetime.now()
        self.config_file = config_file

        self.dyn_x = []
        self.dyn_y = []

        f = open(config_file, "r")
        self.name = f.readline()
        self.wakeup_span = int(f.readline())    # float val of num minutes to perform the wakeup cycle
        self.predictive_max = f.readline()      # earliest wakeup where a state transition will be permitted
        self.num_divisions = int(f.readline())  # how many steps to break wakeup into, bounded [1, 100] on Windows and [1, 1000] on Unix due to clock limits (for now)
        self.port = int(f.readline())           # port for local communication to send control signals
        self.log_file = f.readline()[:-1]       # ouput logfile
        self.use_data = f.readline()            # whether or not to use predictive wakeup feature
        self.data_file = f.readline()           # (optional) dataset for predictive optimal wakeup
        self.sets = f.readline().split(';')     # (optional) additional datasets for prediction
        f.close()

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(('localhost', self.port))

        self.max = 0
        self.bk_avg_counter = 0
        self.bucket = []
        self.prev_buckets_avg = []
        self.state = "LIGHT "
        #self.prev_state = "LIGHT "
        for k in range(10):
            self.bucket.append(float(0))
            self.prev_buckets_avg.append(float(0))

        #self.clf = svm.SVC(gamma = 0.001, C = 100.0)
        #self.train_data = load_svmlight_file(self.data_file)                    # caveat: data must be of form line by line < [identifier] [feature-id]:[value] ... >
        #self.clf.fit(self.train_data.data[:-1], self.train_data.target[:-1])    # fit the datafile to a dataset which can now facilitate predictions
        
        # plt.xlabel("HRV")
        # plt.ylabel("Time (s)")
        # plt.show()

    def fill_bucket(self, pos, val):
        if val > self.max:
            self.max = val
        self.bucket[pos % 10] = val

    def get_bucket_avg(self):
        tmp = float(0)
        for i in range(10):
            tmp = tmp + self.bucket[i]
        tmp = tmp / 10
        return tmp

    def fill_bucket_avg(self, pos, val):
        if (pos % 10 == 0):
            self.prev_buckets_avg[self.bk_avg_counter % 10] = self.get_bucket_avg()
            self.bk_avg_counter = self.bk_avg_counter + 1

    def get_state(self, pos):
        n_count = int(0)
        my_sum = float(0)
        for ii in range(10):
            if (self.prev_buckets_avg[ii] != 0) and (ii != pos):
                my_sum = my_sum + self.prev_buckets_avg[ii]
                n_count = n_count + 1
        my_sum = my_sum / n_count
        if(self.prev_buckets_avg[pos % 10] > my_sum):
            if(self.prev_buckets_avg[pos % 10] > (my_sum * 1.0)):
               self.state = "LIGHT "
        else:
            if(self.prev_buckets_avg[pos % 10] < (my_sum * 1.0)):
               self.state = "DEEP "
        print(str(self.prev_buckets_avg[pos % 10]) + " " + str(my_sum))

    def log(self, line):
        with open(self.log_file, "a+") as writer:
            writer.write(str(line+"\n"))
        
    def graph_baseline(self):                   # graph historical dataset files using format
        self.fig, (self.ax, self.ax2) = plt.subplots(2)
        self.x = []
        self.y = []
        self.x_acc = []
        self.y_acc = []
        n_counted = []
        
        for i in range(2000):
            n_counted.append(int(0))
            #self.x.append(float(0))
            #self.y.append(float(0))
            #self.x_acc.append(float(0))
            #self.y_acc.append(float(0))
        
        for num in self.sets:
            print(num)
            # self.x = []
            # self.y = []
            fd = open(str("datasets/hrv_fake_" + num + ".txt"), "r")
            dataset =  fd.readlines()
            fd.close()
            lc = 0
            for line in dataset[:-1]:
                split_tupple = line.split(';')
                if len(self.x) > lc:            # average the datasets
                    n_counted[lc] = n_counted[lc] + 1
                    self.y[lc] = (float(self.y[lc]*(n_counted[lc]-1)) + float(split_tupple[1])) / n_counted[lc]
                    #("acc: "+str(self.y[lc]))
                else:
                    n_counted[lc] = 1
                    self.x.append(split_tupple[0])
                    self.y.append(split_tupple[1])
            #        print("append")
                lc = lc + 1
                
            # plt.plot(x, y, color = 'blue', linestyle = 'solid', linewidth = 2)
                    # marker = 'o', markerfacecolor = 'blue', markersize = 5)
            #self.ax.plot(self.x, self.y, color = 'blue', linestyle = 'solid', linewidth = 1)

            # self.x = []
            # self.y = []
            fd = open(str("datasets/acc_fake_"+ num + ".txt"), "r")
            dataset = fd.readlines()
            fd.close()
            lc = int(0)
            for line in dataset[:-1]:
                #print(line)
                split_tupple = line.split(';')
                split_tupple[1] = split_tupple[1].split('\n')[0]
                if len(self.x_acc) > lc:
                    n_counted[lc] = n_counted[lc] + 1
                    # self.x_acc[lc] = (self.x_acc[lc]*(n_counted[lc]-1) + split_tupple[0]) / n_counted[lc]
                    self.y_acc[lc] = (self.y_acc[lc]*int(n_counted[lc]-1) + float(split_tupple[1])) / n_counted[lc]
                else:
                    n_counted[lc] = 1
                    self.x_acc.append(split_tupple[0])
                    self.y_acc.append(float(split_tupple[1]))
                lc = lc + 1
                #print(str(lc) + " " + str(self.y_acc[lc]))
        print('-_-')
        print(str(len(self.x_acc)))
        print(str(len(self.y_acc)))
        print(str(len(self.x)))
        self.ax.set_autoscaley_on(True)
        self.ax.plot(self.x_acc, self.y_acc, color = 'blue', linestyle = 'solid', linewidth = 1)
        self.ax.tick_params(axis = 'y', labelcolor = 'blue')

        #self.ax3 = self.ax.twinx()
        #self.ax3.set_autoscaley_on(True)
        self.ax.plot(self.x, self.y, color = 'green', linestyle = 'solid', linewidth = 1)
        self.ax.tick_params(axis = 'y', labelcolor = 'green')
        self.ax.autoscale_view()
        self.ax.set_yticks(self.ax.get_yticks()[::10])
        self.ax.set_xticks(self.ax.get_xticks()[::100])
            
        plt.gcf().autofmt_xdate()
        plt.xlabel('Time')
        plt.ylabel('HRV(B) & ACC(G)')
        plt.title('Dynamic HRV Graph (Debug)')
            # plt.ion()
        plt.show()

    def send_com(self, msg):
        n_sent = 0
        n_max = len(msg)
        while n_sent < n_max:
            status = self.sock.send(msg[n_sent:])
            if status == 0:
                print("Socket dropped")
                self.sock.shutdown()
                return None
            n_sent = n_sent + status

    def update(self, data_file, regex):
        with open(data_file) as reader:
            new_data = reader.readlines()
            for line in new_data:
                split_tupple = line.split(regex)
                self.dyn_x.append(split_tupple[0])
                self.dyn_y.append(str(datetime.datetime.now()))
        self.ax3 = plt.subplots()
        self.ax3.plot(self.dyn_x, self.dyn_y, color = 'green', linestype = 'dashdot', linewidth = 1)
        self.ax3.set_ylabel('Live HRV')
        self.ax3.set_xticks([])                 # adjust later after tests
        plt.show()
    
    def simple(self):                           # simple wakeup with no real-time predictive analysis
        print("Sleep ", str(60*(self.duration - self.wakeup_span)))
        #time.sleep(60*(self.duration - self.wakeup_span))
        print(datetime.datetime.now())

        # n_hrv_per_interval = int(len(self.x) / self.duration)
        # it = int(0)
        
        for zz in range(self.num_divisions):
            self.send_com(str(datetime.datetime.now()).encode())   # send time for debug purposes, normally send a float on [0, 1] representing intensity
            time.sleep(1/self.num_divisions)
        self.send_com("WAKEUP".encode())

        y2 = []
        it = int(0);
        start = len(self.x) - int(float(float(self.wakeup_span) / float(self.duration)) * len(self.x))
        print("Start", start)
        for k in range(len(self.x)):
            if k < start:
                print('set 0')
                y2.append(0)
            else:
                print('set val')
                y2.append(it / self.num_divisions)
                it = it + 1
        self.ax4 = self.ax.twinx()
        self.ax4.plot(self.x, y2, color = 'red', linestyle = 'dashed', linewidth = 1)
        self.ax4.set_ylabel('Light Intensity')
        self.ax4.set_xticks([])
        # self.ax2.ylim(0, 1)
        # plt.figure(figsize = (13.3, 10))
        plt.rcParams['figure.figsize'] = [13.3, 10]
        plt.show()

    def sim(self, speedup, run_file):
        # Data points will be read at a rate of 1 * speedup per second
        #plt.ion()
        self.ax_int = self.ax2.twinx()

        print("In sim")

        sim_x = []
        sim_y = []

        # g1, ax3 = plt.subplots()
        linez, = self.ax2.plot([],[], color = 'blue', linewidth = 1)
        line_intensity, = self.ax_int.plot([],[], color = 'red', linestyle = 'dashed', linewidth = 1)
        self.ax_int.set_ylim([-0.025, 1])
        self.ax2.set_autoscaley_on(True)
        # self.ax_int.set_autoscaley_on(True)
        # self.ax_int.grid()
        self.ax2.grid()
        
        with open(run_file) as reader:
            data = reader.readlines()
            wk_start = len(data) - int(float(float(self.wakeup_span) / float(self.duration)) * len(data))
            print(wk_start)
            print(len(data))
            
            elapsed = 0
            intensity = 0
            for line in data[:-1]:
                time.sleep(1/speedup)
                elapsed = elapsed + 1
                split_tupple = line.split(';')
                split_tupple[1] = split_tupple[1][:-1]
                linez.set_xdata(np.append(linez.get_xdata(), float(elapsed)))
                linez.set_ydata(np.append(linez.get_ydata(), float(split_tupple[1])))

                self.fill_bucket(elapsed, float(split_tupple[1]))
                if (elapsed % 10 == 0):
                    self.fill_bucket_avg(elapsed, self.get_bucket_avg())
                    self.get_state(self.bk_avg_counter)
                    if (intensity / (len(data)-wk_start-1) <= 0):
                        self.log(self.state + str(intensity / (len(data)-wk_start-1)))

                if (intensity / (len(data)-wk_start-1) > 0):
                    self.log(self.state + str(100 * (intensity / (len(data)-wk_start-1)))[:6])

                line_intensity.set_xdata(np.append(line_intensity.get_xdata(), elapsed))
                if elapsed >= wk_start:
                    val = intensity / (len(data) - wk_start - 1)
                    line_intensity.set_ydata(np.append(line_intensity.get_ydata(), (intensity / (len(data) - wk_start - 1))))
                    print(intensity / (len(data) - wk_start))
                    intensity = intensity + 1
                    self.send_com(str("DEEP " + str(100 * val)).encode())
                else:
                    line_intensity.set_ydata(np.append(line_intensity.get_ydata(), 0))
                    
                self.ax_int.relim()
                self.ax_int.autoscale_view()
                self.ax2.relim()
                self.ax2.autoscale_view()
                self.fig.canvas.draw()
                self.fig.canvas.flush_events()
                
                if elapsed % 20 == 0 and elapsed < wk_start:
                    self.send_com(str("DEEP 0.0").encode())

                #print("Sim")
            #plt.show()
                
        
    def manage(self):                           # manage control signals from the ECG feeding to the State Machine while asleep
        uncertainty = 0.25                      # max relative difference between ss
    #    scale
        
if __name__ == '__main__':
    S1 = Sleeper(100, "sleep_config.txt")
    clear_logfile(S1.log_file)
    while True:
        print("Run Simulation? (y/n)")
        do_sim = str(input())
        S1.send_com("DONE 0".encode())
        time.sleep(0.25)
        #if do_sim != 'y' and do_sim != 'Y':
        S1.graph_baseline()
        # S1.simple()
        # S1.manage()
        S1.log(str("DONE 0"))
        if do_sim == 'y' or do_sim == 'Y':
            S1.sim(10000, "datasets/acc_fake_1.txt")
            S1.send_com(str(S1.state + "100.0").encode())
            S1.log(str(S1.state + "100.0"))
            time.sleep(0.25)
            S1.send_com("DONE 100.0".encode())
            S1.log("DONE 100.0")

import time
import datetime
import socket
import asyncio
import matplotlib.pyplot as plt
import numpy as np
import argparse
import select
import selectors
import sys
from sklearn import datasets
from sklearn import metrics
from sklearn.tree import DecisionTreeClassifier
from sklearn.datasets import load_svmlight_file
from sklearn.datasets import load_iris
from sklearn.model_selection import validation_curve
from sklearn.linear_model import Ridge
from sklearn.svm import SVC

plt.ion()
plt.rcParams['figure.figsize'] = (16, 9)

def clear_logfile(f_name):
    with open(f_name, "r+") as writer:
        writer.truncate(0)

class Sleeper:
    def __init__(self, duration, df, config_file):
        #TODO: add timescale functionality
        self.duration = int(duration)
        self.time_start = datetime.datetime.now()
        self.config_file = config_file

        self.dyn_x = []
        self.dyn_y = []

        f = open(config_file, "r")
        self.name = f.readline()[:-1]
        self.wakeup_span = int(f.readline())    # float val of num minutes to perform the wakeup cycle
        self.predictive_max = f.readline()      # earliest wakeup where a state transition will be permitted
        self.num_divisions = int(f.readline())  # how many steps to break wakeup into, bounded [1, 100] on Windows and [1, 1000] on Unix due to clock limits (for now)
        self.port = int(f.readline())           # port for local communication to send control signals
        self.log_file = f.readline()[:-1]       # ouput logfile
        self.use_data = f.readline()            # whether or not to use predictive wakeup feature
        self.data_file = f.readline()[:-1]      # (optional) dataset for predictive optimal wakeup
        self.sets = f.readline().split(';')     # (optional) additional datasets for prediction
        f.close()

        if (df != "__no__"):
            self.data_file = df

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect(('localhost', self.port))

        self.max = 0
        self.bk_avg_counter = 0
        self.bucket = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.prev_buckets_avg = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.state = "LIGHT "
        self.do_g = True
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

    def enable_graph(self):
        self.do_g = True

    def disable_graph(self):
        self.do_g = False

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
            for line in dataset[1:-1]:
                split_tupple = line.split(';')
                split_tupple[1] = split_tupple[1].split('\n')[0]
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
            for line in dataset[1:-1]:
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
        l1 = self.ax.plot(self.x_acc, self.y_acc, color = 'blue', linestyle = 'solid', linewidth = 1, label = 'Acceleration Data')
        self.ax.tick_params(axis = 'y', labelcolor = 'blue')

        self.ax3 = self.ax.twinx()
        
        self.ax3.set_autoscaley_on(True)
        l2 = self.ax3.plot(self.x, self.y, color = 'green', linestyle = 'solid', linewidth = 1, label = 'Heart Rate Variability Data')
        self.ax3.tick_params(axis = 'y', labelcolor = 'green')
        self.ax.autoscale_view()
        self.ax3.autoscale_view()
        #self.ax.set_yticks(self.ax.get_yticks()[::10])
        self.ax.set_xticks(self.ax.get_xticks()[::10])
        self.ax3.set_yticks(self.ax3.get_yticks()[::15])
        #self.ax3.set_xticks(self.ax3.get_xticks()[::100])

        t_lines = l1 + l2
        lbs = [ll.get_label() for ll in t_lines]
        self.ax.legend(t_lines, lbs, loc = 0)
        plt.gcf().autofmt_xdate()
        plt.xlabel('Time')
        #plt.ylabel('HRV(B) & ACC(G)')
        plt.title('HRV and ACC Composite Data')
        plt.show()

    def mdecode(self, msg):
        tmp = msg.rstrip('\n')
        tmp = msg.split(' ')
        return [int(tmp[0]), float(tmp[1])]
            

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

        predict_begin = False
        wake_begin = False

        if self.do_g == True:
            #print("Good")
            #time.sleep(10)
            self.ax_int = self.ax2.twinx()

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
        
        with open(self.data_file) as reader:
            data = reader.readlines()
            wk_start = len(data) - int(float(float(self.wakeup_span) / float(self.duration)) * len(data))
            print(wk_start)
            print(len(data))
            
            elapsed = 0
            intensity = 0
            for line in data[1:-1]:
                time.sleep(1/speedup)
                elapsed = elapsed + 1
                split_tupple = line.split(';')
                split_tupple[1] = split_tupple[1][:-1]
                print(split_tupple[1])
                #split_tupple[1] = split_tupple[1].split('\n')[0]

                if self.do_g == True:
                    linez.set_xdata(np.append(linez.get_xdata(), float(elapsed)))
                    linez.set_ydata(np.append(linez.get_ydata(), float(split_tupple[1])))

                self.fill_bucket(elapsed, float(split_tupple[1]))
                if (elapsed % 10 == 0):
                    self.fill_bucket_avg(elapsed, self.get_bucket_avg())
                    self.get_state(self.bk_avg_counter)
                    if (intensity / (len(data)-wk_start-1) <= 0):
                        self.log(self.state + str(intensity / (len(data)-wk_start-1)))
                        self.send_com(str(self.state + str(intensity / (len(data)-wk_start-1))).encode())

                if (intensity / (len(data)-wk_start-1) > 0):
                    self.log(self.state + str(100 * (intensity / (len(data)-wk_start-1)))[:6])
                    self.send_com(str(self.state + str(100 * (intensity / (len(data)-wk_start-1)))[:6]).encode())

                if self.do_g == True:
                    line_intensity.set_xdata(np.append(line_intensity.get_xdata(), elapsed))
                if elapsed >= wk_start:
                    val = intensity / (len(data) - wk_start - 1)
                    if self.do_g == True:
                        line_intensity.set_ydata(np.append(line_intensity.get_ydata(), (intensity / (len(data) - wk_start - 1))))
                    print(intensity / (len(data) - wk_start))
                    intensity = intensity + 1
                    #self.send_com(str("DEEP " + str(100 * val)).encode())
                else:
                    if self.do_g == True:
                        line_intensity.set_ydata(np.append(line_intensity.get_ydata(), 0))

                if self.do_g == True:
                    self.ax_int.relim()
                    self.ax_int.autoscale_view()
                    self.ax2.relim()
                    self.ax2.autoscale_view()
                    self.fig.canvas.draw()
                    self.fig.canvas.flush_events()
                
                #if elapsed % 20 == 0 and elapsed < wk_start:
                #    self.send_com(str("DEEP 0.0").encode())

                #print("Sim")
            #plt.show()
    
    def manage(self, duration_sec, duration_wakeup_minimum, duration_predict):                           # manage control signals from the ECG feeding to the State Machine while asleep

        SIZE = 1024
        pos = 0
        
        r_lines = []
        data = []
        tupple = []
        maxim = 0
        since_last_max = 0
        prev_5 = [0, 0, 0, 0, 0]
        prev_5_avg = 0
        prev_5_counter = 0
        intensity = float(0.0)
        sec_elapsed = int(0)

        prev_t = 0
        cur_t = 0
        del_t = 0
        
        start_force_wake = duration_sec - duration_wakeup_minimum
        start_predict_time = start_force_wake - duration_predict
        trigger = False
        predict_begin = False
        wake_begin = False
        time_wake_sec = float(duration_wakeup_minimum)
    
        #self.sock.listen(1)

        if self.do_g == True:
            self.ax_int = self.ax2.twinx()
            linez, = self.ax2.plot([],[], color = 'blue', linewidth = 1)
            line_intensity, = self.ax_int.plot([],[], color = 'red', linestyle = 'dashed', linewidth = 1)
            self.ax_int.set_ylim([-0.025, 1])
            self.ax2.set_autoscaley_on(True)
            self.ax2.grid()

        while True: # Poll Loop
            trigger = False
            # with open('../AccelData/AccelData.txt') as reader:
            with open('Parse_Test/FakeAccel.txt') as reader:          # Can swap this and the above line out for the purpose of testing
                r_lines = reader.readlines()

                for line in r_lines[pos:]:
                    tupple = self.mdecode(line)
                    print(tupple)
                    
                    data.append(tupple)
                    self.fill_bucket(tupple[0], tupple[1])
                    prev_t = cur_t
                    cur_t = tupple[0]
                    del_t = int(cur_t - prev_t)

                    # Important: with this line the program depends on an accurate elapsed time in the read file
                    sec_elapsed = tupple[0]
                    
                    # In the first part of the night, search for a maximum to compare against
                    if(tupple[1] > maxim):
                        maxim = tupple[1]
                        since_last_max = 0
                    else:
                        since_last_max = since_last_max + 1

                    if (predict_begin == True):
                        prev_5_avg = 0
                        for jj in range(5):
                            prev_5_avg = prev_5_avg + prev_5[jj]
                        prev_5_avg = prev_5_avg / 5

                        # This is arbitrary, one might even say Byzantine behavior
                        if (prev_5_counter > 4) and (tupple[1] > (2 * prev_5_avg)):
                            wake_begin = True
                            predict_begin = False
                            time_wake_sec = float(duration_sec - sec_elapsed)
                        
                        prev_5[prev_5_counter % 5] = tupple[1]
                        prev_5_counter = prev_5_counter + 1
                    
                    # Trigger indicates a probability of wakeup beginning
                    if(tupple[1] > maxim * 0.75):
                        trigger = True

                    # If in wakeup, appropriately increment the intensity and send a message
                    if (wake_begin == True):
                        time.sleep(0.05)                            # make sure socket has enough time to read msg, then log and send msg on socket
                        if (intensity < time_wake_sec):
                            intensity = intensity + del_t
                            if (intensity > time_wake_sec):
                                intensity = time_wake_sec
                                
                        self.log(self.state + str(100 * (intensity/time_wake_sec))[:6])
                        self.send_com(str(self.state + str(100 * (intensity/time_wake_sec))[:6]).encode())

                    if (int(sec_elapsed) >= int(start_predict_time)):
                        predict_begin = True

                    if (int(sec_elapsed) >= int(start_force_wake)):
                        wake_begin = True
                
                    if (wake_begin == False):
                        # If it's been over 2.5 hours since the last max value and we are over 3/4 through the sleep cycle, check for an early wakeup
                        if (since_last_max >= 600 and sec_elapsed > ((duration_sec * 3) / 4)):
                            # If light sleep and relatively high movement, start the wakeup early
                            if (self.state == "LIGHT " and trigger == True):
                                wake_begin = True
                                time_wake_sec = float(duration_sec - sec_elapsed)

                    print(intensity)
                    print(time_wake_sec)
                    print('---')
    
                    # Graph stuff
                    if self.do_g == True:
                        linez.set_xdata(np.append(linez.get_xdata(), float(tupple[0])))
                        linez.set_ydata(np.append(linez.get_ydata(), float(tupple[1])))
                        line_intensity.set_xdata(np.append(line_intensity.get_xdata(), float(tupple[0])))
                        line_intensity.set_ydata(np.append(line_intensity.get_ydata(), intensity/time_wake_sec))

                        self.ax_int.relim()
                        self.ax_int.autoscale_view()
                        self.ax2.relim()
                        self.ax2.autoscale_view()
                        self.fig.canvas.draw()
                        self.fig.canvas.flush_events()

                    if (intensity >= time_wake_sec):
                        return

                ppos = pos
                pos = len(r_lines)
                for jj in range(ppos, pos):
                    if (jj % 10) == 0:
                        self.fill_bucket_avg(jj, self.get_bucket_avg())
                        self.get_state(self.bk_avg_counter)
                        self.log(self.state + str(100 * (intensity/time_wake_sec))[:6])
                        self.send_com(str(self.state + str(100 * (intensity/time_wake_sec))[:6]).encode())
                        time.sleep(0.2)

                print(wake_begin)
                print(sec_elapsed)
                print(start_force_wake)

                #if (int(sec_elapsed) >= int(start_force_wake)):
                #    wake_begin = True
                #
                #if (wake_begin == False):
                #    # If it's been over 2.5 hours since the last max value and we are over 3/4 through the sleep cycle, check for an early wakeup
                #    if (since_last_max >= 600 and sec_elapsed > ((duration_sec * 3) / 4)):
                #        # If light sleep and relatively high movement, start the wakeup early
                #        if (self.state == "LIGHT " and trigger == True):
                #            wake_begin = True
                #            time_wake_sec = float(duration_sec - sec_elapsed)

            time.sleep(15)
            # time.sleep(2)         # for testing, normally 15 second poll intervals
            # sec_elapsed = sec_elapsed + 15
        
if __name__ == '__main__':

    data_f = None
    do_g = False
    do_verbose = False
    do_sim = False
    do_live = False
    
    if (len(sys.argv) <= 1):
        print("Specify Dataset? (Y/N)")
        use_ds = str(input())
        if use_ds == 'y' or use_ds == 'Y':
            print("Enter datafile:")
            df = str(input())
            S1 = Sleeper(100, df, "sleep_config.txt")
        else:
            S1 = Sleeper(100, "__no__", "sleep_config.txt")

        print("Enable Graphing (Slow)? (Y/N)")
        do_g = str(input())
        if do_g == 'n' or do_g == 'N':
            S1.disable_graph()
            #plt.ioff()
        else:
            S1.enable_graph()
            plt.ion()
    elif (len(sys.argv) > 1):
        for jj in sys.argv[1:]:
            if (jj.strip() == '-h' or jj.strip() == '--h'):
                print("Usage: <python3 parse_data.py> or <python3 parse_data.py [-h] [-verbose] [-sim OR -live] [-file:<filename>]")
            if (jj.strip() == '-verbose'):
                do_verbose = False
            if (jj.strip()[0:5] == '-file:'):
                data_f = jj.strip()[6:]
            if (jj.strip() == '-sim'):
                do_sim = True
            if (jj.strip() == '-live'):
                do_live = True
        
    clear_logfile(S1.log_file)
    
    while True:
        print("Run Simulation or Live Transfer? (S/L)")
        do_sim = str(input())
        S1.send_com("DONE 0".encode())
        time.sleep(0.25)
        if do_g != 'n' and do_sim != 'N':
            S1.graph_baseline()
        # S1.simple
        # S1.manage
        S1.log(str("DONE 0"))
        
        if do_sim == 'y' or do_sim == 'Y' or do_sim == 's' or do_sim == 'S':
            S1.sim(10000, "datasets/hrv_fake_4.txt")
            S1.send_com(str(S1.state + "100.0").encode())
            S1.log(str(S1.state + "100.0"))
            time.sleep(0.25)
            S1.send_com("DONE 100.0".encode())
            S1.log("DONE 100.0")
            
        elif do_sim == 'n' or do_sim == 'N' or do_sim == 'l' or do_sim == 'L':
            print("Enter duration of sleep in seconds:")
            sleep_time = int(input())
            print("Enter minimum duration of wakeup in seconds:")
            wk_time = int(input())
            
            S1.manage(sleep_time, wk_time, wk_time)
            S1.send_com(str(S1.state + "100.0").encode())
            S1.log(str(S1.state + "100.0"))
            time.sleep(0.25)
            S1.send_com("DONE 100.0".encode())
            S1.log("DONE 100.0")

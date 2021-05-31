import subprocess

# Pre: 24-hour time vals
def get_time_clk(start_t, end_t):
    if (end_t - start_t) > 0:
        return int((24-start_t)+end_t)
    else:
        return int(end_t-start_t)

if __name__ == '__main__':
    pr_wind = 500
    wk_wind = 500
    sl_time = 10000
    with open("config.txt", "r") as reader:
        vals = reader.readline().split(';')
        if len(vals) >= 5:
            pr_wind = vals[3]
            wk_wind = vals[4]
            times = vals[0].split(':')
            sl_time = 3600 * get_time_clk(int(times[0]), int(times[1]))

    pr_str = "-p:"+str(pr_wind)
    wk_str = "-w:"+str(wk_wind)
    sl_str = "-t:"+str(sl_time)
    rets = []
    
    output = subprocess.call(["python3", "parse_data.py", "-live", sl_str, pr_str, wk_str])
    rets.append(output)
    output = subprocess.call(["sudo", "gcc", "-o", "StateM", "StateM.c", "time.c", "-lwiringPi"])
    rets.append(output)
    output = subprocess.call(["sudo", "./StateM"])
    rets.append(output)
    # Anything else that needs to be ran... order might be different too...

    for entries in rets:
        print(entries)
        print('---')

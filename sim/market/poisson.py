import numpy as np
import matplotlib.pyplot as plt

def draw_poisson(time_interval, rate):
    nb_events = np.random.poisson(time_interval*rate)
    s = np.random.uniform(0,time_interval,nb_events)
    s.sort()
    return s

def draw_poisson_periodic(time_interval, rate, period):
    s=[]
    for timestamp in np.arange(0,time_interval,period):
        nb_events = np.random.poisson(period*rate)
        s.extend([timestamp]*nb_events)
    return s

def plot_poisson(sample, ax, horizontal=True):
    ax.plot(sample,np.ones(len(sample)) if horizontal else np.arange(len(s)),linestyle='None',marker='+')
    ax.set_xlabel('Time')
    ax.set_ylabel('Event number')

if __name__ == "__main__":
    TIME_INTERVAL=600
    RATE=0.1
    HORIZONTAL=True

    s=draw_poisson(TIME_INTERVAL,RATE)
    s.sort()

    _,ax=plt.subplots(2,1)
    ax[0].hist(s)
    plot_poisson(s, ax[1], HORIZONTAL)
    plt.show()
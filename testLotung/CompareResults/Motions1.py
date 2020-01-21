import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as sg
from scipy.interpolate import interp1d

def plot_motionData():

    PlotResults('FA1-5','surface.acc','x')
    PlotResults('FA1-5','surface.acc','y')
    PlotResults('FA1-5','surface.acc','z')

    PlotResults('DHB6','6_Clay6.acc','x')
    PlotResults('DHB6','6_Clay6.acc','y')
    PlotResults('DHB6','6_Clay6.acc','z')

    PlotResults('DHB11','11_Clay11.acc','x')
    PlotResults('DHB11','11_Clay11.acc','y')
    PlotResults('DHB11','11_Clay11.acc','z')

    PlotResults('DHB17','17_Clay17.acc','x')
    PlotResults('DHB17','17_Clay17.acc','y')
    PlotResults('DHB17','17_Clay17.acc','z')

    #accUP = np.loadtxt(open("", 'r').readlines()[:-1], skiprows=skip)

def PlotResults (RecFName, SimFName, Dir):

    if Dir == 'X' or Dir == 'x':
        bID = 'N'
        rID = '7N'
        sID = 1
        if RecFName == 'FA1-5':
            FaridFname = 'O4.out'
        elif RecFName == 'DHB6':
            FaridFname = 'O3.out'
        elif RecFName == 'DHB11':
            FaridFname = 'O2.out'
        elif RecFName == 'DHB17':
            FaridFname = 'O1.out'
    elif Dir == 'Y' or Dir == 'y':
        bID = 'U'
        rID = '7U'
        sID = 2
        if RecFName == 'FA1-5':
            FaridFname = 'O8.out'
        elif RecFName == 'DHB6':
            FaridFname = 'O7.out'
        elif RecFName == 'DHB11':
            FaridFname = 'O6.out'
        elif RecFName == 'DHB17':
            FaridFname = 'O5.out'
    elif Dir == 'Z' or Dir == 'z':
        bID = 'E'
        rID = '7E'
        sID = 3
        if RecFName == 'FA1-5':
            FaridFname = 'O12.out'
        elif RecFName == 'DHB6':
            FaridFname = 'O11.out'
        elif RecFName == 'DHB11':
            FaridFname = 'O10.out'
        elif RecFName == 'DHB17':
            FaridFname = 'O9.out'
    else:
        print("Wrong Dir")

    # Read Recorded Data
    dT = 0.005
    DS = 4
    skip = 9
    RecData, time = readLotung(RecFName+"."+rID, dT, DS, skip)

    # Read Simulated data
    SimData = np.loadtxt(open(SimFName, 'r'))
    SimTime = SimData[:,0]
    Sim = SimData[:,sID]

    baseTimeData = np.loadtxt("DHB47" + bID + ".time")
    baseAccData = np.loadtxt("DHB47" + bID + ".acc")

    # interpolate base acceleraion with recorded time
    baseTimeInterp = interp1d(baseTimeData, baseAccData, kind='linear', fill_value='extrapolate')
    #baseAcc = Sim[:,0] + 9.81 * baseTimeInterp(SimTime).reshape(len(SimTime), 1)
    TotalAcc = Sim + 9.81 * baseTimeInterp(SimTime)
    #TotalAcc = Sim

    # Create Plot comparing recorded and simulated data
    fig1 = plt.figure()
    ax1 = fig1.add_subplot(111)
    ax1.plot(time, RecData/980, color='b', linewidth=0.8)
    #ax1.plot(SimTime, Sim/9.81, 'r')
    ax1.plot(SimTime, TotalAcc/9.81, color='r', linewidth=0.8)
    ax1.set_ylabel('acc (g)')
    ax1.set_xlabel('time $(s)$')
    ax1.set_xlim(0, 20)
    ax1.set_ylim(-0.2, 0.2)
    plt.grid(color='k', linestyle='--', linewidth=0.5)
    plt.legend(('Recorded', 'SiteResponse'), loc = 0 )
    plt.savefig(RecFName+"-"+rID+".png")
    plt.show(block = False)

    #baseAccData2NS = np.loadtxt(open('DHB47NS.txt', 'r'))
    #motionSteps = np.size(baseAccData2NS)
    #motionTotalTime = motionSteps * 0.02
    #baseTimeData2NS = np.linspace(0.0, motionTotalTime, motionSteps)

    #baseAccData2EW = np.loadtxt(open('DHB47EW.txt', 'r'))
    #motionSteps = np.size(baseAccData2EW)
    #motionTotalTime = motionSteps * 0.02
    #baseTimeData2EW = np.linspace(0.0, motionTotalTime, motionSteps)

    #baseAccData2UP = np.loadtxt(open('DHB47UP.txt', 'r'))
    #motionSteps = np.size(baseAccData2UP)
    #motionTotalTime = motionSteps * 0.02
    #baseimeData2UP = np.linspace(0.0, motionTotalTime, motionSteps)

    # Plots for checking accel components
    #fig2 = plt.figure()
    #ax2 = fig2.add_subplot(111)
    #ax2.plot(baseTimeData, baseAccData)
    #ax2.plot(baseTimeData2NS, baseAccData2EW)
    #ax2.plot(SimTime, TotalAcc)
    #plt.show()

    ## Plots Comparing with Farid
    FaridData = np.loadtxt(open(FaridFname, 'r'))
    # Plots for checking accel components
    fig2 = plt.figure()
    ax2 = fig2.add_subplot(111)
    #ax2.plot(baseTimeData, baseAccData)
    #ax2.plot(time, RecData / 980, color='k', linewidth=0.8)
    ax2.plot(FaridData[:,0], FaridData[:,1]/9.81,  color='r', linewidth=0.8)
    ax2.plot(SimTime, TotalAcc/9.81,  color='b', linewidth=0.8)
    #plt.legend(('Recorded', 'OpenSees', 'SiteResponse'), loc = 0 )
    plt.legend(('OpenSees', 'SiteResponse'), loc = 0 )
    ax2.set_ylabel('acc (g)')
    ax2.set_xlabel('time $(s)$')
    ax2.set_xlim(0, 20)
    ax2.set_ylim(-0.2, 0.2)
    plt.grid(color='k', linestyle='--', linewidth=0.3)
    plt.savefig(RecFName+"-"+rID+"-OP.png")
    plt.show(block = False)

def readLotung(filename, dT, DS, skip):
    motionDT = dT*DS
    mdata = np.loadtxt(open(filename, 'r').readlines()[:-1], skiprows=skip)
    aa = mdata.reshape(-1)
    aa_resample = sg.decimate(aa,DS, n=8, ftype='iir')

    motionSteps = np.size(aa_resample)
    motionTotalTime = motionSteps * motionDT
    #time = np.linspace(0.0, motionTotalTime, motionSteps)
    time = np.linspace(0.0, motionTotalTime, motionSteps)


    return aa_resample, time

if __name__ == "__main__":
    plot_motionData()


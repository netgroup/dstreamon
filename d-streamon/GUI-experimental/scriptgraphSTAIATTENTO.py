import os
import pprint
import random
import sys
import wxversion
wxversion.select('2.8')
import wx
# The recommended way to use wx with mpl is with the WXAgg backend
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigCanvas, NavigationToolbar2WxAgg as NavigationToolbar
import numpy as np
import pylab


DATA_LENGTH = 100
REDRAW_TIMER_MS = 1000


i=106
list_ip=[]



def nomefile():
    p="misuredetectionreparse.txt"
    return p

'''
def get_result(ip1,ip2):
    k=0
    t=open("misuredetectionreparse.txt","r")
    for line in t.readlines():
        good1=line.split()[1]
        good2=line.split()[2]
        if(good1==ip1 and good2==ip2):
            resut=line.split()[0]
            pippo.append(int(result))
        while k < len(pippo):
            yield pippo[k]
'''


IP1='192.168.100.150'
IP2='192.168.100.151'
IP3='192.168.100.152'
IP4='192.168.100.92'
IP5='192.168.100.59'


def ip1():
    return IP1 
def ip2():
    return IP2 
def ip3():
    return IP3 
def ip4():
    return IP4 
def ip5():
    return IP5 

#def ip_getter(i):
#    return str(ip_list[i])


globalip="192.168.100.151"
d={}
d[101]=IP1,'red',1
d[102]=IP2,'blue',2
d[103]=IP3,'green',3
d[104]=IP4,'yellow',4
d[105]=IP5,'black',5



class DataGetter():
    t=None
    def __init__(self):
        self.t=os.popen('sudo ./start.sh auletta_SCRIPT.xml')

    def getData(self):
        print globalip, "start get datstart get dataa.------------"
        found = False
	return 1
	'''
        while not found:
            try:
	        line = self.t.readline()
            except:
                return None
            try:
                print globalip, ("in gt data")
                splitz = line.split()
	        value = splitz[0]
                feat1 = splitz[1]
                feat2 = splitz[2]
                good3 = splitz[3]
                #if (i<106 and good1 in ip_list):
                #    ip_list.append(good1)
                #    i+=1
                #else:
                #    pass
            except IndexError:
                return None
            print globalip, ("in condizione")
            if (good3 == globalip):
                found = True
                #print value, good1, globalip, good2, ("---------------------------------------------------------------------------------------------------")
                return value
            else:
                pass
        return None
	'''

#    def getData():
#        k=0
#        i=0
#        pippo=[]
#        for line in t.readlines():
#                good1=line.split()[1]
#                good2=line.split()[2]
#                if (good1==ip1() and good2==ip2() in line):
#                    pippo.append(float(line.split()[0])) 
#                    yield pippo[k]
#                    k=k+1
       
    #pippo.append(line.split()[0])
 #while k < len(pippo):
     #return int(pippo[k])
     #k=k+1






class GraphFrame(wx.Frame):
    g = None
    dg = None
 # the main frame of the application
    def __init__(self):
        wx.Frame.__init__(self, None, -1, "Real time traffic minitoring data plotter", size=(800,600))

        self.Centre()

        self.data = []
        self.paused = False

        self.create_menu()
        #self.create_menu_ip()
        self.create_status_bar()
        self.create_main_panel()

        self.redraw_timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.on_redraw_timer, self.redraw_timer)        
        self.redraw_timer.Start(REDRAW_TIMER_MS)

        #self.g = getData()
        self.dg = DataGetter()

    def create_menu(self):
        self.menubar = wx.MenuBar()
        self.menuip = wx.MenuBar()

        '''
        menu_file = wx.Menu()
        m_expt = menu_file.Append(-1, "&Save plot\tCtrl-S", "Save plot to file")
        self.Bind(wx.EVT_MENU, self.on_save_plot, m_expt)
        menu_file.AppendSeparator()
        m_exit = menu_file.Append(-1, "E&xit\tCtrl-X", "Exit")
        self.Bind(wx.EVT_MENU, self.on_exit, m_exit)
        menu_file.AppendSeparator()        

        self.menubar.Append(menu_file, "&File")
        self.SetMenuBar(self.menubar)
        '''
        
        
        menu_ip = wx.Menu()
        #while i < 106:
         #   m_ip
            



        m_ip1 = menu_ip.Append(101,d[101][0])
        self.Bind(wx.EVT_MENU, self.get_vect, m_ip1)
        menu_ip.AppendSeparator()
        m_ip2 = menu_ip.Append(102,d[102][0])
        self.Bind(wx.EVT_MENU, self.get_vect, m_ip2)
        menu_ip.AppendSeparator()
        m_ip3 = menu_ip.Append(103,d[103][0])
        self.Bind(wx.EVT_MENU, self.get_vect, m_ip3)
        menu_ip.AppendSeparator()
        m_ip4 = menu_ip.Append(104,d[104][0])
        self.Bind(wx.EVT_MENU, self.get_vect, m_ip4)
        menu_ip.AppendSeparator()
        m_ip5 = menu_ip.Append(105,d[105][0])
        self.Bind(wx.EVT_MENU, self.get_vect, m_ip5)
        menu_ip.AppendSeparator()
        self.menuip.Append(menu_ip,"&IPs") 
        self.SetMenuBar(self.menuip)       


   # def create_menu_ip(self):    
   #     menu_ip = wx.Menu()
   #     m_ip = menu_ip.Append(-2, "&IPs", "Show the IPs")
   #     self.menuip.Append(menu_ip,"&IPs")
   #     self.SetMenuBar(self.menuip)
        

    def get_vect(self,event):
        print d[event.GetId()][0]
        global globalip 
        globalip=d[event.GetId()][0]
        print globalip
        self.axes.set_title('SVM results for connection with '+globalip+' as src:', size=12)
        self.plot_data = self.axes.plot(
            self.data, 
            linewidth=d[event.GetId()][2],
            color=d[event.GetId()][1],
            )[0]




    def create_main_panel(self):
        self.panel = wx.Panel(self)

        self.init_plot()
        self.canvas = FigCanvas(self.panel, -1, self.fig)

  # pause button
        self.pause_button = wx.Button(self.panel, -1, "Pause")
        self.Bind(wx.EVT_BUTTON, self.on_pause_button, self.pause_button)
        self.Bind(wx.EVT_UPDATE_UI, self.on_update_pause_button, self.pause_button)

        self.hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        self.hbox1.Add(self.pause_button, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)

        self.vbox = wx.BoxSizer(wx.VERTICAL)
        self.vbox.Add(self.canvas, 1, flag=wx.LEFT | wx.TOP | wx.GROW)        
        self.vbox.Add(self.hbox1, 0, flag=wx.ALIGN_LEFT | wx.TOP)

        self.panel.SetSizer(self.vbox)
        #self.vbox.Fit(self)

    def create_status_bar(self):
        self.statusbar = self.CreateStatusBar()

    def init_plot(self):
        self.dpi = 100
        self.fig = Figure((3.0, 3.0), dpi=self.dpi)

        self.axes = self.fig.add_subplot(111)
        self.axes.set_axis_bgcolor('white')
        self.axes.set_title('SVM results for connection with '+globalip+' as src:', size=12)
         
        self.axes.set_ylabel('SVM Result:', size=10)        

        pylab.setp(self.axes.get_xticklabels(), fontsize=8)
        pylab.setp(self.axes.get_yticklabels(), fontsize=8)

        # plot the data as a line series, and save the reference 
        # to the plotted line series
        
        self.plot_data = self.axes.plot(
            self.data, 
            linewidth=1,
            color="red",
            )[0]

    def draw_plot(self):
  # redraws the plot
        xmax = len(self.data) if len(self.data) > DATA_LENGTH else DATA_LENGTH         
        xmin = xmax - DATA_LENGTH
        ymin = -5
        ymax = 5

        self.axes.set_xbound(lower=xmin, upper=xmax)
        self.axes.set_ybound(lower=ymin, upper=ymax)

  # enable grid
        self.axes.grid(True, color='black')

        # Using setp here is convenient, because get_xticklabels
        # returns a list over which one needs to explicitly 
        # iterate, and setp already handles this.
        #  
        pylab.setp(self.axes.get_xticklabels(), visible=True)

        self.plot_data.set_xdata(np.arange(len(self.data)))
        self.plot_data.set_ydata(np.array(self.data))
        self.canvas.draw()

    def on_pause_button(self, event):
        self.paused = not self.paused

    def on_update_pause_button(self, event):
        label = "Resume" if self.paused else "Pause"
        self.pause_button.SetLabel(label)

    def on_save_plot(self, event):
        print event.GetId()
        sys.exit(0)
        file_choices = "PNG (*.png)|*.png"

        dlg = wx.FileDialog(
            self, 
            message="Save plot as...",
            defaultDir=os.getcwd(),
            defaultFile="plot.png",
            wildcard=file_choices,
            style=wx.SAVE)

        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            self.canvas.print_figure(path, dpi=self.dpi)
            self.flash_status_message("Saved to %s" % path)

    def on_redraw_timer(self, event):
        if not self.paused:
	    newData = self.dg.getData()
            if newData:
                self.data.append(newData)
            else:
                print "."
            #try:
            #    newData = self.g.next()
            #    self.data.append(newData)
            #except StopIteration:
            #    self.g = getData()
        self.draw_plot()

    def on_exit(self, event):
        self.Destroy()

    def flash_status_message(self, msg, flash_len_ms=1500):
        self.statusbar.SetStatusText(msg)
        self.timeroff = wx.Timer(self)
        self.Bind(
            wx.EVT_TIMER, 
            self.on_flash_status_off, 
            self.timeroff)
        self.timeroff.Start(flash_len_ms, oneShot=True)

    def on_flash_status_off(self, event):
        self.statusbar.SetStatusText('')


if __name__ == '__main__':
    app = wx.PySimpleApp()
    app.frame = GraphFrame()
    app.frame.Show()
    app.MainLoop()

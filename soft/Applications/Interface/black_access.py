#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Dec  9 07:13:54 2020

@author: 
'/dev/cu.usbmodem145201'
"""
from serial import *
from tkinter import *
class app():
    def __init__(self):

        self.ser = ""
        #make a TkInter Window
        self.root = Tk()
        self.root.wm_title("Reading Serial")

        # make a scrollbar
        scrollbar = Scrollbar(self.root)
        scrollbar.pack(side=RIGHT, fill=Y)

        # make a text box to put the serial output
        self.log = Text ( self.root, width=30, height=30, takefocus=0)
        self.log.pack()
        button_connect = Button(self.root, text="Connect", command= lambda:self.connectSerial())
        button_connect.pack()

        # attach text box to scrollbar
        self.log.config(yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.log.yview)

        #make our own buffer
        #useful for parsing commands
        #Serial.readline seems unreliable at times too
        self.serBuffer = ""

        # after initializing serial, an arduino may need a bit of time to reset
        #self.root.after(100, self.readSerial)

        self.root.mainloop()
    def connectSerial(self):
        serialPort = '/dev/cu.usbmodem145201'
        baudRate = 9600
        self.ser = Serial(serialPort , baudRate, timeout=0, writeTimeout=0) #ensure non-blocking
        self.root.after(100, self.readSerial)
    def readSerial(self):
        if self.ser != "":
            while True:
                c = str(self.ser.read().decode('ascii')) # attempt to read a character from Serial

                #was anything read?
                if len(c) == 0:
                    break

                # get the buffer from outside of this function

                # check if character is a delimeter
                if c == '\r':
                    c = '' # don't want returns. chuck it

                if c == '\n':
                    self.serBuffer += "\n" # add the newline to the buffer

                    #add the line to the TOP of the log
                    print(self.serBuffer)
                    self.log.insert('0.0', self.serBuffer)
                    self.log.yview_pickplace("end")
                    self.serBuffer = "" # empty the buffer
                else:
                    self.serBuffer += c # add to the buffer

        self.root.after(10, self.readSerial) # check serial again soon


if __name__ == '__main__':
    app()
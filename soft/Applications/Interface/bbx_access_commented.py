# Import required libraries
from tkinter import *
from tkinter import ttk
from tkinter import filedialog
from tkinter import messagebox
from  ttkthemes import ThemedStyle
import serial
import serial.tools.list_ports
import datetime
import threading
import multiprocessing
import os
import csv

# Debugging variable; if set to 1, debugging messages will print to the console
dbg = 0

# Setup main window
gRoot = Tk()
gRoot.config(bg="white")
gRoot.geometry("1080x640")
gRoot.title("Black Box Connect")

# Style configuration
sty = ThemedStyle(gRoot)
sty.set_theme('radiance') 

# Configure main window grid
gRoot.columnconfigure(0,weight=1)
gRoot.rowconfigure(0,weight=1)

# Create a frame for connection settings
gFrame = ttk.LabelFrame(gRoot,text="Connection Setting",padding=10, style='TFrame')
gFrame.grid(column=1,row=1, sticky=(W,E))

# Create a frame for command buttons
gFrameCmd = ttk.LabelFrame(gRoot,text="List of commands",padding=10, width=130 , style='TFrame')
gFrameCmd.grid(column=1,row=3, sticky=(N, S, E, W))

# Create a frame for COM messages
gFrame21 = ttk.Frame(gRoot,padding=10, style='TFrame')
gFrame21.grid(column=1,row=2, sticky=(W, E, N))
gRoot.resizable(0,0)

# Configure weight of columns and rows for gFrame
for x in range(10):
    gFrame.columnconfigure(x,weight = x)
    gFrame.rowconfigure(x,weight = x)
    
# Add labels to frames
label1=ttk.Label(gFrame, text = "Serial Console")
label1.grid(column=2,row=0)

# Configure style for label2
sty.configure("label2.TLabel",borderwidth=4,relief="ridge",foreground="red",ipadx=10)
label2=ttk.Label(gFrame,sty="label2.TLabel", text = "Select Com Port")
label2.grid(column=1,row=1, sticky = (N,E,W,S))

# Fetch available COM ports
ports = serial.tools.list_ports.comports()
com_port_list = [com[0] for com in ports]
com_port_list.insert(0,"Select an Option")
if dbg == 1:
    print(com_port_list)

# Variable initializations for dropdowns
com_value_inside = StringVar()
baud_value_inside = StringVar()

# Baud rate dropdown menu
baud_menu = ttk.OptionMenu(gFrame,baud_value_inside,"select baud rate","9600",
                           '19200','28800','38400','57600','76800')
baud_menu.grid(column=3, row=1, sticky = (E))

# Function to update the available COM port list
def com_port_list_update():
    # ... [rest of the function]

# Function to print serial data
def serial_print():
    # ... [rest of the function]

# Serial command functions
def CONFIG():
    # ... [rest of the function]
# ... [similar command functions]

# Function to manage serial connection
def serial_connect(com_port,baud_rate):
    # ... [rest of the function]

counter1 = 0;

# More command functions
def SHUTDOWN():
    # ... [rest of the function]
# ... [similar command functions]

# Function to close the serial connection
def serial_close():
    # ... [rest of the function]

# Function to close the application and the serial connection
def power_off():
    # ... [rest of the function]

# Function to fetch and display the user's selections
def submit_value():
    # ... [rest of the function]

# Listbox to display serial data
Lb2 = Listbox(gFrame21, width = 130, height=20, xscrollcommand = 1)
Lb2.grid(column=1, row = 1, sticky = W+E)
# ... [rest of the scrollbar configuration]

# Function to clear the listbox
def clear_listbox():
    Lb2.delete(0,END)

# Command buttons for the gFrameCmd frame
subBtn = ttk.Button(gFrameCmd,text="GNSS live data",command = GLIVE, width=15)
# ... [other button declarations]

# Command buttons for the gFrame frame
subBtn = ttk.Button(gFrame,text="submit",command = submit_value)
# ... [other button declarations]

# Menu function that does nothing (possibly for further development or debugging)
def donothing():
   # ... [rest of the function]

# Function to save the displayed data to a file
def save():
    # ... [rest of the function]

# Function to display about info (GitHub link in this case)
def About_me():
    # ... [rest of the function]

# Menu configuration
menubar = Menu(gRoot)
# ... [rest of the menu configuration]

# Protocol for window close button
gRoot.protocol("WM_DELETE_WINDOW", power_off)

# Attach the menu to the main window
gRoot.config(menu=menubar)

# Start the GUI loop
gRoot.mainloop()

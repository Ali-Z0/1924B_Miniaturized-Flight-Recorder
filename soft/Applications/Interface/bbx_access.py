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

#for printing debugging messages in console
dbg = 0

gRoot = Tk()
gRoot.config(bg="white")
gRoot.geometry("1080x640")
gRoot.title("Black Box Connect")

# sty = ttk.Style()
# sty.theme_use("clam")
sty = ThemedStyle(gRoot)
sty.set_theme('radiance') 


gRoot.columnconfigure(0,weight=1)
gRoot.rowconfigure(0,weight=1)
#sty.configure("gframe.TFrame",background="white")
gFrame = ttk.LabelFrame(gRoot,text="Connection Setting",padding=10, style='TFrame')
gFrame.grid(column=1,row=1, sticky=(W,E))

# Frame for commands

gFrameCmd = ttk.LabelFrame(gRoot,text="List of commands",padding=10, width=130 , style='TFrame')
gFrameCmd.grid(column=1,row=3, sticky=(N, S, E, W))


#Frame for COM messages

gFrame21 = ttk.Frame(gRoot,padding=10, style='TFrame')
gFrame21.grid(column=1,row=2, sticky=(W, E, N))
gRoot.resizable(0,0)


for x in range(10):
    gFrame.columnconfigure(x,weight = x)
    gFrame.rowconfigure(x,weight = x)
    
label1=ttk.Label(gFrame, text = "Serial Console")
label1.grid(column=2,row=0)
gFrame.rowconfigure(0,weight=2)

sty.configure("label2.TLabel",borderwidth=4,relief="ridge",foreground="red",ipadx=10)
label2=ttk.Label(gFrame,sty="label2.TLabel", text = "Select Com Port")
label2.grid(column=1,row=1, sticky = (N,E,W,S))

"""
Com Port List
"""
#Start
ports = serial.tools.list_ports.comports()
com_port_list = [com[0] for com in ports]
com_port_list.insert(0,"Select an Option")
if dbg == 1:
    print(com_port_list)
#END
com_value_inside = StringVar()
baud_value_inside = StringVar()
baud_menu = ttk.OptionMenu(gFrame,baud_value_inside,"select baud rate","9600",
                           '19200','28800','38400','57600','76800')
baud_menu.grid(column=3, row=1, sticky = (E))
def com_port_list_update():
    global ports
    global com_port_list
    ports = serial.tools.list_ports.comports()
    com_port_list = [com[0] for com in ports]
    com_port_list.insert(0,"Select an Option")
    if dbg == 1:
        print(com_port_list)
    com_menu = ttk.OptionMenu(gFrame,com_value_inside,*com_port_list)
    com_menu.grid(column=2, row=1, sticky = (E))
    #Frame for the COM LIST
    gRoot_com_list = Toplevel(gRoot)
    x = gRoot.winfo_x()
    y = gRoot.winfo_y()
    gRoot_com_list.geometry("+%d+%d" %(x+200,y+200))
    gFrame01 = ttk.Frame(gRoot_com_list,padding=10)
    gFrame01.grid(column=0,row=1, sticky=(W))
    #Create a horizontal scrollbar
    scrollbar = ttk.Scrollbar(gFrame01, orient= 'horizontal')
    scrollbar.grid(column=1,row=2, sticky=W+E)

    Lb1 = Listbox(gFrame01, xscrollcommand = 1, width = 50, font= ('Helvetica 8 bold'))
    counter = 0;
    for x in ports:
        Lb1.insert(counter, str(x))
    #print (counter)
    counter += 1
    Lb1.grid(column=1,row=1, sticky=W+E)
    Lb1.config(xscrollcommand= scrollbar.set)

    #Configure the scrollbar
    scrollbar.config(command= Lb1.xview)


def serial_print():
    global serFlag
    global ser
    global counter1
    x =""
    #print("Task 1 assigned to thread: {}".format(threading.current_thread().name))
    #print("ID of process running task 1: {}".format(os.getpid()))
    if(serFlag):
        if(ser.in_waiting>0):
            #
            try:
                #x = ser.read(ser.in_waiting)
                x = ser.readline(ser.in_waiting)
                #x = ser.read_until(expected='\n', size=ser.in_waiting)
                #print(x)
                y = str(x.decode())
                Lb2.insert(counter1, str(y))
                Lb2.see("end")
                #print (counter1)
                counter1 += 1
                #gFrame.after(100,serial_print)
            except:
                pass
        ser.flush()
        gFrame.after(100,serial_print)
        
def CONFIG():
    ser.write("CONFIG".encode())
        
def write_config(tgnss : str, timu : str, toff: str, ledst : str):
    
    CINTG = "INTG:" + tgnss + "\r\n"
    # ser.write(CINTG.encode())
    # CINTI = "INTI:" + timu + "\r\n"
    # ser.write(CINTI.encode())
    # CTOFF = "TOFF:" + toff + "\r\n"
    # ser.write(CTOFF.encode())
    
    # if(ledst == "ON"):
    #     LEDS = 1
    # elif(ledst == "OFF"):
    #     LEDS = 0
    # else:
    #     LEDS = 1
        
    # CLEDS = ("\rLEDV:"+str(LEDS)+"\r")
    # ser.write(CLEDS.encode())
 
def EXIT():
    ser.write("EXIT".encode())
       
def config_mode():
    filewin2 = Toplevel(gRoot)
    filewin2.geometry("300x200")
    
    Label(filewin2, text = "GNSS measure interval : ", anchor='w').grid(column=1, row = 1)
    txt_tgnss = Text(filewin2, height=1, width=10)
    txt_tgnss.grid(column=2, row = 1)
    txt_tgnss.insert(END, "5000")
    
    Label(filewin2, text = "IMU measure interval : ", anchor='w').grid(column=1, row = 2)
    txt_timu = Text(filewin2, height=1, width=10)
    txt_timu.grid(column=2, row = 2)
    txt_timu.insert(END, "500")
    
    Label(filewin2, text = "Inactive delay : ",anchor='w').grid(column=1, row = 3)
    txt_toff = Text(filewin2, height=1, width=10)
    txt_toff.grid(column=2, row = 3)
    txt_toff.insert(END, "60")

    ledList = StringVar(filewin2)
    ledList.set("txt")
    Label(filewin2, text = "Etat LED de Vie : ",anchor='w').grid(column=1, row = 4)
    led_menu = ttk.OptionMenu(filewin2,  ledList, "ON", "ON", "OFF")
    led_menu.config(width=5)
    led_menu.grid(column=2, row = 4)
    
    
    Label(filewin2, text = "").grid(column=1, row = 5)
    _tgnss = txt_tgnss.get(1.0, END)
    _tium = txt_timu.get(1.0, END)
    _toff = txt_toff.get(1.0, END)
    #_leds = led_menu.getdouble(1.0, END)
    button = ttk.Button(filewin2, text="Send config", command=lambda:[write_config(_tgnss,_tium, _toff, ledList)], underline=TRUE).grid(column=1, row = 6, columnspan=2)
    button2 = ttk.Button(filewin2, text="Exit", command=lambda:[EXIT(), filewin2.destroy()], underline=TRUE).grid(column=1, row = 7, columnspan=2)
    filewin2.protocol("WM_DELETE_WINDOW", EXIT())

ser = serial.Serial()
serFlag = 0
def serial_connect(com_port,baud_rate):
    global ser
    ser.baudrate = baud_rate
    ser.port = com_port
    ser.timeout = 1
    ser._xonxoff=1
    ser.bytesize=serial.EIGHTBITS
    ser.parity=serial.PARITY_NONE
    ser.stopbits=serial.STOPBITS_ONE
    ser.open()
    global serFlag
    serFlag = 1
    
    t1 = threading.Thread(target = serial_print, args = (), daemon=1)
    t1.start()
    #t1.join()
    """
    P1 = multiprocessing.Process(target = serial_print, args=())
    P1.start()
    P1.join()
    """
    #serial_print()
counter1 = 0;

def SHUTDOWN():
    ser.write("SHUTDOWN".encode())
def GCLR():
    ser.write("GCLR".encode())
def ICLR():
    ser.write("ICLR".encode())
def GLIVE():
    ser.write("GLIVE".encode())
def ILIVE():
    ser.write("ILIVE".encode())
def GLOG():
    ser.write("GLOG".encode())
def ILOG():
    ser.write("ILOG".encode())
    
def serial_close():
    global ser
    global serFlag
    serFlag = 0
    ser.close()
    
def power_off():
    global ser
    global serFlag
    serFlag = 0
    ser.close()
    if messagebox.askokcancel("Quit", "Do you want to quit?"):
        gRoot.destroy()
    
    
def submit_value():
    if dbg == 1:
        print("selected option: {}".format(com_value_inside.get()))
        print(" Baud Rate {}".format(baud_value_inside.get()))
    serial_connect(com_value_inside.get(),baud_value_inside.get())


Lb2 = Listbox(gFrame21, width = 130, height=20, xscrollcommand = 1)
Lb2.grid(column=1, row = 1, sticky = W+E)
Sb2 = ttk.Scrollbar(gFrame21,orient = 'vertical')
Sb2.config(command=Lb2.yview)
Sb2.grid(column = 2,row =1, sticky=N+S)
Sb2v = ttk.Scrollbar(gFrame21,orient = 'horizontal')
Sb2v.grid(column = 1,row =2, sticky=W+E)
Sb2v.config(command = Lb2.xview)
Lb2.configure(xscrollcommand = Sb2v.set, yscrollcommand = Sb2.set)

def clear_listbox():
    Lb2.delete(0,END)
    



subBtn = ttk.Button(gFrameCmd,text="GNSS live data",command = GLIVE, width=15)
subBtn.grid(column=1,row=1, sticky = (E))
subBtn = ttk.Button(gFrameCmd,text="IMU live data",command = ILIVE, width=15)
subBtn.grid(column=1,row=2, sticky = (E))
    
subBtn = ttk.Button(gFrameCmd,text="Get GNSS logs",command = GLOG, width=20)
subBtn.grid(column=2,row=1, sticky = (E))
subBtn = ttk.Button(gFrameCmd,text="Get IMU logs",command = ILOG, width=20)
subBtn.grid(column=2,row=2, sticky = (E))

subBtn = ttk.Button(gFrameCmd,text="Delete GNSS logs",command = GCLR, width=20)
subBtn.grid(column=3,row=1, sticky = (E))
subBtn = ttk.Button(gFrameCmd,text="Delete IMU logs",command = ICLR, width=20)
subBtn.grid(column=3,row=2, rowspan=2, sticky = (E))

subBtn = ttk.Button(gFrameCmd,text="Configurate BlackBox",command = lambda:[config_mode(), CONFIG()], width=24)
subBtn.grid(column=0,row=1, rowspan=2, sticky = (E))


subBtn = ttk.Button(gFrameCmd,text="Exit",command = EXIT, style = "W.TButton", underline=TRUE, width=10)
subBtn.grid(column=4,row=1, sticky = (E))

subBtn = ttk.Button(gFrameCmd,text="Shutdown",command = SHUTDOWN, style = "W.TButton", underline=TRUE, width=10)
subBtn.grid(column=4,row=2, sticky = (E))



subBtn = ttk.Button(gFrame,text="submit",command = submit_value)
subBtn.grid(column=4,row=1, sticky = (E))

RefreshBtn = ttk.Button(gFrame,text="Get List",command = com_port_list_update)
RefreshBtn.grid(column=2,row=2, sticky = (E))



closeBtn = ttk.Button(gFrame,text="Disconnect",command = serial_close)
closeBtn.grid(column=4,row=2, sticky = (E))

clearBtn = ttk.Button(gFrame,text="Clear Messages",command = clear_listbox)
clearBtn.grid(column=3,row=2, sticky = (E))



"""
#Add a Listbox Widget
listbox = Listbox(win, width= 350, font= ('Helvetica 15 bold'))
listbox.pack(side= LEFT, fill= BOTH)

#Add values to the Listbox
for values in range(1,101):
   listbox.insert(END, values)
"""
def donothing():
   filewin = Toplevel(gRoot)
   button = Button(filewin, text="Do nothing button")
   button.pack()
   
def writeFile(fileName : str, extension : str):
    
    extension = extension.replace("\n", "")
    fileName = fileName.replace("\n", "")
    filePath = str("./" + fileName + "." + extension)
    txtToSave = "".join(str(el) for el in Lb2.get(0, END))  
    
    if(extension == "csv"):
        with open(filePath, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(Lb2.get(0, END))
        
    else:
        file = open(filePath,"w")   
        file.write(txtToSave)
    
       
    
def save():
    filewin = Toplevel(gRoot)
    filewin.geometry("200x170")
    Label(filewin, text = "File name : ").pack()
    
    today = datetime.datetime.now()

    display_text = StringVar()
    txt_save = Text(filewin, height=1, width=20)
    txt_save.pack()
    txt_save.insert(END, "log_"+today.strftime("%m%d%y_%H%M%S"))

    Label(filewin, text = "Format : ").pack()
    
    varList = StringVar(filewin)
    varList.set("txt")
    format_menu = ttk.OptionMenu(filewin,  varList, "txt", "txt", "csv", "xls", "docx", "odf")
    format_menu.config(width=5)
    format_menu.pack()
    
    Label(filewin, text = "").pack()

    button = ttk.Button(filewin, text="Sauvegarder", command=lambda:[writeFile(txt_save.get(1.0, END), varList.get()), filewin.destroy() ], underline=TRUE).pack()
    
    #Lb2.text

def About_me():
   filewin = Toplevel(gRoot)
   Label1 = Label(filewin, text = "https://github.com/Ali-Z0/1924B_MiniBoiteNoire").pack()
   button = Button(filewin, text="Quit", command = filewin.destroy)
   button.pack()

menubar = Menu(gRoot)
filemenu = Menu(menubar, tearoff=0)
#filemenu.add_command(label="New", command=donothing)
#filemenu.add_command(label="Open", command=donothing)
filemenu.add_command(label="Save", command=save)
#filemenu.add_command(label="Save as...", command=donothing)
filemenu.add_command(label="Close", command=power_off)

#filemenu.add_separator()

#filemenu.add_command(label="Exit", command=gRoot.quit)
menubar.add_cascade(label="File", menu=filemenu)
editmenu = Menu(menubar, tearoff=0)
editmenu.add_command(label="Undo", command=donothing)

editmenu.add_separator()

"""
editmenu.add_command(label="Cut", command=donothing)
editmenu.add_command(label="Copy", command=donothing)
editmenu.add_command(label="Paste", command=donothing)
editmenu.add_command(label="Delete", command=donothing)
editmenu.add_command(label="Select All", command=donothing)
"""

#menubar.add_cascade(label="Edit", menu=editmenu)
helpmenu = Menu(menubar, tearoff=0)
#helpmenu.add_command(label="Help Index", command=donothing)
#helpmenu.add_command(label="About...", command=donothing)
#menubar.add_cascade(label="Help", menu=helpmenu)
menubar.add_command(label = "Black Box", command = About_me)
menubar.add_separator()
menubar.add_command(label = "Quit", command = power_off)

gRoot.protocol("WM_DELETE_WINDOW", power_off)
gRoot.config(menu=menubar)
gRoot.mainloop()
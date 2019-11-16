from tkinter import *
from threading import Thread
import sys
import argparse

def gui():
    win = Tk()
    titre = Label(win, text='Application de Recherche de Films', fg='black')
    titre.pack()
    bStart = Button(win, text='Quitter', command = win.destroy)
    bStart.pack()
    win.mainloop()
    
def recherche_id(id):
    print('id : ', end='')
    print(id)
    
def check_args():
    parser = argparse.ArgumentParser(description='RechFilmCC # Recherche de films', usage = '%(prog)s [--options]')
    #Create a --gui option to execute gui() function #
    parser.add_argument('-g', '--gui', action="store_true",
                        help='Enable Graphical User Interface')
    #Create a --id option to execute recherche_id() function #
    parser.add_argument('--id', 
                        help='Recherche par id')
    args = parser.parse_args()
    
    if args.id:
        recherche_id(args.id)
        
    if args.gui:
        thread_gui = ThreadGui()
        thread_gui.start()
        #thread_gui.join()


class ThreadGui(Thread):

    # Thread s'occupant de la partie graphique
    def _init_(self): 
        Thread._init_(self)
        
    def run(self):
        print('# Start Thread Graphical Interface') 
        gui()
        
print('#' * 30)
print('# Start app')
# Check command line arguments: --gui --db
check_args()
print('# Fin app') 
#while(1):
 #   command = input('>>> ')
  #  print(command)
   # if(command == 'id')

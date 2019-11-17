from tkinter import *
from threading import Thread
from db import DB
from subprocess import Popen
import sys
import argparse
import os

def gui():
    win = Tk()
    titre = Label(win, text='Application de Recherche de Films', fg='black')
    titre.pack()
    bStart = Button(win, text='Quitter', command = win.destroy)
    bStart.pack()
    win.mainloop()
    
def recherche_id(_id, db):
    print('\nRecherche par id :')
    #db = DB(username="root", password="root", hostname="localhost", dbtype="mysql", dbname="movies")
    #print(db.tables)
    #print(db.find_column("id"))
    print("SELECT * FROM movie WHERE id LIKE '" + str(id) + "';")
    
    #resultat = db.query("SELECT * FROM movie WHERE id LIKE '" + str(_id) + "';")
    resultat = db.query("SELECT id, title FROM movie WHERE id LIKE '" + str(_id) + "';")
    
    print(resultat)

def recherche_title(_title, db):
    print(_title)
    
    resultat = db.query("SELECT id, title FROM movie WHERE title LIKE '" + _title + "';")
    print(resultat)
def git():
    try:
        print("Save to github")
        p = Popen("save.bat", cwd=os.getcwd())
        stdout, stderr = p.communicate()
        print("Save Succedeed")
    except FileNotFoundError:
        print("Save Failed - File save.bat not found")
    
def check_args(db):
    parser = argparse.ArgumentParser(description='RechFilmCC # Recherche de films', usage = '%(prog)s [--options]')
    #Create a --gui option to execute gui() function #
    parser.add_argument('-g', '--gui', action="store_true",
                        help='Enable Graphical User Interface')
    #Create a --id option to execute recherche_id() function #
    parser.add_argument('--id', 
                        help='Recherche par id')
    #Create a --title option to execute recherche_title() function #
    parser.add_argument('--title', 
                        help='Recherche par titre')
    #Create a --actor option to execute recherche_actor() function #
    parser.add_argument('--actor', 
                        help='Recherche par titre')
    #Create a --budget option to execute recherche_budget() function #
    parser.add_argument('--budget', 
                        help='Recherche par titre')                    
    #Create a --released-date option to execute recherche_releasedDate() function #
    parser.add_argument('--released-date', 
                        help='Recherche par titre')
    #Create a --git option to execute git() function # => push to gitHub https://github.com/Marcotty/RENNEQUINEPOLIS.git
    parser.add_argument('--git', action="store_true",
                        help='Save to github')                  
    args = parser.parse_args()
    if args.gui:
        thread_gui = ThreadGui()
        thread_gui.start()
        #thread_gui.join()
    if args.id:
        recherche_id(args.id, db)
    elif args.title:
        recherche_title(args.title, db)
    elif args.git:
        git()
    else:
        print("#No args passed")

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
db = DB(username="root", password="root", hostname="localhost", dbtype="mysql", dbname="movies")

check_args(db)
print('\n# Fin app') 
#while(1):
 #   command = input('>>> ')
  #  print(command)
   # if(command == 'id')

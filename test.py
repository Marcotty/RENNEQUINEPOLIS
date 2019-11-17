# - * - coding:Utf8 - * -    print(number)

import argparse
from db import DB
def gui():
    print('GUI enabled')
    
def exemple():
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('integers', metavar='N', type=int, nargs='+',
                        help='an integer for the accumulator')
    parser.add_argument('--sum', dest='accumulate', action='store_const',
                        const=sum, default=max,
                        help='sum the integers (default: find the max)')

    args = parser.parse_args()
    print(args.accumulate(args.integers))
    
def cin():
    parser = argparse.ArgumentParser(description='RechFilmCC # Recherche de films', usage = '%(prog)s [--options]')
    #Create a --gui option to execute gui() function #
    parser.add_argument('-g', '--gui', action="store_true",
                        help='Enable Graphical User Interface')
    args = parser.parse_args()
    if args.gui:
        gui()
        
def recherche_id(_id):

    db = DB(username="root", password="root", hostname="localhost", dbtype="mysql", dbname="movies")
    print(db.tables)
    print(db.find_column("id"))
    print("SELECT * FROM movie WHERE id LIKE '" + str(id) + "';")
    print(db.query("SELECT * FROM movie WHERE id LIKE '" + str(_id) + "';"))
#exemple()
#cin()
recherche_id(1234)

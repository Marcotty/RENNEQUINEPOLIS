# - * - coding:Utf8 - * -    print(number)

import argparse
from db import MySQL, Postgres
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
        
def recherche_id(id):

    systems = { '--mysql': MySQL, '--postgres': Postgres }
    DB = systems['--mysql']
    db = DB['movies']
#exemple()
#cin()
recherche_id(555)

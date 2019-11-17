# -*- coding: utf-8 -*-
#
#  split.py
#
#  Copyright 2019 Vincent Cadet <vincent.cadet@hepl.be>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

"""
===========================================
Importation de données de films (movies.txt)
===========================================

Ce script illustre une méthode générique pour l'importation de données en
provenance d'un fichier (ou d'un flux) texte depuis la ligne de commandes
par manipulation des flux de texte d'entrée et de sortie standard.

En raison de son format, non-standard, le flux texte source, movies.txt, ne
peut pas être importé facilement via les méthodes de traduction (mapping)
présentes dans des SGBD comme Oracle ou PostgreSQL, comme ce serait le cas
d'un fichier CSV, par exemple.

Le flux texte source est transmis au script par son entrée standard, stdin.
Ceci permet de filtrer le flux source avant traitement et de sélectrionner
avec précision les lignes (films) à importer et évite d'elourdir inutilement
le script avec des fonctions déjà fournies par le système d'exploitation.

La sortie du script doit être transmise à un client SQL comme `mysql´ (MySQL)
ou `psql´ (PostgreSQL) pour l'importation des données, via l'enchaînement de
commandes. Par exemple:

	split.py --mysql movies < movies.txt | mysql -u root

Cet exemple importe la totalité des données. Pour importer une sélection de
films:

	grep -E '^11\b' movies.txt | split.py --mysql movies | mysql -u root
	grep -E '^(11|497|12317)\b' movies.txt | split.py ...

Dans le premier exemple, seul le film dont l'identifiant, 11 (premier champ,
en début de ligne) est importé. Dans le second, ce sont les films 11, 497 et
12317.

Le client SQL peut aussi se trouver sur une autre machine, en réseau:

	split.py ... | ssh <adresse> "mysql -u root"

À titre d'exemples, les systèmes pris en charge dans le module db.py sont MySQL
et PostgreSQL. D'autres systèmes peuvent être ajoutés au module, pour autant
qu'ils fournissent exactement la même interface que les autres (duck typing).

Vu le nombre d'arguments en ligne de commande, leur gestion est simplifiée.
Une prise en charge plus élaborée à l'aide du module "getopt" est laissée en
tant qu'exercice.

Les doublons sont ignorés:

 * sous MySQL, par l'instruction INSERT IGNORE ...
 * sous PostgreSQL avec INSERT ... ON CONFLICT DO NOTHING

Le choix de la syntaxe est confié aux classes fournies par le module db.py en
fonction du type de base de données passé en argument à la ligne de commande.
"""

import sys
from collections import namedtuple
from db import Postgres, MySQL

# Database record definitions
MovieActor = namedtuple('movie_actor', ['movie_id', 'actor_id', 'character_name'])
MovieDirector = namedtuple('movie_director', ['movie_id', 'director_id'])
Movie = namedtuple('movie',
	['id', 'title', 'original_title', 'release_date', 'status',
	'vote_average', 'vote_count', 'runtime', 'certification',
	'poster_path', 'budget', 'tag_line', 'genre'])

# Field formats from the incoming text stream
Genre = namedtuple('genre', ['id', 'name'])
Person = namedtuple('person', ['id', 'full_name'])
Actor = namedtuple('actor', ['movie_id', 'actor_id', 'name', 'character'])
Director = namedtuple('director', ['movie_id', 'director_id', 'name'])

def split(text):
	""" Split a subfield and return a list of values. """
	try:
		return ( item.split('\u2024') for item in text.split('\u2016') )
	except AttributeError:
		return tuple()


# List of constructors for each possible long option
systems = { '--mysql': MySQL, '--postgres': Postgres }
try:
	# Check command line arguments: database system and name
	DB = systems[sys.argv[1]]
	db = DB(*sys.argv[2:3])
    

except:
	print('Syntax:', sys.argv[0], '--mysql|--postgres [DATABASE]', file=sys.stderr)
	sys.exit(1)
print(db)
for line in sys.stdin:
	# Remove trailing spaces from each line in the source stream
	raw_movie = line.strip().split('\u2023')

	# Make genres either an array (Postgres) or a SET (MySQL)
	genres = db.list(genre[1] for genre in split(raw_movie[-3])) if raw_movie[-3] else ''

	# Format movie record
	movie = Movie._make(raw_movie[:-3] + [genres])

	# Insert decoded movie first (primary table)
	db.insert('movies', movie)

	# Build and insert people list from actors and directors
	for person in split(raw_movie[-1]):
		try:
			actor = Actor._make([movie.id] + person)
			db.insert('people', Person(actor.actor_id, actor.name))
			db.insert('characters', MovieActor(movie.id, actor.actor_id, actor.character))
		except TypeError as e:
			# Error message format: [movie ID::actor] "message" in "characters field"
			print('[{}::actor] {} in “{}”'.format(raw_movie[0], e, raw_movie[-1]), file=sys.stderr)

	for person in split(raw_movie[-2]):
		try:
			director = Director._make([movie.id] + person)
			db.insert('people', Person(director.director_id, director.name))
			db.insert('directors', MovieDirector(movie.id, director.director_id))
		except TypeError as e:
			# Error message format: see actor
			print('[{}::director] {} in “{}”'.format(raw_movie[0], e, raw_movie[-2]), file=sys.stderr)
